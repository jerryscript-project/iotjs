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

#include "iotjs_module_https.h"
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>

#include "iotjs_module_buffer.h"

// A Per-Request Struct, native bound to https.ClientRequest
struct iotjs_https_t {
  // Original Request Details
  const char* URL;
  HTTPS_Methods method;
  struct curl_slist* header_list;
  // TLS certs Options
  const char* ca;
  const char* cert;
  const char* key;
  bool reject_unauthorized;
  // Content-Length for Post and Put
  long content_length;

  // Handles
  uv_loop_t* loop;
  jerry_value_t jthis_native;
  CURLM* curl_multi_handle;
  uv_timer_t timeout;
  CURL* curl_easy_handle;
  // Curl Context
  int running_handles;
  int closing_handles;
  bool request_done;
  iotjs_https_poll_t* poll_data;

  // For SetTimeOut
  uv_timer_t socket_timeout;
  long timeout_ms;
  double last_bytes_num;
  uint64_t last_bytes_time;

  // For Writable Stream ClientRequest
  size_t cur_read_index;
  bool is_stream_writable;
  bool data_to_read;
  bool stream_ended;
  bool to_destroy_read_onwrite;
  iotjs_string_t read_chunk;
  jerry_value_t read_callback;
  jerry_value_t read_onwrite;
  uv_timer_t async_read_onwrite;
};

struct iotjs_https_poll_t {
  uv_poll_t poll_handle;
  iotjs_https_poll_t* next;
  iotjs_https_t* https_data;
  curl_socket_t sockfd;
  bool closing;
};

IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(https);

//-------------Constructor------------
iotjs_https_t* iotjs_https_create(const char* URL, const char* method,
                                  const char* ca, const char* cert,
                                  const char* key,
                                  const bool reject_unauthorized,
                                  jerry_value_t jthis) {
  iotjs_https_t* https_data = IOTJS_ALLOC(iotjs_https_t);

  // Original Request Details
  https_data->URL = URL;
  https_data->header_list = NULL;
  if (strcmp(method, STRING_GET) == 0)
    https_data->method = HTTPS_GET;
  else if (strcmp(method, STRING_POST) == 0)
    https_data->method = HTTPS_POST;
  else if (strcmp(method, STRING_PUT) == 0)
    https_data->method = HTTPS_PUT;
  else if (strcmp(method, STRING_DELETE) == 0)
    https_data->method = HTTPS_DELETE;
  else if (strcmp(method, STRING_HEAD) == 0)
    https_data->method = HTTPS_HEAD;
  else if (strcmp(method, STRING_CONNECT) == 0)
    https_data->method = HTTPS_CONNECT;
  else if (strcmp(method, STRING_OPTIONS) == 0)
    https_data->method = HTTPS_OPTIONS;
  else if (strcmp(method, STRING_TRACE) == 0)
    https_data->method = HTTPS_TRACE;
  else {
    IOTJS_ASSERT(0);
  }

  // TLS certs stuff
  https_data->ca = ca;
  https_data->cert = cert;
  https_data->key = key;
  https_data->reject_unauthorized = reject_unauthorized;
  // Content Length stuff
  https_data->content_length = -1;

  // Handles
  https_data->loop = iotjs_environment_loop(iotjs_environment_get());
  https_data->jthis_native = jerry_acquire_value(jthis);
  jerry_set_object_native_pointer(https_data->jthis_native, https_data,
                                  &this_module_native_info);
  https_data->curl_multi_handle = curl_multi_init();
  https_data->curl_easy_handle = curl_easy_init();
  https_data->timeout.data = (void*)https_data;
  uv_timer_init(https_data->loop, &(https_data->timeout));
  https_data->request_done = false;
  https_data->closing_handles = 3;
  https_data->poll_data = NULL;

  // Timeout stuff
  https_data->timeout_ms = -1;
  https_data->last_bytes_num = -1;
  https_data->last_bytes_time = 0;
  https_data->socket_timeout.data = (void*)https_data;
  uv_timer_init(https_data->loop, &(https_data->socket_timeout));

  // ReadData stuff
  https_data->cur_read_index = 0;
  https_data->is_stream_writable = false;
  https_data->stream_ended = false;
  https_data->data_to_read = false;
  https_data->to_destroy_read_onwrite = false;
  https_data->async_read_onwrite.data = (void*)https_data;
  uv_timer_init(https_data->loop, &(https_data->async_read_onwrite));
  // No Need to read data for following types of requests
  if (https_data->method == HTTPS_GET || https_data->method == HTTPS_DELETE ||
      https_data->method == HTTPS_HEAD || https_data->method == HTTPS_OPTIONS ||
      https_data->method == HTTPS_TRACE)
    https_data->stream_ended = true;

  return https_data;
}

