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
} iotjs_http_parserwrap_t;


typedef enum http_parser_type http_parser_type;


static void iotjs_http_parserwrap_initialize(
    iotjs_http_parserwrap_t* http_parserwrap, http_parser_type type) {
  http_parser_init(&http_parserwrap->parser, type);
  iotjs_string_destroy(&http_parserwrap->url);
  iotjs_string_destroy(&http_parserwrap->status_msg);
  http_parserwrap->n_fields = 0;
  http_parserwrap->n_values = 0;
  http_parserwrap->flushed = false;
  http_parserwrap->cur_jbuf = jerry_create_null();
  http_parserwrap->cur_buf = NULL;
  http_parserwrap->cur_buf_len = 0;
}


IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(http_parserwrap);


static void iotjs_http_parserwrap_create(const jerry_value_t jparser,
                                         http_parser_type type) {
  iotjs_http_parserwrap_t* http_parserwrap =
      IOTJS_ALLOC(iotjs_http_parserwrap_t);
  http_parserwrap->jobject = jparser;
  jerry_set_object_native_pointer(jparser, http_parserwrap,
                                  &this_module_native_info);

  http_parserwrap->url = iotjs_string_create();
  http_parserwrap->status_msg = iotjs_string_create();
  for (size_t i = 0; i < HEADER_MAX; i++) {
    http_parserwrap->fields[i] = iotjs_string_create();
    http_parserwrap->values[i] = iotjs_string_create();
  }

  iotjs_http_parserwrap_initialize(http_parserwrap, type);
  http_parserwrap->parser.data = http_parserwrap;

  IOTJS_ASSERT(jerry_value_is_object(http_parserwrap->jobject));
}


static void iotjs_http_parserwrap_destroy(
    iotjs_http_parserwrap_t* http_parserwrap) {
  iotjs_string_destroy(&http_parserwrap->url);
  iotjs_string_destroy(&http_parserwrap->status_msg);
  for (size_t i = 0; i < HEADER_MAX; i++) {
    iotjs_string_destroy(&http_parserwrap->fields[i]);
    iotjs_string_destroy(&http_parserwrap->values[i]);
  }

  IOTJS_RELEASE(http_parserwrap);
}


static jerry_value_t iotjs_http_parserwrap_make_header(
    iotjs_http_parserwrap_t* http_parserwrap) {
  jerry_value_t jheader = jerry_create_array(http_parserwrap->n_values * 2);
  for (size_t i = 0; i < http_parserwrap->n_values; i++) {
    jerry_value_t f = iotjs_jval_create_string(&http_parserwrap->fields[i]);
    jerry_value_t v = iotjs_jval_create_string(&http_parserwrap->values[i]);
    iotjs_jval_set_property_by_index(jheader, i * 2, f);
    iotjs_jval_set_property_by_index(jheader, i * 2 + 1, v);
    jerry_release_value(f);
    jerry_release_value(v);
  }
  return jheader;
}


static void iotjs_http_parserwrap_flush(
    iotjs_http_parserwrap_t* http_parserwrap) {
  const jerry_value_t jobj = http_parserwrap->jobject;
  jerry_value_t func =
      iotjs_jval_get_property(jobj, IOTJS_MAGIC_STRING_ONHEADERS);
  IOTJS_ASSERT(jerry_value_is_function(func));

  jerry_value_t jheader = iotjs_http_parserwrap_make_header(http_parserwrap);
  size_t argc = 1;
  jerry_value_t argv[2] = { jheader, 0 };

  if (http_parserwrap->parser.type == HTTP_REQUEST &&
      !iotjs_string_is_empty(&http_parserwrap->url)) {
    argv[argc++] = iotjs_jval_create_string(&http_parserwrap->url);
  }

  iotjs_invoke_callback(func, jobj, argv, argc);

  iotjs_string_destroy(&http_parserwrap->url);
  for (size_t i = 0; i < argc; i++) {
    jerry_release_value(argv[i]);
  }
  jerry_release_value(func);
  http_parserwrap->flushed = true;
}


static void iotjs_http_parserwrap_set_buf(
    iotjs_http_parserwrap_t* http_parserwrap, jerry_value_t jbuf, char* buf,
    size_t sz) {
  http_parserwrap->cur_jbuf = jbuf;
  http_parserwrap->cur_buf = buf;
  http_parserwrap->cur_buf_len = sz;
}


