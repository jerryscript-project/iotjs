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
#include "iotjs_module_httpparser.h"
#include "iotjs_module_buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define THIS iotjs_httpparserwrap_t* httpparserwrap


static void iotjs_httpparserwrap_destroy(THIS);


iotjs_httpparserwrap_t* iotjs_httpparserwrap_create(const iotjs_jval_t* jparser,
                                                    http_parser_type type) {
  iotjs_httpparserwrap_t* httpparserwrap = IOTJS_ALLOC(iotjs_httpparserwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_httpparserwrap_t, httpparserwrap);

  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, jparser,
                               (JFreeHandlerType)iotjs_httpparserwrap_destroy);

  _this->url = iotjs_string_create("");
  _this->status_msg = iotjs_string_create("");
  for (size_t i = 0; i < HEADER_MAX; i++) {
    _this->fields[i] = iotjs_string_create("");
    _this->values[i] = iotjs_string_create("");
  }

  iotjs_httpparserwrap_initialize(httpparserwrap, type);
  _this->parser.data = httpparserwrap;

  return httpparserwrap;
}


static void iotjs_httpparserwrap_destroy(THIS) {
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


void iotjs_httpparserwrap_initialize(THIS, http_parser_type type) {
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


// http-parser callbacks
int iotjs_httpparserwrap_on_message_begin(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);
  iotjs_string_make_empty(&_this->url);
  iotjs_string_make_empty(&_this->status_msg);
  return 0;
}


int iotjs_httpparserwrap_on_url(THIS, const char* at, size_t length) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);
  iotjs_string_append(&_this->url, at, length);
  return 0;
}


int iotjs_httpparserwrap_on_status(THIS, const char* at, size_t length) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);
  iotjs_string_append(&_this->status_msg, at, length);
  return 0;
}


int iotjs_httpparserwrap_on_header_field(THIS, const char* at, size_t length) {
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


int iotjs_httpparserwrap_on_header_value(THIS, const char* at, size_t length) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);
  if (_this->n_fields != _this->n_values) {
    _this->n_values++;
    iotjs_string_make_empty(&_this->values[_this->n_values - 1]);
  }

  IOTJS_ASSERT(_this->n_fields == _this->n_values);

  iotjs_string_append(&_this->values[_this->n_values - 1], at, length);

  return 0;
}


int iotjs_httpparserwrap_on_headers_complete(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);

  const iotjs_jval_t* jobj = iotjs_httpparserwrap_jobject(httpparserwrap);
  iotjs_jval_t func = iotjs_jval_get_property(jobj, "OnHeadersComplete");
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
    iotjs_jval_set_property_jval(&info, "headers", &jheader);
    iotjs_jval_destroy(&jheader);
    if (_this->parser.type == HTTP_REQUEST) {
      IOTJS_ASSERT(!iotjs_string_is_empty(&_this->url));
      iotjs_jval_set_property_string(&info, "url", &_this->url);
    }
  }
  _this->n_fields = _this->n_values = 0;

  // Method
  if (_this->parser.type == HTTP_REQUEST) {
    iotjs_jval_set_property_number(&info, "method", _this->parser.method);
  }

  // Status
  if (_this->parser.type == HTTP_RESPONSE) {
    iotjs_jval_set_property_number(&info, "status", _this->parser.status_code);
    iotjs_jval_set_property_string(&info, "status_msg", &_this->status_msg);
  }


  // For future support, current http_server module does not support
  // upgrade and keepalive.
  // upgrade
  iotjs_jval_set_property_boolean(&info, "upgrade", _this->parser.upgrade);
  // shouldkeepalive
  iotjs_jval_set_property_boolean(&info, "shouldkeepalive",
                                  http_should_keep_alive(&_this->parser));


  iotjs_jargs_append_jval(&argv, &info);

  iotjs_jval_t res = iotjs_make_callback_with_result(&func, jobj, &argv);
  bool ret = iotjs_jval_as_boolean(&res);

  iotjs_jargs_destroy(&argv);
  iotjs_jval_destroy(&func);
  iotjs_jval_destroy(&res);
  iotjs_jval_destroy(&info);

  return ret;
}