// Destructor
void iotjs_https_destroy(iotjs_https_t* https_data) {
  https_data->URL = NULL;
  IOTJS_RELEASE(https_data);
}

//----------------Utility Functions------------------
void iotjs_https_check_done(iotjs_https_t* https_data) {
  char* done_url;
  CURLMsg* message;
  int pending;
  bool error = false;

  while ((message =
              curl_multi_info_read(https_data->curl_multi_handle, &pending))) {
    switch (message->msg) {
      case CURLMSG_DONE:
        curl_easy_getinfo(message->easy_handle, CURLINFO_EFFECTIVE_URL,
                          &done_url);
        break;
      default:
        error = true;
    }
    if (error) {
      iotjs_jargs_t jarg = iotjs_jargs_create(1);
      char error[] = "Unknown Error has occured.";
      iotjs_string_t jresult_string =
          iotjs_string_create_with_size(error, strlen(error));
      iotjs_jargs_append_string(&jarg, &jresult_string);
      iotjs_https_jcallback(https_data, IOTJS_MAGIC_STRING_ONERROR, &jarg,
                            false);
      iotjs_string_destroy(&jresult_string);
      iotjs_jargs_destroy(&jarg);
    }
    if (https_data->stream_ended) {
      iotjs_https_cleanup(https_data);
    } else {
      if (https_data->to_destroy_read_onwrite) {
        iotjs_https_call_read_onwrite_async(https_data);
      }
      https_data->request_done = true;
    }
    break;
  }
}

// Cleanup before destructor
void iotjs_https_cleanup(iotjs_https_t* https_data) {
  https_data->loop = NULL;

  uv_close((uv_handle_t*)&https_data->timeout,
           (uv_close_cb)iotjs_https_uv_close_callback);
  uv_close((uv_handle_t*)&https_data->socket_timeout,
           (uv_close_cb)iotjs_https_uv_close_callback);
  uv_close((uv_handle_t*)&https_data->async_read_onwrite,
           (uv_close_cb)iotjs_https_uv_close_callback);

  iotjs_https_jcallback(https_data, IOTJS_MAGIC_STRING_ONEND,
                        iotjs_jargs_get_empty(), false);
  iotjs_https_jcallback(https_data, IOTJS_MAGIC_STRING_ONCLOSED,
                        iotjs_jargs_get_empty(), false);

  curl_multi_remove_handle(https_data->curl_multi_handle,
                           https_data->curl_easy_handle);
  curl_easy_cleanup(https_data->curl_easy_handle);
  https_data->curl_easy_handle = NULL;
  curl_multi_cleanup(https_data->curl_multi_handle);
  https_data->curl_multi_handle = NULL;
  curl_slist_free_all(https_data->header_list);

  if (https_data->poll_data != NULL)
    iotjs_https_poll_close_all(https_data->poll_data);

  if (https_data->to_destroy_read_onwrite) {
    const iotjs_jargs_t* jarg = iotjs_jargs_get_empty();
    jerry_value_t jthis = https_data->jthis_native;
    IOTJS_ASSERT(jerry_value_is_function((https_data->read_onwrite)));

    if (!jerry_value_is_undefined((https_data->read_callback)))
      iotjs_make_callback(https_data->read_callback, jthis, jarg);

    iotjs_make_callback(https_data->read_onwrite, jthis, jarg);
    https_data->to_destroy_read_onwrite = false;
    iotjs_string_destroy(&(https_data->read_chunk));
    jerry_release_value((https_data->read_onwrite));
    jerry_release_value((https_data->read_callback));
  }
  return;
}

