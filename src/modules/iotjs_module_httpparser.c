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
  jerry_value_t jobject;

  http_parser parser;

  iotjs_string_t url;
  iotjs_string_t status_msg;

  iotjs_string_t fields[HEADER_MAX];
  iotjs_string_t values[HEADER_MAX];
  size_t n_fields;
  size_t n_values;

  jerry_value_t cur_jbuf;
  char* cur_buf;
  size_t cur_buf_len;

  bool flushed;
} iotjs_httpparserwrap_t;


typedef enum http_parser_type http_parser_type;


static void iotjs_httpparserwrap_initialize(
    iotjs_httpparserwrap_t* httpparserwrap, http_parser_type type) {
  http_parser_init(&httpparserwrap->parser, type);
  iotjs_string_make_empty(&httpparserwrap->url);
  iotjs_string_make_empty(&httpparserwrap->status_msg);
  httpparserwrap->n_fields = 0;
  httpparserwrap->n_values = 0;
  httpparserwrap->flushed = false;
  httpparserwrap->cur_jbuf = jerry_create_null();
  httpparserwrap->cur_buf = NULL;
  httpparserwrap->cur_buf_len = 0;
}


IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(httpparserwrap);


static void iotjs_httpparserwrap_create(const jerry_value_t jparser,
                                        http_parser_type type) {
  iotjs_httpparserwrap_t* httpparserwrap = IOTJS_ALLOC(iotjs_httpparserwrap_t);
  httpparserwrap->jobject = jparser;
  jerry_set_object_native_pointer(jparser, httpparserwrap,
                                  &this_module_native_info);

  httpparserwrap->url = iotjs_string_create();
  httpparserwrap->status_msg = iotjs_string_create();
  for (size_t i = 0; i < HEADER_MAX; i++) {
    httpparserwrap->fields[i] = iotjs_string_create();
    httpparserwrap->values[i] = iotjs_string_create();
  }

  iotjs_httpparserwrap_initialize(httpparserwrap, type);
  httpparserwrap->parser.data = httpparserwrap;

  IOTJS_ASSERT(jerry_value_is_object(httpparserwrap->jobject));
}


static void iotjs_httpparserwrap_destroy(
    iotjs_httpparserwrap_t* httpparserwrap) {
  iotjs_string_destroy(&httpparserwrap->url);
  iotjs_string_destroy(&httpparserwrap->status_msg);
  for (size_t i = 0; i < HEADER_MAX; i++) {
    iotjs_string_destroy(&httpparserwrap->fields[i]);
    iotjs_string_destroy(&httpparserwrap->values[i]);
  }

  IOTJS_RELEASE(httpparserwrap);
}


static jerry_value_t iotjs_httpparserwrap_make_header(
    iotjs_httpparserwrap_t* httpparserwrap) {
  jerry_value_t jheader = jerry_create_array(httpparserwrap->n_values * 2);
  for (size_t i = 0; i < httpparserwrap->n_values; i++) {
    jerry_value_t f = iotjs_jval_create_string(&httpparserwrap->fields[i]);
    jerry_value_t v = iotjs_jval_create_string(&httpparserwrap->values[i]);
    iotjs_jval_set_property_by_index(jheader, i * 2, f);
    iotjs_jval_set_property_by_index(jheader, i * 2 + 1, v);
    jerry_release_value(f);
    jerry_release_value(v);
  }
  return jheader;
}


static void iotjs_httpparserwrap_flush(iotjs_httpparserwrap_t* httpparserwrap) {
  const jerry_value_t jobj = httpparserwrap->jobject;
  jerry_value_t func =
      iotjs_jval_get_property(jobj, IOTJS_MAGIC_STRING_ONHEADERS);
  IOTJS_ASSERT(jerry_value_is_function(func));

  iotjs_jargs_t argv = iotjs_jargs_create(2);
  jerry_value_t jheader = iotjs_httpparserwrap_make_header(httpparserwrap);
  iotjs_jargs_append_jval(&argv, jheader);
  jerry_release_value(jheader);
  if (httpparserwrap->parser.type == HTTP_REQUEST &&
      !iotjs_string_is_empty(&httpparserwrap->url)) {
    iotjs_jargs_append_string(&argv, &httpparserwrap->url);
  }

  iotjs_make_callback(func, jobj, &argv);

  iotjs_string_make_empty(&httpparserwrap->url);
  iotjs_jargs_destroy(&argv);
  jerry_release_value(func);
  httpparserwrap->flushed = true;
}