// http-parser callbacks
static int iotjs_http_parserwrap_on_message_begin(http_parser* parser) {
  iotjs_http_parserwrap_t* http_parserwrap =
      (iotjs_http_parserwrap_t*)(parser->data);
  iotjs_string_destroy(&http_parserwrap->url);
  iotjs_string_destroy(&http_parserwrap->status_msg);
  return 0;
}


static int iotjs_http_parserwrap_on_url(http_parser* parser, const char* at,
                                        size_t length) {
  iotjs_http_parserwrap_t* http_parserwrap =
      (iotjs_http_parserwrap_t*)(parser->data);
  iotjs_string_append(&http_parserwrap->url, at, length);
  return 0;
}


static int iotjs_http_parserwrap_on_status(http_parser* parser, const char* at,
                                           size_t length) {
  iotjs_http_parserwrap_t* http_parserwrap =
      (iotjs_http_parserwrap_t*)(parser->data);
  iotjs_string_append(&http_parserwrap->status_msg, at, length);
  return 0;
}


static int iotjs_http_parserwrap_on_header_field(http_parser* parser,
                                                 const char* at,
                                                 size_t length) {
  iotjs_http_parserwrap_t* http_parserwrap =
      (iotjs_http_parserwrap_t*)(parser->data);
  if (http_parserwrap->n_fields == http_parserwrap->n_values) {
    http_parserwrap->n_fields++;
    // values and fields are flushed to JS
    // before corresponding OnHeaderValue is called.
    if (http_parserwrap->n_fields == HEADER_MAX) {
      iotjs_http_parserwrap_flush(http_parserwrap); // to JS world
      http_parserwrap->n_fields = 1;
      http_parserwrap->n_values = 0;
    }
    iotjs_string_destroy(
        &http_parserwrap->fields[http_parserwrap->n_fields - 1]);
  }
  IOTJS_ASSERT(http_parserwrap->n_fields == http_parserwrap->n_values + 1);
  iotjs_string_append(&http_parserwrap->fields[http_parserwrap->n_fields - 1],
                      at, length);

  return 0;
}


static int iotjs_http_parserwrap_on_header_value(http_parser* parser,
                                                 const char* at,
                                                 size_t length) {
  iotjs_http_parserwrap_t* http_parserwrap =
      (iotjs_http_parserwrap_t*)(parser->data);
  if (http_parserwrap->n_fields != http_parserwrap->n_values) {
    http_parserwrap->n_values++;
    iotjs_string_destroy(
        &http_parserwrap->values[http_parserwrap->n_values - 1]);
  }

  IOTJS_ASSERT(http_parserwrap->n_fields == http_parserwrap->n_values);

  iotjs_string_append(&http_parserwrap->values[http_parserwrap->n_values - 1],
                      at, length);

  return 0;
}