// Set various parameters of curl handles
void iotjs_https_initialize_curl_opts(iotjs_https_t* https_data) {
  // Setup Some parameters for multi handle
  curl_multi_setopt(https_data->curl_multi_handle, CURLMOPT_SOCKETFUNCTION,
                    iotjs_https_curl_socket_callback);
  curl_multi_setopt(https_data->curl_multi_handle, CURLMOPT_SOCKETDATA,
                    (void*)https_data);
  curl_multi_setopt(https_data->curl_multi_handle, CURLMOPT_TIMERFUNCTION,
                    iotjs_https_curl_start_timeout_callback);
  curl_multi_setopt(https_data->curl_multi_handle, CURLMOPT_TIMERDATA,
                    (void*)https_data);

  curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_PROXY, "");
  curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_HEADERDATA,
                   (void*)https_data);
  curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_WRITEFUNCTION,
                   iotjs_https_curl_write_callback);
  curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_WRITEDATA,
                   (void*)https_data);

  // Read and send data to server only for some request types
  if (https_data->method == HTTPS_POST || https_data->method == HTTPS_PUT ||
      https_data->method == HTTPS_CONNECT) {
    curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_READFUNCTION,
                     iotjs_https_curl_read_callback);
    curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_READDATA,
                     (void*)https_data);
  }

  curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_SOCKOPTFUNCTION,
                   iotjs_https_curl_sockopt_callback);
  curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_SOCKOPTDATA,
                   (void*)https_data);

  curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_URL, https_data->URL);
  https_data->URL = NULL;
  curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_PROTOCOLS,
                   CURLPROTO_HTTP | CURLPROTO_HTTPS);

  if (strlen(https_data->ca) > 0)
    curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_CAINFO,
                     https_data->ca);
  https_data->ca = NULL;
  if (strlen(https_data->cert) > 0)
    curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_SSLCERT,
                     https_data->cert);
  https_data->cert = NULL;
  if (strlen(https_data->key) > 0)
    curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_SSLKEY,
                     https_data->key);
  https_data->key = NULL;
  if (!https_data->reject_unauthorized) {
    curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_SSL_VERIFYHOST, 0);
  }

  // Various request types
  switch (https_data->method) {
    case HTTPS_GET:
      curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_HTTPGET, 1L);
      break;
    case HTTPS_POST:
      curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_POST, 1L);
      break;
    case HTTPS_PUT:
      curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_UPLOAD, 1L);
      break;
    case HTTPS_DELETE:
      curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_CUSTOMREQUEST,
                       "DELETE");
      break;
    case HTTPS_HEAD:
      curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_NOBODY, 1L);
      break;
    case HTTPS_CONNECT:
      curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_CUSTOMREQUEST,
                       "CONNECT");
      break;
    case HTTPS_OPTIONS:
      curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_CUSTOMREQUEST,
                       "OPTIONS");
      break;
    case HTTPS_TRACE:
      curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_CUSTOMREQUEST,
                       "TRACE");
      break;
  }

  curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_HTTP_TRANSFER_DECODING,
                   0L);
}

// Call any property of ClientRequest._Incoming
bool iotjs_https_jcallback(iotjs_https_t* https_data, const char* property,
                           const iotjs_jargs_t* jarg, bool resultvalue) {
  jerry_value_t jthis = https_data->jthis_native;
  bool retval = true;
  if (jerry_value_is_null(jthis))
    return retval;

  jerry_value_t jincoming =
      iotjs_jval_get_property(jthis, IOTJS_MAGIC_STRING__INCOMING);
  jerry_value_t cb = iotjs_jval_get_property(jincoming, property);

  IOTJS_ASSERT(jerry_value_is_function(cb));
  if (!resultvalue) {
    iotjs_make_callback(cb, jincoming, jarg);
  } else {
    jerry_value_t result = iotjs_make_callback_with_result(cb, jincoming, jarg);
    retval = iotjs_jval_as_boolean(result);
    jerry_release_value(result);
  }

  jerry_release_value(jincoming);
  jerry_release_value(cb);
  return retval;
}