int iotjs_httpparserwrap_on_body(THIS, const char* at, size_t length) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);

  const iotjs_jval_t* jobj = iotjs_httpparserwrap_jobject(httpparserwrap);
  iotjs_jval_t func = iotjs_jval_get_property(jobj, "OnBody");
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


int iotjs_httpparserwrap_on_message_complete(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);

  const iotjs_jval_t* jobj = iotjs_httpparserwrap_jobject(httpparserwrap);
  iotjs_jval_t func = iotjs_jval_get_property(jobj, "OnMessageComplete");
  IOTJS_ASSERT(iotjs_jval_is_function(&func));

  iotjs_make_callback(&func, jobj, iotjs_jargs_get_empty());

  iotjs_jval_destroy(&func);

  return 0;
}


iotjs_jval_t iotjs_httpparserwrap_make_header(THIS) {
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


void iotjs_httpparserwrap_flush(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);

  const iotjs_jval_t* jobj = iotjs_httpparserwrap_jobject(httpparserwrap);
  iotjs_jval_t func = iotjs_jval_get_property(jobj, "OnHeaders");
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


void iotjs_httpparserwrap_set_buf(THIS, iotjs_jval_t* jbuf, char* buf, int sz) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);
  _this->cur_jbuf = jbuf;
  _this->cur_buf = buf;
  _this->cur_buf_len = sz;
}


iotjs_jval_t* iotjs_httpparserwrap_jobject(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);

  return iotjs_jobjectwrap_jobject(&_this->jobjectwrap);
}


http_parser* iotjs_httpparserwrap_parser(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_httpparserwrap_t, httpparserwrap);
  return &_this->parser;
}


#undef THIS


#define HTTP_CB(name)                              \
  int name(http_parser* parser) {                  \
    iotjs_httpparserwrap_t* container =            \
        (iotjs_httpparserwrap_t*)(parser->data);   \
    return iotjs_httpparserwrap_##name(container); \
  }


#define HTTP_DATA_CB(name)                                       \
  int name(http_parser* parser, const char* at, size_t length) { \
    iotjs_httpparserwrap_t* container =                          \
        (iotjs_httpparserwrap_t*)(parser->data);                 \
    return iotjs_httpparserwrap_##name(container, at, length);   \
  }


// http-parser callbacks
HTTP_CB(on_message_begin);
HTTP_DATA_CB(on_url);
HTTP_DATA_CB(on_status);
HTTP_DATA_CB(on_header_field);
HTTP_DATA_CB(on_header_value);
HTTP_CB(on_headers_complete);
HTTP_DATA_CB(on_body);
HTTP_CB(on_message_complete);


const struct http_parser_settings settings = {
  on_message_begin, on_url,
  on_status,        on_header_field,
  on_header_value,  on_headers_complete,
  on_body,          on_message_complete,
};


static iotjs_httpparserwrap_t* get_parser_wrap(const iotjs_jval_t* jparser) {
  uintptr_t handle = iotjs_jval_get_object_native_handle(jparser);
  return (iotjs_httpparserwrap_t*)(handle);
}


JHANDLER_FUNCTION(Reinitialize) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, number);

  const iotjs_jval_t* jparser = JHANDLER_GET_THIS(object);

  http_parser_type httpparser_type =
      (http_parser_type)(JHANDLER_GET_ARG(0, number));
  IOTJS_ASSERT(httpparser_type == HTTP_REQUEST ||
               httpparser_type == HTTP_RESPONSE);

  iotjs_httpparserwrap_t* parser = get_parser_wrap(jparser);
  iotjs_httpparserwrap_initialize(parser, httpparser_type);
}


