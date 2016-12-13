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


#ifndef IOTJS_MODULE_HTTPPARSER_H
#define IOTJS_MODULE_HTTPPARSER_H


#include "iotjs_objectwrap.h"

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


#define THIS iotjs_httpparserwrap_t* httpparserwrap


iotjs_httpparserwrap_t* iotjs_httpparserwrap_create(const iotjs_jval_t* jparser,
                                                    http_parser_type type);

void iotjs_httpparserwrap_initialize(THIS, http_parser_type type);

int iotjs_httpparserwrap_on_message_begin(THIS);
int iotjs_httpparserwrap_on_url(THIS, const char* at, size_t length);
int iotjs_httpparserwrap_on_status(THIS, const char* at, size_t length);
int iotjs_httpparserwrap_on_header_field(THIS, const char* at, size_t length);
int iotjs_httpparserwrap_on_header_value(THIS, const char* at, size_t length);
int iotjs_httpparserwrap_on_headers_complete(THIS);
int iotjs_httpparserwrap_on_body(THIS, const char* at, size_t length);
int iotjs_httpparserwrap_on_message_complete(THIS);

iotjs_jval_t iotjs_httpparserwrap_make_header(THIS);

void iotjs_httpparserwrap_flush(THIS);

void iotjs_httpparserwrap_set_buf(THIS, iotjs_jval_t* jbuf, char* buf, int sz);

iotjs_jval_t* iotjs_httpparserwrap_jobject(THIS);
http_parser* iotjs_httpparserwrap_parser(THIS);


#undef THIS


#endif /* IOTJS_MODULE_HTTPPARSER_H */