static void iotjs_httpparserwrap_set_buf(iotjs_httpparserwrap_t* httpparserwrap,
                                         jerry_value_t jbuf, char* buf,
                                         size_t sz) {
  httpparserwrap->cur_jbuf = jbuf;
  httpparserwrap->cur_buf = buf;
  httpparserwrap->cur_buf_len = sz;
}


// http-parser callbacks
static int iotjs_httpparserwrap_on_message_begin(http_parser* parser) {
  iotjs_httpparserwrap_t* httpparserwrap =
      (iotjs_httpparserwrap_t*)(parser->data);
  iotjs_string_make_empty(&httpparserwrap->url);
  iotjs_string_make_empty(&httpparserwrap->status_msg);
  return 0;
}


static int iotjs_httpparserwrap_on_url(http_parser* parser, const char* at,
                                       size_t length) {
  iotjs_httpparserwrap_t* httpparserwrap =
      (iotjs_httpparserwrap_t*)(parser->data);
  iotjs_string_append(&httpparserwrap->url, at, length);
  return 0;
}


static int iotjs_httpparserwrap_on_status(http_parser* parser, const char* at,
                                          size_t length) {
  iotjs_httpparserwrap_t* httpparserwrap =
      (iotjs_httpparserwrap_t*)(parser->data);
  iotjs_string_append(&httpparserwrap->status_msg, at, length);
  return 0;
}


static int iotjs_httpparserwrap_on_header_field(http_parser* parser,
                                                const char* at, size_t length) {
  iotjs_httpparserwrap_t* httpparserwrap =
      (iotjs_httpparserwrap_t*)(parser->data);
  if (httpparserwrap->n_fields == httpparserwrap->n_values) {
    httpparserwrap->n_fields++;
    // values and fields are flushed to JS
    // before corresponding OnHeaderValue is called.
    if (httpparserwrap->n_fields == HEADER_MAX) {
      iotjs_httpparserwrap_flush(httpparserwrap); // to JS world
      httpparserwrap->n_fields = 1;
      httpparserwrap->n_values = 0;
    }
    iotjs_string_make_empty(
        &httpparserwrap->fields[httpparserwrap->n_fields - 1]);
  }
  IOTJS_ASSERT(httpparserwrap->n_fields == httpparserwrap->n_values + 1);
  iotjs_string_append(&httpparserwrap->fields[httpparserwrap->n_fields - 1], at,
                      length);

  return 0;
}


static int iotjs_httpparserwrap_on_header_value(http_parser* parser,
                                                const char* at, size_t length) {
  iotjs_httpparserwrap_t* httpparserwrap =
      (iotjs_httpparserwrap_t*)(parser->data);
  if (httpparserwrap->n_fields != httpparserwrap->n_values) {
    httpparserwrap->n_values++;
    iotjs_string_make_empty(
        &httpparserwrap->values[httpparserwrap->n_values - 1]);
  }

  IOTJS_ASSERT(httpparserwrap->n_fields == httpparserwrap->n_values);

  iotjs_string_append(&httpparserwrap->values[httpparserwrap->n_values - 1], at,
                      length);

  return 0;
}