// Call onWrite and callback after ClientRequest._write
void iotjs_https_call_read_onwrite(uv_timer_t* timer) {
  iotjs_https_t* https_data = (iotjs_https_t*)(timer->data);

  uv_timer_stop(&(https_data->async_read_onwrite));
  if (jerry_value_is_null(https_data->jthis_native))
    return;
  const iotjs_jargs_t* jarg = iotjs_jargs_get_empty();
  jerry_value_t jthis = https_data->jthis_native;
  IOTJS_ASSERT(jerry_value_is_function((https_data->read_onwrite)));

  if (!jerry_value_is_undefined((https_data->read_callback)))
    iotjs_make_callback(https_data->read_callback, jthis, jarg);

  iotjs_make_callback(https_data->read_onwrite, jthis, jarg);
}

// Call the above method Asynchronously
void iotjs_https_call_read_onwrite_async(iotjs_https_t* https_data) {
  uv_timer_start(&(https_data->async_read_onwrite),
                 iotjs_https_call_read_onwrite, 0, 0);
}

// ------------Functions almost directly called by JS----------
// Add a header to outgoing request
void iotjs_https_add_header(iotjs_https_t* https_data,
                            const char* char_header) {
  https_data->header_list =
      curl_slist_append(https_data->header_list, char_header);
  if (https_data->method == HTTPS_POST || https_data->method == HTTPS_PUT) {
    if (strncmp(char_header, "Content-Length: ", strlen("Content-Length: ")) ==
        0) {
      const char* numberString = char_header + strlen("Content-Length: ");
      https_data->content_length = strtol(numberString, NULL, 10);
    }
  }
}

// Recieved data to write from ClientRequest._write
void iotjs_https_data_to_write(iotjs_https_t* https_data,
                               iotjs_string_t read_chunk,
                               jerry_value_t callback, jerry_value_t onwrite) {
  if (https_data->to_destroy_read_onwrite) {
    https_data->to_destroy_read_onwrite = false;
    iotjs_string_destroy(&(https_data->read_chunk));
    jerry_release_value((https_data->read_onwrite));
    jerry_release_value((https_data->read_callback));
  }

  https_data->read_chunk = read_chunk;
  https_data->data_to_read = true;

  https_data->read_callback = jerry_acquire_value(callback);
  https_data->read_onwrite = jerry_acquire_value(onwrite);
  https_data->to_destroy_read_onwrite = true;

  if (https_data->request_done) {
    iotjs_https_call_read_onwrite_async(https_data);
  } else if (https_data->is_stream_writable) {
    curl_easy_pause(https_data->curl_easy_handle, CURLPAUSE_CONT);
    uv_timer_stop(&(https_data->timeout));
    uv_timer_start(&(https_data->timeout), iotjs_https_uv_timeout_callback, 1,
                   0);
  }
}

// Finish writing all data from ClientRequest Stream
void iotjs_https_finish_request(iotjs_https_t* https_data) {
  https_data->stream_ended = true;
  if (https_data->request_done) {
    iotjs_https_cleanup(https_data);
  } else if (https_data->is_stream_writable) {
    curl_easy_pause(https_data->curl_easy_handle, CURLPAUSE_CONT);
    uv_timer_stop(&(https_data->timeout));
    uv_timer_start(&(https_data->timeout), iotjs_https_uv_timeout_callback, 1,
                   0);
  }
}

// Start sending the request
void iotjs_https_send_request(iotjs_https_t* https_data) {
  // Add all the headers to the easy handle
  curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_HTTPHEADER,
                   https_data->header_list);

  if (https_data->method == HTTPS_POST && https_data->content_length != -1)
    curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_POSTFIELDSIZE,
                     https_data->content_length);
  else if (https_data->method == HTTPS_PUT && https_data->content_length != -1)
    curl_easy_setopt(https_data->curl_easy_handle, CURLOPT_INFILESIZE,
                     https_data->content_length);

  curl_multi_add_handle(https_data->curl_multi_handle,
                        https_data->curl_easy_handle);
}

