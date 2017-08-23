/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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
#include "iotjs_module_buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http_parser.h"


// If # of header fields == HEADER_MAX, flush header to JS side.
// This is weired : # of maximum headers in C equals to HEADER_MAX-1.
// This is because , OnHeaders cb, we increase n_fields first,
// and check whether field == HEADER_MAX.
// ex) HEADER_MAX 2 means that we can keep at most 1 header field/value
// during http parsing.
// Increase this to minimize inter JS-C call
#define HEADER_MAX 10


typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;

  http_parser parser;

  iotjs_string_t url;
  iotjs_string_t status_msg;

  iotjs_string_t fields[HEADER_MAX];
  iotjs_string_t values[HEADER_MAX];
  size_t n_fields;
  size_t n_values;

  iotjs_jval_t* cur_jbuf;
  char* cur_buf;
  size_t cur_buf_len;

  bool flushed;
} IOTJS_VALIDATED_STRUCT(iotjs_httpparserwrap_t);


typedef enum http_parser_type http_parser_type;


static void iotjs_httpparserwrap_initialize(
    iotjs_httpparserwrap_t* httpparserwrap, http_parser_type type) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);
  http_parser_init(&_this->parser, type);
  iotjs_string_make_empty(&_this->url);
  iotjs_string_make_empty(&_this->status_msg);
  _this->n_fields = 0;
  _this->n_values = 0;
  _this->flushed = false;
  _this->cur_jbuf = NULL;
  _this->cur_buf = NULL;
  _this->cur_buf_len = 0;
}


IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(httpparserwrap);


static void iotjs_httpparserwrap_create(const iotjs_jval_t* jparser,
                                        http_parser_type type) {
  iotjs_httpparserwrap_t* httpparserwrap = IOTJS_ALLOC(iotjs_httpparserwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_httpparserwrap_t, httpparserwrap);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, jparser,
                               &this_module_native_info);

  _this->url = iotjs_string_create();
  _this->status_msg = iotjs_string_create();
  for (size_t i = 0; i < HEADER_MAX; i++) {
    _this->fields[i] = iotjs_string_create();
    _this->values[i] = iotjs_string_create();
  }

  iotjs_httpparserwrap_initialize(httpparserwrap, type);
  _this->parser.data = httpparserwrap;

  IOTJS_ASSERT(
      iotjs_jval_is_object(iotjs_jobjectwrap_jobject(&_this->jobjectwrap)));
}


static void iotjs_httpparserwrap_destroy(
    iotjs_httpparserwrap_t* httpparserwrap) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_httpparserwrap_t, httpparserwrap);

  iotjs_string_destroy(&_this->url);
  iotjs_string_destroy(&_this->status_msg);
  for (size_t i = 0; i < HEADER_MAX; i++) {
    iotjs_string_destroy(&_this->fields[i]);
    iotjs_string_destroy(&_this->values[i]);
  }
  iotjs_jobjectwrap_destroy(&_this->jobjectwrap);

  IOTJS_RELEASE(httpparserwrap);
}


static iotjs_jval_t iotjs_httpparserwrap_make_header(
    iotjs_httpparserwrap_t* httpparserwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);
  iotjs_jval_t jheader = iotjs_jval_create_array(_this->n_values * 2);
  for (size_t i = 0; i < _this->n_values; i++) {
    iotjs_jval_t f = iotjs_jval_create_string(&_this->fields[i]);
    iotjs_jval_t v = iotjs_jval_create_string(&_this->values[i]);
    iotjs_jval_set_property_by_index(&jheader, i * 2, &f);
    iotjs_jval_set_property_by_index(&jheader, i * 2 + 1, &v);
    iotjs_jval_destroy(&f);
    iotjs_jval_destroy(&v);
  }
  return jheader;
}


static void iotjs_httpparserwrap_flush(iotjs_httpparserwrap_t* httpparserwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);
  const iotjs_jval_t* jobj = iotjs_jobjectwrap_jobject(&_this->jobjectwrap);
  iotjs_jval_t func =
      iotjs_jval_get_property(jobj, IOTJS_MAGIC_STRING_ONHEADERS);
  IOTJS_ASSERT(iotjs_jval_is_function(&func));

  iotjs_jargs_t argv = iotjs_jargs_create(2);
  iotjs_jval_t jheader = iotjs_httpparserwrap_make_header(httpparserwrap);
  iotjs_jargs_append_jval(&argv, &jheader);
  iotjs_jval_destroy(&jheader);
  if (_this->parser.type == HTTP_REQUEST &&
      !iotjs_string_is_empty(&_this->url)) {
    iotjs_jargs_append_string(&argv, &_this->url);
  }

  iotjs_make_callback(&func, jobj, &argv);

  iotjs_string_make_empty(&_this->url);
  iotjs_jargs_destroy(&argv);
  iotjs_jval_destroy(&func);
  _this->flushed = true;
}


