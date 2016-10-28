/* Copyright 2015-2016 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "iotjs_def.h"
#include "http_parser.h"
#include "iotjs_objectwrap.h"
#include "iotjs_module_buffer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


namespace iotjs {


// If # of header fields == HEADER_MAX, flush header to JS side.
// This is weired : # of maximum headers in C equals to HEADER_MAX-1.
// This is because , OnHeaders cb, we increase n_fields first,
// and check whether field == HEADER_MAX.
// ex) HEADER_MAX 2 means that we can keep at most 1 header field/value
// during http parsing.
// Increse this to minimize inter JS-C call
#define HEADER_MAX 10



class HTTPParserWrap : public JObjectWrap {
public:
  explicit HTTPParserWrap(const iotjs_jval_t* parser_, http_parser_type type)
    : JObjectWrap(parser_) {
    url = iotjs_string_create("");
    status_msg = iotjs_string_create("");
    for (unsigned i = 0; i < HEADER_MAX; i++) {
      fields[i] = iotjs_string_create("");
      values[i] = iotjs_string_create("");
    }

    Initialize(type);
    parser.data = this;
  }

  ~HTTPParserWrap() {
    iotjs_string_destroy(&url);
    iotjs_string_destroy(&status_msg);
    for (unsigned i = 0; i < HEADER_MAX; i++) {
      iotjs_string_destroy(&fields[i]);
      iotjs_string_destroy(&values[i]);
    }
  }

  void Initialize(http_parser_type type);

  // http-parser callbacks
  int OnMessageBegin() {
    iotjs_string_make_empty(&url);
    iotjs_string_make_empty(&status_msg);
    return 0;
  }

  int OnUrl(const char* at, size_t length) {
    iotjs_string_append(&url, at, length);
    return 0;
  }

  int OnStatus(const char* at, size_t length) {
    iotjs_string_append(&status_msg, at, length);
    return 0;
  }

  int OnHeaderField(const char* at, size_t length) {

    if (n_fields == n_values) {
      n_fields++;
      // values and fields are flushed to JS
      // before corresponding OnHeaderValue is called.
      if (n_fields == HEADER_MAX) {
        Flush(); // to JS world
        n_fields = 1;
        n_values = 0;
      }
      iotjs_string_make_empty(&fields[n_fields-1]);
    }
    IOTJS_ASSERT(n_fields == n_values + 1);
    iotjs_string_append(&fields[n_fields-1], at, length);

    return 0;
  }

  int OnHeaderValue(const char* at, size_t length) {
    if (n_fields != n_values) {
      n_values++;
      iotjs_string_make_empty(&values[n_values-1]);
    }

    IOTJS_ASSERT(n_fields == n_values);

    iotjs_string_append(&values[n_values-1], at, length);

    return 0;
  }

  iotjs_jval_t makeHeader() {
    iotjs_jval_t jheader = iotjs_jval_create_array(n_values * 2);
    for (int i = 0; i < n_values; i++) {
      iotjs_jval_t f = iotjs_jval_create_string(&fields[i]);
      iotjs_jval_t v = iotjs_jval_create_string(&values[i]);
      iotjs_jval_set_property_by_index(&jheader, i * 2, &f);
      iotjs_jval_set_property_by_index(&jheader, i * 2 + 1, &v);
      iotjs_jval_destroy(&f);
      iotjs_jval_destroy(&v);
    }
    return jheader;
  }


  int OnHeadersComplete() {
    const iotjs_jval_t* jobj = jobject();
    iotjs_jval_t func = iotjs_jval_get_property(jobj, "OnHeadersComplete");
    IOTJS_ASSERT(iotjs_jval_is_function(&func));

    // URL
    iotjs_jargs_t argv = iotjs_jargs_create(1);
    iotjs_jval_t info = iotjs_jval_create_object();

    if (flushed) {
      // If some headers already are flushed,
      // flush the remaining headers.
      // In Flush function, url is already flushed to JS.
      Flush();
    }
    else {
      // Here, there was no flushed header.
      // We need to make a new header object with all header fields
      iotjs_jval_t jheader = makeHeader();
      iotjs_jval_set_property_jval(&info, "headers", &jheader);
      iotjs_jval_destroy(&jheader);
      if (parser.type == HTTP_REQUEST) {
        IOTJS_ASSERT(!iotjs_string_is_empty(&url));
        iotjs_jval_set_property_string(&info, "url", &url);
      }
    }
    n_fields = n_values = 0;

    // Method
    if (parser.type == HTTP_REQUEST) {
      iotjs_jval_set_property_number(&info, "method", parser.method);
    }

    // Status
    if (parser.type == HTTP_RESPONSE) {
      iotjs_jval_set_property_number(&info, "status", parser.status_code);
      iotjs_jval_set_property_string(&info, "status_msg", &status_msg);
    }


    // For future support, current http_server module does not support
    // upgrade and keepalive.
    // upgrade
    iotjs_jval_set_property_boolean(&info, "upgrade", parser.upgrade);
    // shouldkeepalive
    iotjs_jval_set_property_boolean(&info, "shouldkeepalive",
                                    http_should_keep_alive(&parser));


    iotjs_jargs_append_jval(&argv, &info);

    iotjs_jval_t res = MakeCallbackWithResult(&func, jobj, &argv);
    bool ret = iotjs_jval_as_boolean(&res);

    iotjs_jargs_destroy(&argv);
    iotjs_jval_destroy(&func);
    iotjs_jval_destroy(&res);
    iotjs_jval_destroy(&info);

    return ret;
  }

  int OnBody(const char* at, size_t length) {
    const iotjs_jval_t* jobj = jobject();
    iotjs_jval_t func = iotjs_jval_get_property(jobj, "OnBody");
    IOTJS_ASSERT(iotjs_jval_is_function(&func));

    iotjs_jargs_t argv = iotjs_jargs_create(3);
    iotjs_jargs_append_jval(&argv, cur_jbuf);
    iotjs_jargs_append_number(&argv, at-cur_buf);
    iotjs_jargs_append_number(&argv, length);


    MakeCallback(&func, jobj, &argv);

    iotjs_jargs_destroy(&argv);
    iotjs_jval_destroy(&func);

    return 0;
  }

  int OnMessageComplete() {
    const iotjs_jval_t* jobj = jobject();
    iotjs_jval_t func = iotjs_jval_get_property(jobj, "OnMessageComplete");
    IOTJS_ASSERT(iotjs_jval_is_function(&func));

    MakeCallback(&func, jobj, iotjs_jargs_get_empty());

    iotjs_jval_destroy(&func);

    return 0;
  }

  void Flush() {
    const iotjs_jval_t* jobj = jobject();
    iotjs_jval_t func = iotjs_jval_get_property(jobj, "OnHeaders");
    IOTJS_ASSERT(iotjs_jval_is_function(&func));

    iotjs_jargs_t argv = iotjs_jargs_create(2);
    iotjs_jval_t jheader = makeHeader();
    iotjs_jargs_append_jval(&argv, &jheader);
    iotjs_jval_destroy(&jheader);
    if (parser.type == HTTP_REQUEST && !iotjs_string_is_empty(&url)) {
      iotjs_jargs_append_string(&argv, &url);
    }

    MakeCallback(&func, jobj, &argv);

    iotjs_string_make_empty(&url);
    iotjs_jargs_destroy(&argv);
    iotjs_jval_destroy(&func);
    flushed = true;
  }

  http_parser parser;
  iotjs_string_t url;
  iotjs_string_t status_msg;
  iotjs_string_t fields[HEADER_MAX];
  iotjs_string_t values[HEADER_MAX];
  int n_fields;
  int n_values;
  iotjs_jval_t* cur_jbuf;
  char* cur_buf;
  int cur_buf_len;
  bool flushed;
};


#define HTTP_CB(name)                                                   \
  int name(http_parser* parser) {                                       \
    HTTPParserWrap* container = (HTTPParserWrap*)(parser->data);        \
    return container->name();                                           \
  }                                                                     \


#define HTTP_DATA_CB(name)                                              \
  int name(http_parser* parser, const char* at, size_t length) {        \
    HTTPParserWrap* container = (HTTPParserWrap*)(parser->data);        \
    return container->name(at, length);                                 \
  }                                                                     \



//http-parser callbacks
HTTP_CB( OnMessageBegin );
HTTP_DATA_CB( OnUrl );
HTTP_DATA_CB( OnStatus );
HTTP_DATA_CB( OnHeaderField );
HTTP_DATA_CB( OnHeaderValue );
HTTP_CB( OnHeadersComplete );
HTTP_DATA_CB( OnBody );
HTTP_CB( OnMessageComplete );


const struct http_parser_settings settings = {
  iotjs::OnMessageBegin,
  iotjs::OnUrl,
  iotjs::OnStatus,
  iotjs::OnHeaderField,
  iotjs::OnHeaderValue,
  iotjs::OnHeadersComplete,
  iotjs::OnBody,
  iotjs::OnMessageComplete,
};


void HTTPParserWrap::Initialize(http_parser_type type) {
  http_parser_init(&parser, type);
  iotjs_string_make_empty(&url);
  iotjs_string_make_empty(&status_msg);
  n_fields = 0;
  n_values = 0;
  flushed = false;
  cur_jbuf = NULL;
  cur_buf = NULL;
  cur_buf_len = 0;
}


static HTTPParserWrap* getParserWrap(const iotjs_jval_t* jparser) {
  uintptr_t handle = iotjs_jval_get_object_native_handle(jparser);
  return reinterpret_cast<HTTPParserWrap*>(handle);
}


JHANDLER_FUNCTION(Reinitialize) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, number);

  const iotjs_jval_t* jparser = JHANDLER_GET_THIS(object);

  http_parser_type httpparser_type =
    static_cast<http_parser_type>(JHANDLER_GET_ARG(0, number));
  IOTJS_ASSERT(httpparser_type == HTTP_REQUEST ||
               httpparser_type == HTTP_RESPONSE);

  HTTPParserWrap* parser = getParserWrap(jparser);
  parser->Initialize(httpparser_type);
}


JHANDLER_FUNCTION(Finish) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(0);

  const iotjs_jval_t* jparser = JHANDLER_GET_THIS(object);
  HTTPParserWrap* parser = getParserWrap(jparser);

  int rv = http_parser_execute(&(parser->parser), &settings, NULL, 0);

  if (rv != 0) {
    enum http_errno err = HTTP_PARSER_ERRNO(&parser->parser);

    iotjs_jval_t eobj = iotjs_jval_create_error("Parse Error");
    iotjs_jval_set_property_number(&eobj, "byteParsed", 0);
    iotjs_jval_set_property_string_raw(&eobj, "code", http_errno_name(err));
    iotjs_jhandler_return_jval(jhandler, &eobj);
    iotjs_jval_destroy(&eobj);
  }
}


JHANDLER_FUNCTION(Execute) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, object);

  const iotjs_jval_t* jparser = JHANDLER_GET_THIS(object);
  HTTPParserWrap* parser = getParserWrap(jparser);


  const iotjs_jval_t* jbuffer = JHANDLER_GET_ARG(0, object);
  BufferWrap* buffer = BufferWrap::FromJBuffer(jbuffer);
  char* buf_data = buffer->buffer();
  int buf_len = buffer->length();
  JHANDLER_CHECK(buf_data != NULL);
  JHANDLER_CHECK(buf_len > 0);

  parser->cur_jbuf = (iotjs_jval_t*)jbuffer;
  parser->cur_buf = buf_data;
  parser->cur_buf_len = buf_len;

  int nparsed = http_parser_execute(&(parser->parser), &settings,
                                    buf_data, buf_len);

  parser->cur_jbuf = NULL;
  parser->cur_buf = NULL;
  parser->cur_buf_len = 0;


  if (!parser->parser.upgrade && nparsed != buf_len) {
    // nparsed should equal to buf_len except UPGRADE protocol
    enum http_errno err = HTTP_PARSER_ERRNO(&parser->parser);
    iotjs_jval_t eobj = iotjs_jval_create_error("Parse Error");
    iotjs_jval_set_property_number(&eobj, "byteParsed", 0);
    iotjs_jval_set_property_string_raw(&eobj, "code", http_errno_name(err));
    iotjs_jhandler_return_jval(jhandler, &eobj);
    iotjs_jval_destroy(&eobj);
  }
  else{
    iotjs_jhandler_return_number(jhandler, nparsed);
  }
}


JHANDLER_FUNCTION(Pause) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(0);
  const iotjs_jval_t* jparser = JHANDLER_GET_THIS(object);
  HTTPParserWrap* parser = getParserWrap(jparser);
  http_parser_pause(&parser->parser, 1);
}


JHANDLER_FUNCTION(Resume) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(0);
  const iotjs_jval_t* jparser = JHANDLER_GET_THIS(object);
  HTTPParserWrap* parser = getParserWrap(jparser);
  http_parser_pause(&parser->parser, 0);
}


JHANDLER_FUNCTION(HTTPParserCons) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, number);

  const iotjs_jval_t* jparser = JHANDLER_GET_THIS(object);

  http_parser_type httpparser_type =
    static_cast<http_parser_type>(JHANDLER_GET_ARG(0, number));
  IOTJS_ASSERT(httpparser_type == HTTP_REQUEST ||
               httpparser_type == HTTP_RESPONSE);
  HTTPParserWrap* httpparser_wrap = new HTTPParserWrap(jparser,
                                                       httpparser_type);
  IOTJS_ASSERT(iotjs_jval_is_object(httpparser_wrap->jobject()));
  IOTJS_ASSERT(getParserWrap(jparser) == httpparser_wrap);
}


iotjs_jval_t InitHttpparser() {
  iotjs_jval_t httpparser = iotjs_jval_create_object();

  iotjs_jval_t jParserCons = iotjs_jval_create_function(HTTPParserCons);
  iotjs_jval_set_property_jval(&httpparser, "HTTPParser", &jParserCons);

  iotjs_jval_set_property_number(&jParserCons, "REQUEST", HTTP_REQUEST);
  iotjs_jval_set_property_number(&jParserCons, "RESPONSE", HTTP_RESPONSE);

  iotjs_jval_t methods = iotjs_jval_create_object();
#define V(num, name, string)                                                  \
  iotjs_jval_set_property_string_raw(&methods, #num, #string);
  HTTP_METHOD_MAP(V)
#undef V

  iotjs_jval_set_property_jval(&jParserCons, "methods", &methods);

  iotjs_jval_t prototype = iotjs_jval_create_object();

  iotjs_jval_set_method(&prototype, "execute", Execute);
  iotjs_jval_set_method(&prototype, "reinitialize", Reinitialize);
  iotjs_jval_set_method(&prototype, "finish", Finish);
  iotjs_jval_set_method(&prototype, "pause", Pause);
  iotjs_jval_set_method(&prototype, "resume", Resume);

  iotjs_jval_set_property_jval(&jParserCons, "prototype", &prototype);

  iotjs_jval_destroy(&jParserCons);
  iotjs_jval_destroy(&methods);
  iotjs_jval_destroy(&prototype);

  return httpparser;
}


} // namespace iotjs