// Set timeout for request.
void iotjs_https_set_timeout(long ms, iotjs_https_t* https_data) {
  if (ms < 0)
    return;
  https_data->timeout_ms = ms;
  uv_timer_start(&(https_data->socket_timeout),
                 iotjs_https_uv_socket_timeout_callback, 1, (uint64_t)ms);
}


//--------------CURL Callbacks------------------
// Read callback is actually to write data to outgoing request
size_t iotjs_https_curl_read_callback(void* contents, size_t size, size_t nmemb,
                                      void* userp) {
  iotjs_https_t* https_data = (iotjs_https_t*)userp;

  // If stream wasnt made writable yet, make it so.
  if (!https_data->is_stream_writable) {
    https_data->is_stream_writable = true;
    iotjs_https_jcallback(https_data, IOTJS_MAGIC_STRING_ONWRITABLE,
                          iotjs_jargs_get_empty(), false);
  }

  if (https_data->data_to_read) {
    size_t real_size = size * nmemb;
    size_t chunk_size = iotjs_string_size(&(https_data->read_chunk));
    size_t left_to_copy_size = chunk_size - https_data->cur_read_index;

    if (real_size < 1)
      return 0;

    // send some data
    if (https_data->cur_read_index < chunk_size) {
      size_t num_to_copy =
          (left_to_copy_size < real_size) ? left_to_copy_size : real_size;
      const char* buf = iotjs_string_data(&(https_data->read_chunk));
      buf = &buf[https_data->cur_read_index];
      strncpy((char*)contents, buf, num_to_copy);
      https_data->cur_read_index = https_data->cur_read_index + num_to_copy;
      return num_to_copy;
    }

    // Finished sending one chunk of data
    https_data->cur_read_index = 0;
    https_data->data_to_read = false;
    iotjs_https_call_read_onwrite_async(https_data);
  }

  // If the data is sent, and stream hasn't ended, wait for more data
  if (!https_data->stream_ended) {
    return CURL_READFUNC_PAUSE;
  }

  // All done, end the transfer
  return 0;
}

// Pass Curl events on its fd sockets
int iotjs_https_curl_socket_callback(CURL* easy, curl_socket_t sockfd,
                                     int action, void* userp, void* socketp) {
  iotjs_https_t* https_data = (iotjs_https_t*)userp;
  if (action == CURL_POLL_IN || action == CURL_POLL_OUT ||
      action == CURL_POLL_INOUT) {
    iotjs_https_poll_t* poll_data = NULL;

    if (!socketp) {
      poll_data = iotjs_https_poll_create(https_data->loop, sockfd, https_data);
      curl_multi_assign(https_data->curl_multi_handle, sockfd,
                        (void*)poll_data);
      https_data->closing_handles = https_data->closing_handles + 1;
      if (https_data->poll_data == NULL)
        https_data->poll_data = poll_data;
      else
        iotjs_https_poll_append(https_data->poll_data, poll_data);
    } else
      poll_data = (iotjs_https_poll_t*)socketp;

    if (action == CURL_POLL_IN)
      uv_poll_start(&poll_data->poll_handle, UV_READABLE,
                    iotjs_https_uv_poll_callback);
    else if (action == CURL_POLL_OUT)
      uv_poll_start(&poll_data->poll_handle, UV_WRITABLE,
                    iotjs_https_uv_poll_callback);
    else if (action == CURL_POLL_INOUT)
      uv_poll_start(&poll_data->poll_handle, UV_READABLE | UV_WRITABLE,
                    iotjs_https_uv_poll_callback);
  } else {
    if (socketp) {
      iotjs_https_poll_t* poll_data = (iotjs_https_poll_t*)socketp;
      iotjs_https_poll_close(poll_data);
      curl_multi_assign(https_data->curl_multi_handle, sockfd, NULL);
    }
  }
  return 0;
}