JHANDLER_FUNCTION(Finish) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(0);

  const iotjs_jval_t* jparser = JHANDLER_GET_THIS(object);
  iotjs_httpparserwrap_t* parser = get_parser_wrap(jparser);

  http_parser* nativeparser = iotjs_httpparserwrap_parser(parser);
  int rv = http_parser_execute(nativeparser, &settings, NULL, 0);

  if (rv != 0) {
    enum http_errno err = HTTP_PARSER_ERRNO(nativeparser);

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
  iotjs_httpparserwrap_t* parser = get_parser_wrap(jparser);


  const iotjs_jval_t* jbuffer = JHANDLER_GET_ARG(0, object);
  iotjs_bufferwrap_t* buffer = iotjs_bufferwrap_from_jbuffer(jbuffer);
  char* buf_data = iotjs_bufferwrap_buffer(buffer);
  int buf_len = iotjs_bufferwrap_length(buffer);
  JHANDLER_CHECK(buf_data != NULL);
  JHANDLER_CHECK(buf_len > 0);

  iotjs_httpparserwrap_set_buf(parser, (iotjs_jval_t*)jbuffer, buf_data,
                               buf_len);

  http_parser* nativeparser = iotjs_httpparserwrap_parser(parser);
  int nparsed = http_parser_execute(nativeparser, &settings, buf_data, buf_len);

  iotjs_httpparserwrap_set_buf(parser, NULL, NULL, 0);


  if (!nativeparser->upgrade && nparsed != buf_len) {
    // nparsed should equal to buf_len except UPGRADE protocol
    enum http_errno err = HTTP_PARSER_ERRNO(nativeparser);
    iotjs_jval_t eobj = iotjs_jval_create_error("Parse Error");
    iotjs_jval_set_property_number(&eobj, "byteParsed", 0);
    iotjs_jval_set_property_string_raw(&eobj, "code", http_errno_name(err));
    iotjs_jhandler_return_jval(jhandler, &eobj);
    iotjs_jval_destroy(&eobj);
  } else {
    iotjs_jhandler_return_number(jhandler, nparsed);
  }
}


JHANDLER_FUNCTION(Pause) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(0);
  const iotjs_jval_t* jparser = JHANDLER_GET_THIS(object);
  iotjs_httpparserwrap_t* parser = get_parser_wrap(jparser);
  http_parser* nativeparser = iotjs_httpparserwrap_parser(parser);
  http_parser_pause(nativeparser, 1);
}


JHANDLER_FUNCTION(Resume) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(0);
  const iotjs_jval_t* jparser = JHANDLER_GET_THIS(object);
  iotjs_httpparserwrap_t* parser = get_parser_wrap(jparser);
  http_parser* nativeparser = iotjs_httpparserwrap_parser(parser);
  http_parser_pause(nativeparser, 0);
}


JHANDLER_FUNCTION(HTTPParserCons) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, number);

  const iotjs_jval_t* jparser = JHANDLER_GET_THIS(object);

  http_parser_type httpparser_type =
      (http_parser_type)(JHANDLER_GET_ARG(0, number));
  IOTJS_ASSERT(httpparser_type == HTTP_REQUEST ||
               httpparser_type == HTTP_RESPONSE);
  iotjs_httpparserwrap_t* parser =
      iotjs_httpparserwrap_create(jparser, httpparser_type);
  IOTJS_ASSERT(iotjs_jval_is_object(iotjs_httpparserwrap_jobject(parser)));
  IOTJS_ASSERT(get_parser_wrap(jparser) == parser);
}


iotjs_jval_t InitHttpparser() {
  iotjs_jval_t httpparser = iotjs_jval_create_object();

  iotjs_jval_t jParserCons = iotjs_jval_create_function(HTTPParserCons);
  iotjs_jval_set_property_jval(&httpparser, "HTTPParser", &jParserCons);

  iotjs_jval_set_property_number(&jParserCons, "REQUEST", HTTP_REQUEST);
  iotjs_jval_set_property_number(&jParserCons, "RESPONSE", HTTP_RESPONSE);

  iotjs_jval_t methods = iotjs_jval_create_object();
#define V(num, name, string) \
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