static int iotjs_httpparserwrap_on_headers_complete(http_parser* parser) {
  iotjs_httpparserwrap_t* httpparserwrap =
      (iotjs_httpparserwrap_t*)(parser->data);
  const jerry_value_t jobj = httpparserwrap->jobject;
  jerry_value_t func =
      iotjs_jval_get_property(jobj, IOTJS_MAGIC_STRING_ONHEADERSCOMPLETE);
  IOTJS_ASSERT(jerry_value_is_function(func));

  // URL
  iotjs_jargs_t argv = iotjs_jargs_create(1);
  jerry_value_t info = jerry_create_object();

  if (httpparserwrap->flushed) {
    // If some headers already are flushed,
    // flush the remaining headers.
    // In Flush function, url is already flushed to JS.
    iotjs_httpparserwrap_flush(httpparserwrap);
  } else {
    // Here, there was no flushed header.
    // We need to make a new header object with all header fields
    jerry_value_t jheader = iotjs_httpparserwrap_make_header(httpparserwrap);
    iotjs_jval_set_property_jval(info, IOTJS_MAGIC_STRING_HEADERS, jheader);
    jerry_release_value(jheader);
    if (httpparserwrap->parser.type == HTTP_REQUEST) {
      IOTJS_ASSERT(!iotjs_string_is_empty(&httpparserwrap->url));
      iotjs_jval_set_property_string(info, IOTJS_MAGIC_STRING_URL,
                                     &httpparserwrap->url);
    }
  }
  httpparserwrap->n_fields = httpparserwrap->n_values = 0;

  // Method
  if (httpparserwrap->parser.type == HTTP_REQUEST) {
    iotjs_jval_set_property_number(info, IOTJS_MAGIC_STRING_METHOD,
                                   httpparserwrap->parser.method);
  }
  // Status
  else if (httpparserwrap->parser.type == HTTP_RESPONSE) {
    iotjs_jval_set_property_number(info, IOTJS_MAGIC_STRING_STATUS,
                                   httpparserwrap->parser.status_code);
    iotjs_jval_set_property_string(info, IOTJS_MAGIC_STRING_STATUS_MSG,
                                   &httpparserwrap->status_msg);
  }


  // For future support, current http_server module does not support
  // upgrade and keepalive.
  // upgrade
  iotjs_jval_set_property_boolean(info, IOTJS_MAGIC_STRING_UPGRADE,
                                  httpparserwrap->parser.upgrade);
  // shouldkeepalive
  iotjs_jval_set_property_boolean(info, IOTJS_MAGIC_STRING_SHOULDKEEPALIVE,
                                  http_should_keep_alive(
                                      &httpparserwrap->parser));


  iotjs_jargs_append_jval(&argv, info);

  jerry_value_t res = iotjs_make_callback_with_result(func, jobj, &argv);

  int ret = 1;
  if (jerry_value_is_boolean(res)) {
    ret = iotjs_jval_as_boolean(res);
  } else if (jerry_value_is_object(res)) {
    // if exception throw occurs in iotjs_make_callback_with_result, then the
    // result can be an object.
    ret = 0;
  }

  iotjs_jargs_destroy(&argv);
  jerry_release_value(func);
  jerry_release_value(res);
  jerry_release_value(info);

  return ret;
}


static int iotjs_httpparserwrap_on_body(http_parser* parser, const char* at,
                                        size_t length) {
  iotjs_httpparserwrap_t* httpparserwrap =
      (iotjs_httpparserwrap_t*)(parser->data);
  const jerry_value_t jobj = httpparserwrap->jobject;
  jerry_value_t func = iotjs_jval_get_property(jobj, IOTJS_MAGIC_STRING_ONBODY);
  IOTJS_ASSERT(jerry_value_is_function(func));

  iotjs_jargs_t argv = iotjs_jargs_create(3);
  iotjs_jargs_append_jval(&argv, httpparserwrap->cur_jbuf);
  iotjs_jargs_append_number(&argv, at - httpparserwrap->cur_buf);
  iotjs_jargs_append_number(&argv, length);


  iotjs_make_callback(func, jobj, &argv);

  iotjs_jargs_destroy(&argv);
  jerry_release_value(func);

  return 0;
}