// Socket Assigned Callback
int iotjs_https_curl_sockopt_callback(void* userp, curl_socket_t curlfd,
                                      curlsocktype purpose) {
  iotjs_https_t* https_data = (iotjs_https_t*)userp;
  iotjs_https_jcallback(https_data, IOTJS_MAGIC_STRING_ONSOCKET,
                        iotjs_jargs_get_empty(), false);
  return CURL_SOCKOPT_OK;
}

// Curl wants us to signal after timeout
int iotjs_https_curl_start_timeout_callback(CURLM* multi, long timeout_ms,
                                            void* userp) {
  iotjs_https_t* https_data = (iotjs_https_t*)userp;
  if (timeout_ms < 0)
    uv_timer_stop(&(https_data->timeout));
  else {
    if (timeout_ms == 0)
      timeout_ms = 1;
    if ((https_data->timeout_ms != -1) && (timeout_ms > https_data->timeout_ms))
      timeout_ms = https_data->timeout_ms;
    uv_timer_start(&(https_data->timeout), iotjs_https_uv_timeout_callback,
                   (uint64_t)timeout_ms, 0);
  }
  return 0;
}

// Write Callback is actually to read data from incoming response
size_t iotjs_https_curl_write_callback(void* contents, size_t size,
                                       size_t nmemb, void* userp) {
  iotjs_https_t* https_data = (iotjs_https_t*)userp;
  size_t real_size = size * nmemb;
  if (jerry_value_is_null(https_data->jthis_native))
    return real_size - 1;

  iotjs_jargs_t jarg = iotjs_jargs_create(1);

  jerry_value_t jbuffer = iotjs_bufferwrap_create_buffer((size_t)real_size);
  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);
  iotjs_bufferwrap_copy(buffer_wrap, contents, (size_t)real_size);

  iotjs_jargs_append_jval(&jarg, jbuffer);

  bool result =
      iotjs_https_jcallback(https_data, IOTJS_MAGIC_STRING_ONDATA, &jarg, true);

  jerry_release_value(jbuffer);
  iotjs_jargs_destroy(&jarg);

  if (!result) {
    return real_size - 1;
  }

  return real_size;
}


//--------------LibTUV Callbacks------------------
// Callback called on closing handles during cleanup
void iotjs_https_uv_close_callback(uv_handle_t* handle) {
  iotjs_https_t* https_data = (iotjs_https_t*)handle->data;
  https_data->closing_handles = https_data->closing_handles - 1;
  if (https_data->closing_handles <= 0) {
    if (https_data->poll_data != NULL)
      iotjs_https_poll_destroy(https_data->poll_data);
    jerry_release_value(https_data->jthis_native);
  }
}

// Callback called when poll detects actions on FD
void iotjs_https_uv_poll_callback(uv_poll_t* poll, int status, int events) {
  iotjs_https_poll_t* poll_data = (iotjs_https_poll_t*)poll->data;
  iotjs_https_t* https_data = (iotjs_https_t*)poll_data->https_data;

  int flags = 0;
  if (status < 0)
    flags = CURL_CSELECT_ERR;
  if (!status && events & UV_READABLE)
    flags |= CURL_CSELECT_IN;
  if (!status && events & UV_WRITABLE)
    flags |= CURL_CSELECT_OUT;
  int running_handles;
  curl_multi_socket_action(https_data->curl_multi_handle, poll_data->sockfd,
                           flags, &running_handles);
  iotjs_https_check_done(https_data);
}

// This function is for signalling to curl a given time has passed.
// This timeout is usually given by curl itself.
void iotjs_https_uv_timeout_callback(uv_timer_t* timer) {
  iotjs_https_t* https_data = (iotjs_https_t*)(timer->data);
  uv_timer_stop(timer);
  curl_multi_socket_action(https_data->curl_multi_handle, CURL_SOCKET_TIMEOUT,
                           0, &https_data->running_handles);
  iotjs_https_check_done(https_data);
}

