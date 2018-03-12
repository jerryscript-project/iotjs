/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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

#ifndef IOTJS_MODULE_HTTPS_H
#define IOTJS_MODULE_HTTPS_H

#include "iotjs_def.h"
#include <uv.h>
#include <curl/curl.h>

typedef enum {
  HTTPS_GET = 0,
  HTTPS_POST,
  HTTPS_PUT,
  HTTPS_DELETE,
  HTTPS_HEAD,
  HTTPS_CONNECT,
  HTTPS_OPTIONS,
  HTTPS_TRACE
} HTTPS_Methods;

#define STRING_GET "GET"
#define STRING_POST "POST"
#define STRING_PUT "PUT"
#define STRING_DELETE "DELETE"
#define STRING_HEAD "HEAD"
#define STRING_CONNECT "CONNECT"
#define STRING_OPTIONS "OPTIONS"
#define STRING_TRACE "TRACE"

typedef struct iotjs_https_poll_t iotjs_https_poll_t;
typedef struct iotjs_https_t iotjs_https_t;

iotjs_https_t* iotjs_https_create(const char* URL, const char* method,
                                  const char* ca, const char* cert,
                                  const char* key,
                                  const bool reject_unauthorized,
                                  jerry_value_t jthis);

// Some utility functions
void iotjs_https_check_done(iotjs_https_t* https_data);
void iotjs_https_cleanup(iotjs_https_t* https_data);
void iotjs_https_initialize_curl_opts(iotjs_https_t* https_data);
jerry_value_t iotjs_https_jthis_from_https(iotjs_https_t* https_data);
bool iotjs_https_jcallback(iotjs_https_t* https_data, const char* property,
                           const iotjs_jargs_t* jarg, bool resultvalue);
void iotjs_https_call_read_onwrite(uv_timer_t* timer);
void iotjs_https_call_read_onwrite_async(iotjs_https_t* https_data);

// Functions almost directly called by JS via JHANDLER
void iotjs_https_add_header(iotjs_https_t* https_data, const char* char_header);
void iotjs_https_data_to_write(iotjs_https_t* https_data,
                               iotjs_string_t read_chunk,
                               jerry_value_t callback, jerry_value_t onwrite);
void iotjs_https_finish_request(iotjs_https_t* https_data);
void iotjs_https_send_request(iotjs_https_t* https_data);
void iotjs_https_set_timeout(long ms, iotjs_https_t* https_data);


// CURL callbacks
size_t iotjs_https_curl_read_callback(void* contents, size_t size, size_t nmemb,
                                      void* userp);
int iotjs_https_curl_socket_callback(CURL* easy, curl_socket_t sockfd,
                                     int action, void* userp, void* socketp);
int iotjs_https_curl_sockopt_callback(void* userp, curl_socket_t curlfd,
                                      curlsocktype purpose);
int iotjs_https_curl_start_timeout_callback(CURLM* multi, long timeout_ms,
                                            void* userp);
size_t iotjs_https_curl_write_callback(void* contents, size_t size,
                                       size_t nmemb, void* userp);

// UV Callbacks
void iotjs_https_uv_close_callback(uv_handle_t* handle);
void iotjs_https_uv_poll_callback(uv_poll_t* poll, int status, int events);
void iotjs_https_uv_socket_timeout_callback(uv_timer_t* timer);
void iotjs_https_uv_timeout_callback(uv_timer_t* timer);

iotjs_https_poll_t* iotjs_https_poll_create(uv_loop_t* loop,
                                            curl_socket_t sockfd,
                                            iotjs_https_t* https_data);
void iotjs_https_poll_append(iotjs_https_poll_t* head,
                             iotjs_https_poll_t* poll_data);
void iotjs_https_poll_close(iotjs_https_poll_t* poll_data);
void iotjs_https_poll_destroy(iotjs_https_poll_t* poll_data);
void iotjs_https_poll_close_all(iotjs_https_poll_t* head);

#endif /* IOTJS_MODULE_HTTPS_H */