static int iotjs_httpparserwrap_on_message_complete(http_parser* parser) {
  iotjs_httpparserwrap_t* httpparserwrap =
      (iotjs_httpparserwrap_t*)(parser->data);
  const jerry_value_t jobj = httpparserwrap->jobject;
  jerry_value_t func =
      iotjs_jval_get_property(jobj, IOTJS_MAGIC_STRING_ONMESSAGECOMPLETE);
  IOTJS_ASSERT(jerry_value_is_function(func));

  iotjs_make_callback(func, jobj, iotjs_jargs_get_empty());

  jerry_release_value(func);

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


static jerry_value_t iotjs_httpparser_return_parserrror(
    http_parser* nativeparser) {
  enum http_errno err = HTTP_PARSER_ERRNO(nativeparser);

  jerry_value_t eobj =
      iotjs_jval_create_error_without_error_flag("Parse Error");
  iotjs_jval_set_property_number(eobj, IOTJS_MAGIC_STRING_BYTEPARSED, 0);
  iotjs_jval_set_property_string_raw(eobj, IOTJS_MAGIC_STRING_CODE,
                                     http_errno_name(err));
  return eobj;
}


JS_FUNCTION(Reinitialize) {
  JS_DECLARE_THIS_PTR(httpparserwrap, parser);
  DJS_CHECK_ARGS(1, number);

  http_parser_type httpparser_type = (http_parser_type)(JS_GET_ARG(0, number));

  if (httpparser_type != HTTP_REQUEST && httpparser_type != HTTP_RESPONSE) {
    return JS_CREATE_ERROR(TYPE, "Invalid type");
  }

  iotjs_httpparserwrap_initialize(parser, httpparser_type);

  return jerry_create_undefined();
}


JS_FUNCTION(Finish) {
  JS_DECLARE_THIS_PTR(httpparserwrap, parser);

  http_parser* nativeparser = &parser->parser;
  size_t rv = http_parser_execute(nativeparser, &settings, NULL, 0);

  if (rv != 0) {
    return iotjs_httpparser_return_parserrror(nativeparser);
  }

  return jerry_create_undefined();
}


JS_FUNCTION(Execute) {
  JS_DECLARE_THIS_PTR(httpparserwrap, parser);
  DJS_CHECK_ARGS(1, object);

  jerry_value_t jbuffer = JS_GET_ARG(0, object);
  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);
  char* buf_data = buffer_wrap->buffer;
  size_t buf_len = iotjs_bufferwrap_length(buffer_wrap);
  JS_CHECK(buf_data != NULL && buf_len > 0);

  iotjs_httpparserwrap_set_buf(parser, jbuffer, buf_data, buf_len);

  http_parser* nativeparser = &parser->parser;
  size_t nparsed =
      http_parser_execute(nativeparser, &settings, buf_data, buf_len);

  iotjs_httpparserwrap_set_buf(parser, jerry_create_null(), NULL, 0);


  if (!nativeparser->upgrade && nparsed != buf_len) {
    // nparsed should equal to buf_len except UPGRADE protocol
    return iotjs_httpparser_return_parserrror(nativeparser);
  } else {
    return jerry_create_number(nparsed);
  }
}


static jerry_value_t iotjs_httpparser_pause(jerry_value_t jthis, int paused) {
  JS_DECLARE_THIS_PTR(httpparserwrap, parser);

  http_parser* nativeparser = &parser->parser;
  http_parser_pause(nativeparser, paused);
  return jerry_create_undefined();
}


JS_FUNCTION(Pause) {
  return iotjs_httpparser_pause(jthis, 1);
}


JS_FUNCTION(Resume) {
  return iotjs_httpparser_pause(jthis, 0);
}


JS_FUNCTION(HTTPParserCons) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, number);

  const jerry_value_t jparser = JS_GET_THIS();

  http_parser_type httpparser_type = (http_parser_type)(JS_GET_ARG(0, number));

  if (httpparser_type != HTTP_REQUEST && httpparser_type != HTTP_RESPONSE) {
    return JS_CREATE_ERROR(TYPE, "Invalid type");
  }

  iotjs_httpparserwrap_create(jparser, httpparser_type);
  return jerry_create_undefined();
}


jerry_value_t InitHttpparser() {
  jerry_value_t httpparser = jerry_create_object();

  jerry_value_t jParserCons = jerry_create_external_function(HTTPParserCons);
  iotjs_jval_set_property_jval(httpparser, IOTJS_MAGIC_STRING_HTTPPARSER,
                               jParserCons);

  iotjs_jval_set_property_number(jParserCons, IOTJS_MAGIC_STRING_REQUEST_U,
                                 HTTP_REQUEST);
  iotjs_jval_set_property_number(jParserCons, IOTJS_MAGIC_STRING_RESPONSE_U,
                                 HTTP_RESPONSE);

  jerry_value_t methods = jerry_create_object();
#define V(num, name, string) \
  iotjs_jval_set_property_string_raw(methods, #num, #string);
  HTTP_METHOD_MAP(V)
#undef V

  iotjs_jval_set_property_jval(jParserCons, IOTJS_MAGIC_STRING_METHODS,
                               methods);

  jerry_value_t prototype = jerry_create_object();

  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_EXECUTE, Execute);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_REINITIALIZE,
                        Reinitialize);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_FINISH, Finish);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_PAUSE, Pause);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_RESUME, Resume);

  iotjs_jval_set_property_jval(jParserCons, IOTJS_MAGIC_STRING_PROTOTYPE,
                               prototype);

  jerry_release_value(jParserCons);
  jerry_release_value(methods);
  jerry_release_value(prototype);

  return httpparser;
}