// Callback called to check if request has timed out
void iotjs_https_uv_socket_timeout_callback(uv_timer_t* timer) {
  iotjs_https_t* https_data = (iotjs_https_t*)(timer->data);
  double download_bytes = 0;
  double upload_bytes = 0;
  uint64_t total_time_ms = 0;

  if (https_data->timeout_ms != -1) {
    curl_easy_getinfo(https_data->curl_easy_handle, CURLINFO_SIZE_DOWNLOAD,
                      &download_bytes);
    curl_easy_getinfo(https_data->curl_easy_handle, CURLINFO_SIZE_UPLOAD,
                      &upload_bytes);
    total_time_ms = uv_now(https_data->loop);
    double total_bytes = download_bytes + upload_bytes;

    if (https_data->last_bytes_num == total_bytes) {
      if (total_time_ms >
          ((uint64_t)https_data->timeout_ms + https_data->last_bytes_time)) {
        if (!https_data->request_done) {
          iotjs_https_jcallback(https_data, IOTJS_MAGIC_STRING_ONTIMEOUT,
                                iotjs_jargs_get_empty(), false);
        }
        uv_timer_stop(&(https_data->socket_timeout));
      }
    } else {
      https_data->last_bytes_num = total_bytes;
      https_data->last_bytes_time = total_time_ms;
    }
  }
}

//--------------https_poll Functions------------------
iotjs_https_poll_t* iotjs_https_poll_create(uv_loop_t* loop,
                                            curl_socket_t sockfd,
                                            iotjs_https_t* https_data) {
  iotjs_https_poll_t* poll_data = IOTJS_ALLOC(iotjs_https_poll_t);
  poll_data->sockfd = sockfd;
  poll_data->poll_handle.data = poll_data;
  poll_data->https_data = https_data;
  poll_data->closing = false;
  poll_data->next = NULL;
  uv_poll_init_socket(loop, &poll_data->poll_handle, sockfd);
  return poll_data;
}

void iotjs_https_poll_append(iotjs_https_poll_t* head,
                             iotjs_https_poll_t* poll_data) {
  iotjs_https_poll_t* current = head;
  iotjs_https_poll_t* next = current->next;
  while (next != NULL) {
    current = next;
    next = current->next;
  }
  current->next = poll_data;
}

void iotjs_https_poll_close(iotjs_https_poll_t* poll_data) {
  if (poll_data->closing == false) {
    poll_data->closing = true;
    uv_poll_stop(&poll_data->poll_handle);
    poll_data->poll_handle.data = poll_data->https_data;
    uv_close((uv_handle_t*)&poll_data->poll_handle,
             iotjs_https_uv_close_callback);
  }
  return;
}

void iotjs_https_poll_close_all(iotjs_https_poll_t* head) {
  iotjs_https_poll_t* current = head;
  while (current != NULL) {
    iotjs_https_poll_close(current);
    current = current->next;
  }
}

void iotjs_https_poll_destroy(iotjs_https_poll_t* poll_data) {
  if (poll_data->next != NULL) {
    iotjs_https_poll_destroy(poll_data->next);
  }
  IOTJS_RELEASE(poll_data);
}

// ------------JHANDLERS----------------

JS_FUNCTION(createRequest) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, object);

  const jerry_value_t joptions = JS_GET_ARG(0, object);

  jerry_value_t jhost =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_HOST);
  iotjs_string_t host = iotjs_jval_as_string(jhost);
  jerry_release_value(jhost);

  jerry_value_t jmethod =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_METHOD);
  iotjs_string_t method = iotjs_jval_as_string(jmethod);
  jerry_release_value(jmethod);

  jerry_value_t jca = iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_CA);
  iotjs_string_t ca = iotjs_jval_as_string(jca);
  jerry_release_value(jca);

  jerry_value_t jcert =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_CERT);
  iotjs_string_t cert = iotjs_jval_as_string(jcert);
  jerry_release_value(jcert);

  jerry_value_t jkey =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_KEY);
  iotjs_string_t key = iotjs_jval_as_string(jkey);
  jerry_release_value(jkey);

  jerry_value_t jreject_unauthorized =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_REJECTUNAUTHORIZED);
  const bool reject_unauthorized = iotjs_jval_as_boolean(jreject_unauthorized);

  if (curl_global_init(CURL_GLOBAL_SSL)) {
    return jerry_create_null();
  }

  iotjs_https_t* https_data =
      iotjs_https_create(iotjs_string_data(&host), iotjs_string_data(&method),
                         iotjs_string_data(&ca), iotjs_string_data(&cert),
                         iotjs_string_data(&key), reject_unauthorized,
                         joptions);

  iotjs_https_initialize_curl_opts(https_data);

  iotjs_string_destroy(&host);
  iotjs_string_destroy(&method);
  iotjs_string_destroy(&ca);
  iotjs_string_destroy(&cert);
  iotjs_string_destroy(&key);
  return jerry_create_null();
}