static int iotjs_http_parserwrap_on_headers_complete(http_parser* parser) {
  iotjs_http_parserwrap_t* http_parserwrap =
      (iotjs_http_parserwrap_t*)(parser->data);
  const jerry_value_t jobj = http_parserwrap->jobject;
  jerry_value_t func =
      iotjs_jval_get_property(jobj, IOTJS_MAGIC_STRING_ONHEADERSCOMPLETE);
  IOTJS_ASSERT(jerry_value_is_function(func));

  // URL
  jerry_value_t info = jerry_create_object();

  if (http_parserwrap->flushed) {
    // If some headers already are flushed,
    // flush the remaining headers.
    // In Flush function, url is already flushed to JS.
    iotjs_http_parserwrap_flush(http_parserwrap);
  } else {
    // Here, there was no flushed header.
    // We need to make a new header object with all header fields
    jerry_value_t jheader = iotjs_http_parserwrap_make_header(http_parserwrap);
    iotjs_jval_set_property_jval(info, IOTJS_MAGIC_STRING_HEADERS, jheader);
    jerry_release_value(jheader);
    if (http_parserwrap->parser.type == HTTP_REQUEST) {
      IOTJS_ASSERT(!iotjs_string_is_empty(&http_parserwrap->url));
      iotjs_jval_set_property_string(info, IOTJS_MAGIC_STRING_URL,
                                     &http_parserwrap->url);
    }
  }
  http_parserwrap->n_fields = http_parserwrap->n_values = 0;

  // Method
  if (http_parserwrap->parser.type == HTTP_REQUEST) {
    iotjs_jval_set_property_number(info, IOTJS_MAGIC_STRING_METHOD,
                                   http_parserwrap->parser.method);
  }
  // Status
  else if (http_parserwrap->parser.type == HTTP_RESPONSE) {
    iotjs_jval_set_property_number(info, IOTJS_MAGIC_STRING_STATUS,
                                   http_parserwrap->parser.status_code);
    iotjs_jval_set_property_string(info, IOTJS_MAGIC_STRING_STATUS_MSG,
                                   &http_parserwrap->status_msg);
  }


  // For future support, current http_server module does not support
  // upgrade and keepalive.
  // upgrade
  iotjs_jval_set_property_boolean(info, IOTJS_MAGIC_STRING_UPGRADE,
                                  http_parserwrap->parser.upgrade);
  // shouldkeepalive
  iotjs_jval_set_property_boolean(info, IOTJS_MAGIC_STRING_SHOULDKEEPALIVE,
                                  http_should_keep_alive(
                                      &http_parserwrap->parser));

  // http version number
  iotjs_jval_set_property_number(info, IOTJS_MAGIC_STRING_HTTP_VERSION_MAJOR,
                                 parser->http_major);
  iotjs_jval_set_property_number(info, IOTJS_MAGIC_STRING_HTTP_VERSION_MINOR,
                                 parser->http_minor);

  jerry_value_t res = iotjs_invoke_callback_with_result(func, jobj, &info, 1);

  int ret = 1;
  if (jerry_value_is_boolean(res)) {
    ret = iotjs_jval_as_boolean(res);
  } else if (jerry_value_is_error(res)) {
    ret = 0;
  }

  jerry_release_value(func);
  jerry_release_value(res);
  jerry_release_value(info);

  return ret;
}


static int iotjs_http_parserwrap_on_body(http_parser* parser, const char* at,
                                         size_t length) {
  iotjs_http_parserwrap_t* http_parserwrap =
      (iotjs_http_parserwrap_t*)(parser->data);
  const jerry_value_t jobj = http_parserwrap->jobject;
  jerry_value_t func = iotjs_jval_get_property(jobj, IOTJS_MAGIC_STRING_ONBODY);
  IOTJS_ASSERT(jerry_value_is_function(func));

  jerry_value_t argv[3] = { http_parserwrap->cur_jbuf,
                            jerry_create_number(at - http_parserwrap->cur_buf),
                            jerry_create_number(length) };

  iotjs_invoke_callback(func, jobj, argv, 3);

  jerry_release_value(argv[1]);
  jerry_release_value(argv[2]);
  jerry_release_value(func);

  return 0;
}


static int iotjs_http_parserwrap_on_message_complete(http_parser* parser) {
  iotjs_http_parserwrap_t* http_parserwrap =
      (iotjs_http_parserwrap_t*)(parser->data);
  const jerry_value_t jobj = http_parserwrap->jobject;
  jerry_value_t func =
      iotjs_jval_get_property(jobj, IOTJS_MAGIC_STRING_ONMESSAGECOMPLETE);
  IOTJS_ASSERT(jerry_value_is_function(func));

  iotjs_invoke_callback(func, jobj, NULL, 0);

  jerry_release_value(func);

  return 0;
}


const struct http_parser_settings settings = {
  iotjs_http_parserwrap_on_message_begin,
  iotjs_http_parserwrap_on_url,
  iotjs_http_parserwrap_on_status,
  iotjs_http_parserwrap_on_header_field,
  iotjs_http_parserwrap_on_header_value,
  iotjs_http_parserwrap_on_headers_complete,
  iotjs_http_parserwrap_on_body,
  iotjs_http_parserwrap_on_message_complete,
  NULL, /* on_chunk_header */
  NULL, /* on_chunk_complete */
};


static jerry_value_t iotjs_http_parser_return_parserrror(
    http_parser* nativeparser) {
  enum http_errno err = HTTP_PARSER_ERRNO(nativeparser);

  jerry_value_t eobj =
      iotjs_jval_create_error_without_error_flag("Parse Error");
  iotjs_jval_set_property_number(eobj, IOTJS_MAGIC_STRING_BYTEPARSED, 0);
  iotjs_jval_set_property_string_raw(eobj, IOTJS_MAGIC_STRING_CODE,
                                     http_errno_name(err));
  return eobj;
}