static void iotjs_httpparserwrap_set_buf(iotjs_httpparserwrap_t* httpparserwrap,
                                         iotjs_jval_t* jbuf, char* buf,
                                         size_t sz) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);
  _this->cur_jbuf = jbuf;
  _this->cur_buf = buf;
  _this->cur_buf_len = sz;
}


// http-parser callbacks
static int iotjs_httpparserwrap_on_message_begin(http_parser* parser) {
  iotjs_httpparserwrap_t* httpparserwrap =
      (iotjs_httpparserwrap_t*)(parser->data);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);
  iotjs_string_make_empty(&_this->url);
  iotjs_string_make_empty(&_this->status_msg);
  return 0;
}


static int iotjs_httpparserwrap_on_url(http_parser* parser, const char* at,
                                       size_t length) {
  iotjs_httpparserwrap_t* httpparserwrap =
      (iotjs_httpparserwrap_t*)(parser->data);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);
  iotjs_string_append(&_this->url, at, length);
  return 0;
}


static int iotjs_httpparserwrap_on_status(http_parser* parser, const char* at,
                                          size_t length) {
  iotjs_httpparserwrap_t* httpparserwrap =
      (iotjs_httpparserwrap_t*)(parser->data);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);
  iotjs_string_append(&_this->status_msg, at, length);
  return 0;
}


static int iotjs_httpparserwrap_on_header_field(http_parser* parser,
                                                const char* at, size_t length) {
  iotjs_httpparserwrap_t* httpparserwrap =
      (iotjs_httpparserwrap_t*)(parser->data);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);
  if (_this->n_fields == _this->n_values) {
    _this->n_fields++;
    // values and fields are flushed to JS
    // before corresponding OnHeaderValue is called.
    if (_this->n_fields == HEADER_MAX) {
      iotjs_httpparserwrap_flush(httpparserwrap); // to JS world
      _this->n_fields = 1;
      _this->n_values = 0;
    }
    iotjs_string_make_empty(&_this->fields[_this->n_fields - 1]);
  }
  IOTJS_ASSERT(_this->n_fields == _this->n_values + 1);
  iotjs_string_append(&_this->fields[_this->n_fields - 1], at, length);

  return 0;
}


static int iotjs_httpparserwrap_on_header_value(http_parser* parser,
                                                const char* at, size_t length) {
  iotjs_httpparserwrap_t* httpparserwrap =
      (iotjs_httpparserwrap_t*)(parser->data);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);
  if (_this->n_fields != _this->n_values) {
    _this->n_values++;
    iotjs_string_make_empty(&_this->values[_this->n_values - 1]);
  }

  IOTJS_ASSERT(_this->n_fields == _this->n_values);

  iotjs_string_append(&_this->values[_this->n_values - 1], at, length);

  return 0;
}


static int iotjs_httpparserwrap_on_headers_complete(http_parser* parser) {
  iotjs_httpparserwrap_t* httpparserwrap =
      (iotjs_httpparserwrap_t*)(parser->data);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);
  const iotjs_jval_t* jobj = iotjs_jobjectwrap_jobject(&_this->jobjectwrap);
  iotjs_jval_t func =
      iotjs_jval_get_property(jobj, IOTJS_MAGIC_STRING_ONHEADERSCOMPLETE);
  IOTJS_ASSERT(iotjs_jval_is_function(&func));

  // URL
  iotjs_jargs_t argv = iotjs_jargs_create(1);
  iotjs_jval_t info = iotjs_jval_create_object();

  if (_this->flushed) {
    // If some headers already are flushed,
    // flush the remaining headers.
    // In Flush function, url is already flushed to JS.
    iotjs_httpparserwrap_flush(httpparserwrap);
  } else {
    // Here, there was no flushed header.
    // We need to make a new header object with all header fields
    iotjs_jval_t jheader = iotjs_httpparserwrap_make_header(httpparserwrap);
    iotjs_jval_set_property_jval(&info, IOTJS_MAGIC_STRING_HEADERS, &jheader);
    iotjs_jval_destroy(&jheader);
    if (_this->parser.type == HTTP_REQUEST) {
      IOTJS_ASSERT(!iotjs_string_is_empty(&_this->url));
      iotjs_jval_set_property_string(&info, IOTJS_MAGIC_STRING_URL,
                                     &_this->url);
    }
  }
  _this->n_fields = _this->n_values = 0;

  // Method
  if (_this->parser.type == HTTP_REQUEST) {
    iotjs_jval_set_property_number(&info, IOTJS_MAGIC_STRING_METHOD,
                                   _this->parser.method);
  }
  // Status
  else if (_this->parser.type == HTTP_RESPONSE) {
    iotjs_jval_set_property_number(&info, IOTJS_MAGIC_STRING_STATUS,
                                   _this->parser.status_code);
    iotjs_jval_set_property_string(&info, IOTJS_MAGIC_STRING_STATUS_MSG,
                                   &_this->status_msg);
  }


  // For future support, current http_server module does not support
  // upgrade and keepalive.
  // upgrade
  iotjs_jval_set_property_boolean(&info, IOTJS_MAGIC_STRING_UPGRADE,
                                  _this->parser.upgrade);
  // shouldkeepalive
  iotjs_jval_set_property_boolean(&info, IOTJS_MAGIC_STRING_SHOULDKEEPALIVE,
                                  http_should_keep_alive(&_this->parser));


  iotjs_jargs_append_jval(&argv, &info);

  iotjs_jval_t res = iotjs_make_callback_with_result(&func, jobj, &argv);

  int ret = 1;
  if (iotjs_jval_is_boolean(&res)) {
    ret = iotjs_jval_as_boolean(&res);
  } else if (iotjs_jval_is_object(&res)) {
    // if exception throw occurs in iotjs_make_callback_with_result, then the
    // result can be an object.
    ret = 0;
  }

  iotjs_jargs_destroy(&argv);
  iotjs_jval_destroy(&func);
  iotjs_jval_destroy(&res);
  iotjs_jval_destroy(&info);

  return ret;
}