JS_FUNCTION(addHeader) {
  DJS_CHECK_THIS();

  DJS_CHECK_ARGS(2, string, object);
  iotjs_string_t header = JS_GET_ARG(0, string);
  const char* char_header = iotjs_string_data(&header);

  jerry_value_t jarg = JS_GET_ARG(1, object);
  iotjs_https_t* https_data =
      (iotjs_https_t*)iotjs_jval_get_object_native_handle(jarg);
  iotjs_https_add_header(https_data, char_header);

  iotjs_string_destroy(&header);
  return jerry_create_null();
}

JS_FUNCTION(sendRequest) {
  DJS_CHECK_THIS();

  DJS_CHECK_ARG(0, object);
  jerry_value_t jarg = JS_GET_ARG(0, object);
  iotjs_https_t* https_data =
      (iotjs_https_t*)iotjs_jval_get_object_native_handle(jarg);
  iotjs_https_send_request(https_data);
  return jerry_create_null();
}

JS_FUNCTION(setTimeout) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(2, number, object);

  double ms = JS_GET_ARG(0, number);
  jerry_value_t jarg = JS_GET_ARG(1, object);

  iotjs_https_t* https_data =
      (iotjs_https_t*)iotjs_jval_get_object_native_handle(jarg);
  iotjs_https_set_timeout((long)ms, https_data);

  return jerry_create_null();
}

JS_FUNCTION(_write) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(2, object, string);
  // Argument 3 can be null, so not checked directly, checked later
  DJS_CHECK_ARG(3, function);

  jerry_value_t jarg = JS_GET_ARG(0, object);
  iotjs_string_t read_chunk = JS_GET_ARG(1, string);

  jerry_value_t callback = jargv[2];
  jerry_value_t onwrite = JS_GET_ARG(3, function);

  iotjs_https_t* https_data =
      (iotjs_https_t*)iotjs_jval_get_object_native_handle(jarg);
  iotjs_https_data_to_write(https_data, read_chunk, callback, onwrite);

  // readchunk was copied to https_data, hence not destroyed.
  return jerry_create_null();
}

JS_FUNCTION(finishRequest) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARG(0, object);

  jerry_value_t jarg = JS_GET_ARG(0, object);
  iotjs_https_t* https_data =
      (iotjs_https_t*)iotjs_jval_get_object_native_handle(jarg);
  iotjs_https_finish_request(https_data);

  return jerry_create_null();
}

JS_FUNCTION(Abort) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARG(0, object);

  jerry_value_t jarg = JS_GET_ARG(0, object);
  iotjs_https_t* https_data =
      (iotjs_https_t*)iotjs_jval_get_object_native_handle(jarg);
  iotjs_https_cleanup(https_data);

  return jerry_create_null();
}

jerry_value_t InitHttps() {
  jerry_value_t https = jerry_create_object();

  iotjs_jval_set_method(https, IOTJS_MAGIC_STRING_CREATEREQUEST, createRequest);
  iotjs_jval_set_method(https, IOTJS_MAGIC_STRING_ADDHEADER, addHeader);
  iotjs_jval_set_method(https, IOTJS_MAGIC_STRING_SENDREQUEST, sendRequest);
  iotjs_jval_set_method(https, IOTJS_MAGIC_STRING_SETTIMEOUT, setTimeout);
  iotjs_jval_set_method(https, IOTJS_MAGIC_STRING__WRITE, _write);
  iotjs_jval_set_method(https, IOTJS_MAGIC_STRING_FINISHREQUEST, finishRequest);
  iotjs_jval_set_method(https, IOTJS_MAGIC_STRING_ABORT, Abort);

  return https;
}