JS_FUNCTION(js_func_finish) {
  JS_DECLARE_THIS_PTR(http_parserwrap, parser);

  http_parser* nativeparser = &parser->parser;
  size_t rv = http_parser_execute(nativeparser, &settings, NULL, 0);

  if (rv != 0) {
    return iotjs_http_parser_return_parserrror(nativeparser);
  }

  return jerry_create_undefined();
}


JS_FUNCTION(js_func_execute) {
  JS_DECLARE_THIS_PTR(http_parserwrap, parser);
  DJS_CHECK_ARGS(1, object);

  jerry_value_t jbuffer = JS_GET_ARG(0, object);
  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);
  char* buf_data = buffer_wrap->buffer;
  size_t buf_len = iotjs_bufferwrap_length(buffer_wrap);
  DJS_CHECK(buf_data != NULL && buf_len > 0);

  iotjs_http_parserwrap_set_buf(parser, jbuffer, buf_data, buf_len);

  http_parser* nativeparser = &parser->parser;
  size_t nparsed =
      http_parser_execute(nativeparser, &settings, buf_data, buf_len);

  iotjs_http_parserwrap_set_buf(parser, jerry_create_null(), NULL, 0);


  if (!nativeparser->upgrade && nparsed != buf_len) {
    // nparsed should equal to buf_len except UPGRADE protocol
    return iotjs_http_parser_return_parserrror(nativeparser);
  } else {
    return jerry_create_number(nparsed);
  }
}


static jerry_value_t iotjs_http_parser_pause(jerry_value_t jthis, int paused) {
  JS_DECLARE_THIS_PTR(http_parserwrap, parser);

  http_parser* nativeparser = &parser->parser;
  http_parser_pause(nativeparser, paused);
  return jerry_create_undefined();
}


JS_FUNCTION(js_func_pause) {
  return iotjs_http_parser_pause(jthis, 1);
}


JS_FUNCTION(js_func_resume) {
  return iotjs_http_parser_pause(jthis, 0);
}


JS_FUNCTION(http_parser_cons) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, number);

  const jerry_value_t jparser = JS_GET_THIS();

  http_parser_type httpparser_type = (http_parser_type)(JS_GET_ARG(0, number));

  if (httpparser_type != HTTP_REQUEST && httpparser_type != HTTP_RESPONSE) {
    return JS_CREATE_ERROR(TYPE, "Invalid type of HTTP.");
  }

  iotjs_http_parserwrap_create(jparser, httpparser_type);
  return jerry_create_undefined();
}

static void http_parser_register_methods_object(jerry_value_t target) {
  jerry_value_t methods = jerry_create_array(26);

  jerry_value_t method_name;
  uint32_t idx = 0;
#define V(num, name, string)                                       \
  method_name = jerry_create_string((const jerry_char_t*)#string); \
  jerry_set_property_by_index(methods, idx++, method_name);        \
  jerry_release_value(method_name);

  HTTP_METHOD_MAP(V);
#undef V

  iotjs_jval_set_property_jval(target, IOTJS_MAGIC_STRING_METHODS, methods);
  jerry_release_value(methods);
}

jerry_value_t iotjs_init_http_parser(void) {
  jerry_value_t http_parser = jerry_create_object();

  jerry_value_t jparser_cons = jerry_create_external_function(http_parser_cons);
  iotjs_jval_set_property_jval(http_parser, IOTJS_MAGIC_STRING_HTTPPARSER,
                               jparser_cons);

  iotjs_jval_set_property_number(jparser_cons, IOTJS_MAGIC_STRING_REQUEST_U,
                                 HTTP_REQUEST);
  iotjs_jval_set_property_number(jparser_cons, IOTJS_MAGIC_STRING_RESPONSE_U,
                                 HTTP_RESPONSE);

  http_parser_register_methods_object(jparser_cons);

  jerry_value_t prototype = jerry_create_object();

  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_EXECUTE, js_func_execute);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_FINISH, js_func_finish);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_PAUSE, js_func_pause);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_RESUME, js_func_resume);

  iotjs_jval_set_property_jval(jparser_cons, IOTJS_MAGIC_STRING_PROTOTYPE,
                               prototype);

  jerry_release_value(jparser_cons);
  jerry_release_value(prototype);

  return http_parser;
}