static int iotjs_httpparserwrap_on_body(http_parser* parser, const char* at,
                                        size_t length) {
  iotjs_httpparserwrap_t* httpparserwrap =
      (iotjs_httpparserwrap_t*)(parser->data);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);
  const iotjs_jval_t* jobj = iotjs_jobjectwrap_jobject(&_this->jobjectwrap);
  iotjs_jval_t func = iotjs_jval_get_property(jobj, IOTJS_MAGIC_STRING_ONBODY);
  IOTJS_ASSERT(iotjs_jval_is_function(&func));

  iotjs_jargs_t argv = iotjs_jargs_create(3);
  iotjs_jargs_append_jval(&argv, _this->cur_jbuf);
  iotjs_jargs_append_number(&argv, at - _this->cur_buf);
  iotjs_jargs_append_number(&argv, length);


  iotjs_make_callback(&func, jobj, &argv);

  iotjs_jargs_destroy(&argv);
  iotjs_jval_destroy(&func);

  return 0;
}


static int iotjs_httpparserwrap_on_message_complete(http_parser* parser) {
  iotjs_httpparserwrap_t* httpparserwrap =
      (iotjs_httpparserwrap_t*)(parser->data);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);
  const iotjs_jval_t* jobj = iotjs_jobjectwrap_jobject(&_this->jobjectwrap);
  iotjs_jval_t func =
      iotjs_jval_get_property(jobj, IOTJS_MAGIC_STRING_ONMESSAGECOMPLETE);
  IOTJS_ASSERT(iotjs_jval_is_function(&func));

  iotjs_make_callback(&func, jobj, iotjs_jargs_get_empty());

  iotjs_jval_destroy(&func);

  return 0;
}


const struct http_parser_settings settings = {
  iotjs_httpparserwrap_on_message_begin,
  iotjs_httpparserwrap_on_url,
  iotjs_httpparserwrap_on_status,
  iotjs_httpparserwrap_on_header_field,
  iotjs_httpparserwrap_on_header_value,
  iotjs_httpparserwrap_on_headers_complete,
  iotjs_httpparserwrap_on_body,
  iotjs_httpparserwrap_on_message_complete,
  NULL, /* on_chunk_header */
  NULL, /* on_chunk_complete */
};


static void iotjs_httpparser_return_parserrror(iotjs_jhandler_t* jhandler,
                                               http_parser* nativeparser) {
  enum http_errno err = HTTP_PARSER_ERRNO(nativeparser);

  iotjs_jval_t eobj = iotjs_jval_create_error("Parse Error");
  iotjs_jval_set_property_number(&eobj, IOTJS_MAGIC_STRING_BYTEPARSED, 0);
  iotjs_jval_set_property_string_raw(&eobj, IOTJS_MAGIC_STRING_CODE,
                                     http_errno_name(err));
  iotjs_jhandler_return_jval(jhandler, &eobj);
  iotjs_jval_destroy(&eobj);
}


JHANDLER_FUNCTION(Reinitialize) {
  JHANDLER_DECLARE_THIS_PTR(httpparserwrap, parser);
  DJHANDLER_CHECK_ARGS(1, number);

  http_parser_type httpparser_type =
      (http_parser_type)(JHANDLER_GET_ARG(0, number));
  IOTJS_ASSERT(httpparser_type == HTTP_REQUEST ||
               httpparser_type == HTTP_RESPONSE);

  iotjs_httpparserwrap_initialize(parser, httpparser_type);
}


JHANDLER_FUNCTION(Finish) {
  JHANDLER_DECLARE_THIS_PTR(httpparserwrap, parser);
  DJHANDLER_CHECK_ARGS(0);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, parser);

  http_parser* nativeparser = &_this->parser;
  size_t rv = http_parser_execute(nativeparser, &settings, NULL, 0);

  if (rv != 0) {
    iotjs_httpparser_return_parserrror(jhandler, nativeparser);
  }
}


JHANDLER_FUNCTION(Execute) {
  JHANDLER_DECLARE_THIS_PTR(httpparserwrap, parser);
  DJHANDLER_CHECK_ARGS(1, object);

  const iotjs_jval_t* jbuffer = JHANDLER_GET_ARG(0, object);
  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);
  char* buf_data = iotjs_bufferwrap_buffer(buffer_wrap);
  size_t buf_len = iotjs_bufferwrap_length(buffer_wrap);
  JHANDLER_CHECK(buf_data != NULL);
  JHANDLER_CHECK(buf_len > 0);

  iotjs_httpparserwrap_set_buf(parser, (iotjs_jval_t*)jbuffer, buf_data,
                               buf_len);

  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, parser);
  http_parser* nativeparser = &_this->parser;
  size_t nparsed =
      http_parser_execute(nativeparser, &settings, buf_data, buf_len);

  iotjs_httpparserwrap_set_buf(parser, NULL, NULL, 0);


  if (!nativeparser->upgrade && nparsed != buf_len) {
    // nparsed should equal to buf_len except UPGRADE protocol
    iotjs_httpparser_return_parserrror(jhandler, nativeparser);
  } else {
    iotjs_jhandler_return_number(jhandler, nparsed);
  }
}

static void iotjs_httpparser_pause(iotjs_jhandler_t* jhandler, int paused) {
  JHANDLER_DECLARE_THIS_PTR(httpparserwrap, parser);
  DJHANDLER_CHECK_ARGS(0);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, parser);

  http_parser* nativeparser = &_this->parser;
  http_parser_pause(nativeparser, paused);
}

JHANDLER_FUNCTION(Pause) {
  iotjs_httpparser_pause(jhandler, 1);
}


JHANDLER_FUNCTION(Resume) {
  iotjs_httpparser_pause(jhandler, 0);
}


JHANDLER_FUNCTION(HTTPParserCons) {
  DJHANDLER_CHECK_THIS(object);
  DJHANDLER_CHECK_ARGS(1, number);

  const iotjs_jval_t* jparser = JHANDLER_GET_THIS(object);

  http_parser_type httpparser_type =
      (http_parser_type)(JHANDLER_GET_ARG(0, number));
  IOTJS_ASSERT(httpparser_type == HTTP_REQUEST ||
               httpparser_type == HTTP_RESPONSE);
  iotjs_httpparserwrap_create(jparser, httpparser_type);
}


iotjs_jval_t InitHttpparser() {
  iotjs_jval_t httpparser = iotjs_jval_create_object();

  iotjs_jval_t jParserCons =
      iotjs_jval_create_function_with_dispatch(HTTPParserCons);
  iotjs_jval_set_property_jval(&httpparser, IOTJS_MAGIC_STRING_HTTPPARSER,
                               &jParserCons);

  iotjs_jval_set_property_number(&jParserCons, IOTJS_MAGIC_STRING_REQUEST,
                                 HTTP_REQUEST);
  iotjs_jval_set_property_number(&jParserCons, IOTJS_MAGIC_STRING_RESPONSE,
                                 HTTP_RESPONSE);

  iotjs_jval_t methods = iotjs_jval_create_object();
#define V(num, name, string) \
  iotjs_jval_set_property_string_raw(&methods, #num, #string);
  HTTP_METHOD_MAP(V)
#undef V

  iotjs_jval_set_property_jval(&jParserCons, IOTJS_MAGIC_STRING_METHODS,
                               &methods);

  iotjs_jval_t prototype = iotjs_jval_create_object();

  iotjs_jval_set_method(&prototype, IOTJS_MAGIC_STRING_EXECUTE, Execute);
  iotjs_jval_set_method(&prototype, IOTJS_MAGIC_STRING_REINITIALIZE,
                        Reinitialize);
  iotjs_jval_set_method(&prototype, IOTJS_MAGIC_STRING_FINISH, Finish);
  iotjs_jval_set_method(&prototype, IOTJS_MAGIC_STRING_PAUSE, Pause);
  iotjs_jval_set_method(&prototype, IOTJS_MAGIC_STRING_RESUME, Resume);

  iotjs_jval_set_property_jval(&jParserCons, IOTJS_MAGIC_STRING_PROTOTYPE,
                               &prototype);

  iotjs_jval_destroy(&jParserCons);
  iotjs_jval_destroy(&methods);
  iotjs_jval_destroy(&prototype);

  return httpparser;
}
