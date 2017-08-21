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
#include "iotjs_objectwrap.h"
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>

void iotjs_https_destroy(iotjs_https_t* https_data);
IOTJS_DEFINE_NATIVE_HANDLE_INFO(https);

//-------------Constructor------------
iotjs_https_t* iotjs_https_create(const char* URL, const char* method,
                                  const char* ca, const char* cert,
                                  const char* key,
                                  const bool reject_unauthorized,
                                  const iotjs_jval_t* jthis) {
  iotjs_https_t* https_data = IOTJS_ALLOC(iotjs_https_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_https_t, https_data);

  // Original Request Details
  _this->URL = URL;
  _this->header_list = NULL;
  if (strcmp(method, STRING_GET) == 0)
    _this->method = HTTPS_GET;
  else if (strcmp(method, STRING_POST) == 0)
    _this->method = HTTPS_POST;
  else if (strcmp(method, STRING_PUT) == 0)
    _this->method = HTTPS_PUT;
  else if (strcmp(method, STRING_DELETE) == 0)
    _this->method = HTTPS_DELETE;
  else if (strcmp(method, STRING_HEAD) == 0)
    _this->method = HTTPS_HEAD;
  else if (strcmp(method, STRING_CONNECT) == 0)
    _this->method = HTTPS_CONNECT;
  else if (strcmp(method, STRING_OPTIONS) == 0)
    _this->method = HTTPS_OPTIONS;
  else if (strcmp(method, STRING_TRACE) == 0)
    _this->method = HTTPS_TRACE;
  else {
    IOTJS_ASSERT(0);
  }

  // TLS certs stuff
  _this->ca = ca;
  _this->cert = cert;
  _this->key = key;
  _this->reject_unauthorized = reject_unauthorized;
  // Content Length stuff
  _this->content_length = -1;

  // Handles
  _this->loop = iotjs_environment_loop(iotjs_environment_get());
  _this->jthis_native = iotjs_jval_create_copied(jthis);
  iotjs_jval_set_object_native_handle(&(_this->jthis_native),
                                      (uintptr_t)https_data,
                                      &https_native_info);
  _this->curl_multi_handle = curl_multi_init();
  _this->curl_easy_handle = curl_easy_init();
  _this->timeout.data = (void*)https_data;
  uv_timer_init(_this->loop, &(_this->timeout));
  _this->request_done = false;
  _this->closing_handles = 3;
  _this->poll_data = NULL;

  // Timeout stuff
  _this->timeout_ms = -1;
  _this->last_bytes_num = -1;
  _this->last_bytes_time = 0;
  _this->socket_timeout.data = (void*)https_data;
  uv_timer_init(_this->loop, &(_this->socket_timeout));

  // ReadData stuff
  _this->cur_read_index = 0;
  _this->is_stream_writable = false;
  _this->stream_ended = false;
  _this->data_to_read = false;
  _this->to_destroy_read_onwrite = false;
  _this->async_read_onwrite.data = (void*)https_data;
  uv_timer_init(_this->loop, &(_this->async_read_onwrite));
  // No Need to read data for following types of requests
  if (_this->method == HTTPS_GET || _this->method == HTTPS_DELETE ||
      _this->method == HTTPS_HEAD || _this->method == HTTPS_OPTIONS ||
      _this->method == HTTPS_TRACE)
    _this->stream_ended = true;

  return https_data;
}

// Destructor
void iotjs_https_destroy(iotjs_https_t* https_data) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_https_t, https_data);
  // To shutup unused variable _this warning
  _this->URL = NULL;
  IOTJS_RELEASE(https_data);
}

//----------------Utility Functions------------------
void iotjs_https_check_done(iotjs_https_t* https_data) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_t, https_data);
  char* done_url;
  CURLMsg* message;
  int pending;
  bool error = false;

  while ((message = curl_multi_info_read(_this->curl_multi_handle, &pending))) {
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
    if (_this->stream_ended) {
      iotjs_https_cleanup(https_data);
    } else {
      if (_this->to_destroy_read_onwrite) {
        iotjs_https_call_read_onwrite_async(https_data);
      }
      _this->request_done = true;
    }
    break;
  }
}

// Cleanup before destructor
void iotjs_https_cleanup(iotjs_https_t* https_data) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_t, https_data);
  _this->loop = NULL;

  uv_close((uv_handle_t*)&_this->timeout,
           (uv_close_cb)iotjs_https_uv_close_callback);
  uv_close((uv_handle_t*)&_this->socket_timeout,
           (uv_close_cb)iotjs_https_uv_close_callback);
  uv_close((uv_handle_t*)&_this->async_read_onwrite,
           (uv_close_cb)iotjs_https_uv_close_callback);

  iotjs_https_jcallback(https_data, IOTJS_MAGIC_STRING_ONEND,
                        iotjs_jargs_get_empty(), false);
  iotjs_https_jcallback(https_data, IOTJS_MAGIC_STRING_ONCLOSED,
                        iotjs_jargs_get_empty(), false);

  curl_multi_remove_handle(_this->curl_multi_handle, _this->curl_easy_handle);
  curl_easy_cleanup(_this->curl_easy_handle);
  _this->curl_easy_handle = NULL;
  curl_multi_cleanup(_this->curl_multi_handle);
  _this->curl_multi_handle = NULL;
  curl_slist_free_all(_this->header_list);

  if (_this->poll_data != NULL)
    iotjs_https_poll_close_all(_this->poll_data);

  if (_this->to_destroy_read_onwrite) {
    const iotjs_jargs_t* jarg = iotjs_jargs_get_empty();
    const iotjs_jval_t* jthis = &(_this->jthis_native);
    IOTJS_ASSERT(iotjs_jval_is_function(&(_this->read_onwrite)));

    if (!iotjs_jval_is_undefined(&(_this->read_callback)))
      iotjs_make_callback(&(_this->read_callback), jthis, jarg);

    iotjs_make_callback(&(_this->read_onwrite), jthis, jarg);
    _this->to_destroy_read_onwrite = false;
    iotjs_string_destroy(&(_this->read_chunk));
    iotjs_jval_destroy(&(_this->read_onwrite));
    iotjs_jval_destroy(&(_this->read_callback));
  }
  return;
}

CURLM* iotjs_https_get_multi_handle(iotjs_https_t* https_data) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_t, https_data);
  return _this->curl_multi_handle;
}

// Set various parameters of curl handles
void iotjs_https_initialize_curl_opts(iotjs_https_t* https_data) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_t, https_data);

  // Setup Some parameters for multi handle
  curl_multi_setopt(_this->curl_multi_handle, CURLMOPT_SOCKETFUNCTION,
                    iotjs_https_curl_socket_callback);
  curl_multi_setopt(_this->curl_multi_handle, CURLMOPT_SOCKETDATA,
                    (void*)https_data);
  curl_multi_setopt(_this->curl_multi_handle, CURLMOPT_TIMERFUNCTION,
                    iotjs_https_curl_start_timeout_callback);
  curl_multi_setopt(_this->curl_multi_handle, CURLMOPT_TIMERDATA,
                    (void*)https_data);

  curl_easy_setopt(_this->curl_easy_handle, CURLOPT_PROXY, "");
  curl_easy_setopt(_this->curl_easy_handle, CURLOPT_HEADERDATA,
                   (void*)https_data);
  curl_easy_setopt(_this->curl_easy_handle, CURLOPT_WRITEFUNCTION,
                   iotjs_https_curl_write_callback);
  curl_easy_setopt(_this->curl_easy_handle, CURLOPT_WRITEDATA,
                   (void*)https_data);

  // Read and send data to server only for some request types
  if (_this->method == HTTPS_POST || _this->method == HTTPS_PUT ||
      _this->method == HTTPS_CONNECT) {
    curl_easy_setopt(_this->curl_easy_handle, CURLOPT_READFUNCTION,
                     iotjs_https_curl_read_callback);
    curl_easy_setopt(_this->curl_easy_handle, CURLOPT_READDATA,
                     (void*)https_data);
  }

  curl_easy_setopt(_this->curl_easy_handle, CURLOPT_SOCKOPTFUNCTION,
                   iotjs_https_curl_sockopt_callback);
  curl_easy_setopt(_this->curl_easy_handle, CURLOPT_SOCKOPTDATA,
                   (void*)https_data);

  curl_easy_setopt(_this->curl_easy_handle, CURLOPT_URL, _this->URL);
  _this->URL = NULL;
  curl_easy_setopt(_this->curl_easy_handle, CURLOPT_PROTOCOLS,
                   CURLPROTO_HTTP | CURLPROTO_HTTPS);

  if (strlen(_this->ca) > 0)
    curl_easy_setopt(_this->curl_easy_handle, CURLOPT_CAINFO, _this->ca);
  _this->ca = NULL;
  if (strlen(_this->cert) > 0)
    curl_easy_setopt(_this->curl_easy_handle, CURLOPT_SSLCERT, _this->cert);
  _this->cert = NULL;
  if (strlen(_this->key) > 0)
    curl_easy_setopt(_this->curl_easy_handle, CURLOPT_SSLKEY, _this->key);
  _this->key = NULL;
  if (!_this->reject_unauthorized) {
    curl_easy_setopt(_this->curl_easy_handle, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(_this->curl_easy_handle, CURLOPT_SSL_VERIFYHOST, 0);
  }

  // Various request types
  switch (_this->method) {
    case HTTPS_GET:
      curl_easy_setopt(_this->curl_easy_handle, CURLOPT_HTTPGET, 1L);
      break;
    case HTTPS_POST:
      curl_easy_setopt(_this->curl_easy_handle, CURLOPT_POST, 1L);
      break;
    case HTTPS_PUT:
      curl_easy_setopt(_this->curl_easy_handle, CURLOPT_UPLOAD, 1L);
      break;
    case HTTPS_DELETE:
      curl_easy_setopt(_this->curl_easy_handle, CURLOPT_CUSTOMREQUEST,
                       "DELETE");
      break;
    case HTTPS_HEAD:
      curl_easy_setopt(_this->curl_easy_handle, CURLOPT_NOBODY, 1L);
      break;
    case HTTPS_CONNECT:
      curl_easy_setopt(_this->curl_easy_handle, CURLOPT_CUSTOMREQUEST,
                       "CONNECT");
      break;
    case HTTPS_OPTIONS:
      curl_easy_setopt(_this->curl_easy_handle, CURLOPT_CUSTOMREQUEST,
                       "OPTIONS");
      break;
    case HTTPS_TRACE:
      curl_easy_setopt(_this->curl_easy_handle, CURLOPT_CUSTOMREQUEST, "TRACE");
      break;
  }

  curl_easy_setopt(_this->curl_easy_handle, CURLOPT_HTTP_TRANSFER_DECODING, 0L);
}

// Get https.ClientRequest from struct
iotjs_jval_t* iotjs_https_jthis_from_https(iotjs_https_t* https_data) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_t, https_data);
  return &(_this->jthis_native);
}

// Call any property of ClientRequest._Incoming
bool iotjs_https_jcallback(iotjs_https_t* https_data, const char* property,
                           const iotjs_jargs_t* jarg, bool resultvalue) {
  iotjs_jval_t* jthis = iotjs_https_jthis_from_https(https_data);
  bool retval = true;
  if (iotjs_jval_is_null(jthis))
    return retval;

  iotjs_jval_t jincoming =
      iotjs_jval_get_property(jthis, IOTJS_MAGIC_STRING__INCOMING);
  iotjs_jval_t cb = iotjs_jval_get_property(&jincoming, property);

  IOTJS_ASSERT(iotjs_jval_is_function(&cb));
  if (!resultvalue) {
    iotjs_make_callback(&cb, &jincoming, jarg);
  } else {
    iotjs_jval_t result =
        iotjs_make_callback_with_result(&cb, &jincoming, jarg);
    retval = iotjs_jval_as_boolean(&result);
    iotjs_jval_destroy(&result);
  }

  iotjs_jval_destroy(&jincoming);
  iotjs_jval_destroy(&cb);
  return retval;
}

// Call onWrite and callback after ClientRequest._write
void iotjs_https_call_read_onwrite(uv_timer_t* timer) {
  iotjs_https_t* https_data = (iotjs_https_t*)(timer->data);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_t, https_data);

  uv_timer_stop(&(_this->async_read_onwrite));
  if (iotjs_jval_is_null(&_this->jthis_native))
    return;
  const iotjs_jargs_t* jarg = iotjs_jargs_get_empty();
  const iotjs_jval_t* jthis = &(_this->jthis_native);
  IOTJS_ASSERT(iotjs_jval_is_function(&(_this->read_onwrite)));

  if (!iotjs_jval_is_undefined(&(_this->read_callback)))
    iotjs_make_callback(&(_this->read_callback), jthis, jarg);

  iotjs_make_callback(&(_this->read_onwrite), jthis, jarg);
}

// Call the above method Asynchronously
void iotjs_https_call_read_onwrite_async(iotjs_https_t* https_data) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_t, https_data);
  uv_timer_start(&(_this->async_read_onwrite), iotjs_https_call_read_onwrite, 0,
                 0);
}

// ------------Functions almost directly called by JS----------
// Add a header to outgoing request
void iotjs_https_add_header(iotjs_https_t* https_data,
                            const char* char_header) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_t, https_data);
  _this->header_list = curl_slist_append(_this->header_list, char_header);
  if (_this->method == HTTPS_POST || _this->method == HTTPS_PUT) {
    if (strncmp(char_header, "Content-Length: ", strlen("Content-Length: ")) ==
        0) {
      const char* numberString = char_header + strlen("Content-Length: ");
      _this->content_length = strtol(numberString, NULL, 10);
    }
  }
}

// Recieved data to write from ClientRequest._write
void iotjs_https_data_to_write(iotjs_https_t* https_data,
                               iotjs_string_t read_chunk,
                               const iotjs_jval_t* callback,
                               const iotjs_jval_t* onwrite) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_t, https_data);

  if (_this->to_destroy_read_onwrite) {
    _this->to_destroy_read_onwrite = false;
    iotjs_string_destroy(&(_this->read_chunk));
    iotjs_jval_destroy(&(_this->read_onwrite));
    iotjs_jval_destroy(&(_this->read_callback));
  }

  _this->read_chunk = read_chunk;
  _this->data_to_read = true;

  _this->read_callback = iotjs_jval_create_copied(callback);
  _this->read_onwrite = iotjs_jval_create_copied(onwrite);
  _this->to_destroy_read_onwrite = true;

  if (_this->request_done) {
    iotjs_https_call_read_onwrite_async(https_data);
  } else if (_this->is_stream_writable) {
    curl_easy_pause(_this->curl_easy_handle, CURLPAUSE_CONT);
    uv_timer_stop(&(_this->timeout));
    uv_timer_start(&(_this->timeout), iotjs_https_uv_timeout_callback, 1, 0);
  }
}

// Finish writing all data from ClientRequest Stream
void iotjs_https_finish_request(iotjs_https_t* https_data) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_t, https_data);
  _this->stream_ended = true;
  if (_this->request_done) {
    iotjs_https_cleanup(https_data);
  } else if (_this->is_stream_writable) {
    curl_easy_pause(_this->curl_easy_handle, CURLPAUSE_CONT);
    uv_timer_stop(&(_this->timeout));
    uv_timer_start(&(_this->timeout), iotjs_https_uv_timeout_callback, 1, 0);
  }
}

// Start sending the request
void iotjs_https_send_request(iotjs_https_t* https_data) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_t, https_data);
  // Add all the headers to the easy handle
  curl_easy_setopt(_this->curl_easy_handle, CURLOPT_HTTPHEADER,
                   _this->header_list);

  if (_this->method == HTTPS_POST && _this->content_length != -1)
    curl_easy_setopt(_this->curl_easy_handle, CURLOPT_POSTFIELDSIZE,
                     _this->content_length);
  else if (_this->method == HTTPS_PUT && _this->content_length != -1)
    curl_easy_setopt(_this->curl_easy_handle, CURLOPT_INFILESIZE,
                     _this->content_length);

  curl_multi_add_handle(_this->curl_multi_handle, _this->curl_easy_handle);
}

// Set timeout for request.
void iotjs_https_set_timeout(long ms, iotjs_https_t* https_data) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_t, https_data);
  if (ms < 0)
    return;
  _this->timeout_ms = ms;
  uv_timer_start(&(_this->socket_timeout),
                 iotjs_https_uv_socket_timeout_callback, 1, (uint64_t)ms);
}


//--------------CURL Callbacks------------------
// Read callback is actually to write data to outgoing request
size_t iotjs_https_curl_read_callback(void* contents, size_t size, size_t nmemb,
                                      void* userp) {
  iotjs_https_t* https_data = (iotjs_https_t*)userp;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_t, https_data);

  // If stream wasnt made writable yet, make it so.
  if (!_this->is_stream_writable) {
    _this->is_stream_writable = true;
    iotjs_https_jcallback(https_data, IOTJS_MAGIC_STRING_ONWRITABLE,
                          iotjs_jargs_get_empty(), false);
  }

  if (_this->data_to_read) {
    size_t real_size = size * nmemb;
    size_t chunk_size = iotjs_string_size(&(_this->read_chunk));
    size_t left_to_copy_size = chunk_size - _this->cur_read_index;

    if (real_size < 1)
      return 0;

    // send some data
    if (_this->cur_read_index < chunk_size) {
      size_t num_to_copy =
          (left_to_copy_size < real_size) ? left_to_copy_size : real_size;
      const char* buf = iotjs_string_data(&(_this->read_chunk));
      buf = &buf[_this->cur_read_index];
      strncpy((char*)contents, buf, num_to_copy);
      _this->cur_read_index = _this->cur_read_index + num_to_copy;
      return num_to_copy;
    }

    // Finished sending one chunk of data
    _this->cur_read_index = 0;
    _this->data_to_read = false;
    iotjs_https_call_read_onwrite_async(https_data);
  }

  // If the data is sent, and stream hasn't ended, wait for more data
  if (!_this->stream_ended) {
    return CURL_READFUNC_PAUSE;
  }

  // All done, end the transfer
  return 0;
}

// Pass Curl events on its fd sockets
int iotjs_https_curl_socket_callback(CURL* easy, curl_socket_t sockfd,
                                     int action, void* userp, void* socketp) {
  iotjs_https_t* https_data = (iotjs_https_t*)userp;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_t, https_data);
  if (action == CURL_POLL_IN || action == CURL_POLL_OUT ||
      action == CURL_POLL_INOUT) {
    iotjs_https_poll_t* poll_data = NULL;

    if (!socketp) {
      poll_data = iotjs_https_poll_create(_this->loop, sockfd, https_data);
      curl_multi_assign(_this->curl_multi_handle, sockfd, (void*)poll_data);
      _this->closing_handles = _this->closing_handles + 1;
      if (_this->poll_data == NULL)
        _this->poll_data = poll_data;
      else
        iotjs_https_poll_append(_this->poll_data, poll_data);
    } else
      poll_data = (iotjs_https_poll_t*)socketp;

    if (action == CURL_POLL_IN)
      uv_poll_start(iotjs_https_poll_get_poll_handle(poll_data), UV_READABLE,
                    iotjs_https_uv_poll_callback);
    else if (action == CURL_POLL_OUT)
      uv_poll_start(iotjs_https_poll_get_poll_handle(poll_data), UV_WRITABLE,
                    iotjs_https_uv_poll_callback);
    else if (action == CURL_POLL_INOUT)
      uv_poll_start(iotjs_https_poll_get_poll_handle(poll_data),
                    UV_READABLE | UV_WRITABLE, iotjs_https_uv_poll_callback);
  } else {
    if (socketp) {
      iotjs_https_poll_t* poll_data = (iotjs_https_poll_t*)socketp;
      iotjs_https_poll_close(poll_data);
      curl_multi_assign(_this->curl_multi_handle, sockfd, NULL);
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
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_t, https_data);
  if (timeout_ms < 0)
    uv_timer_stop(&(_this->timeout));
  else {
    if (timeout_ms == 0)
      timeout_ms = 1;
    if ((_this->timeout_ms != -1) && (timeout_ms > _this->timeout_ms))
      timeout_ms = _this->timeout_ms;
    uv_timer_start(&(_this->timeout), iotjs_https_uv_timeout_callback,
                   (uint64_t)timeout_ms, 0);
  }
  return 0;
}

// Write Callback is actually to read data from incoming response
size_t iotjs_https_curl_write_callback(void* contents, size_t size,
                                       size_t nmemb, void* userp) {
  iotjs_https_t* https_data = (iotjs_https_t*)userp;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_t, https_data);
  size_t real_size = size * nmemb;
  if (iotjs_jval_is_null(&_this->jthis_native))
    return real_size - 1;
  iotjs_jargs_t jarg = iotjs_jargs_create(1);
  iotjs_jval_t jresult_arr = iotjs_jval_create_byte_array(real_size, contents);
  iotjs_jargs_append_jval(&jarg, &jresult_arr);

  bool result =
      iotjs_https_jcallback(https_data, IOTJS_MAGIC_STRING_ONDATA, &jarg, true);

  iotjs_jval_destroy(&jresult_arr);
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
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_t, https_data);
  _this->closing_handles = _this->closing_handles - 1;
  if (_this->closing_handles <= 0) {
    if (_this->poll_data != NULL)
      iotjs_https_poll_destroy(_this->poll_data);
    iotjs_jval_destroy(&_this->jthis_native);
  }
}

// Callback called when poll detects actions on FD
void iotjs_https_uv_poll_callback(uv_poll_t* poll, int status, int events) {
  iotjs_https_poll_t* poll_data = (iotjs_https_poll_t*)poll->data;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_poll_t, poll_data);
  iotjs_https_t* https_data = (iotjs_https_t*)_this->https_data;

  int flags = 0;
  if (status < 0)
    flags = CURL_CSELECT_ERR;
  if (!status && events & UV_READABLE)
    flags |= CURL_CSELECT_IN;
  if (!status && events & UV_WRITABLE)
    flags |= CURL_CSELECT_OUT;
  int running_handles;
  curl_multi_socket_action(iotjs_https_get_multi_handle(https_data),
                           _this->sockfd, flags, &running_handles);
  iotjs_https_check_done(https_data);
}

// This function is for signalling to curl a given time has passed.
// This timeout is usually given by curl itself.
void iotjs_https_uv_timeout_callback(uv_timer_t* timer) {
  iotjs_https_t* https_data = (iotjs_https_t*)(timer->data);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_t, https_data);
  uv_timer_stop(timer);
  curl_multi_socket_action(_this->curl_multi_handle, CURL_SOCKET_TIMEOUT, 0,
                           &_this->running_handles);
  iotjs_https_check_done(https_data);
}

// Callback called to check if request has timed out
void iotjs_https_uv_socket_timeout_callback(uv_timer_t* timer) {
  iotjs_https_t* https_data = (iotjs_https_t*)(timer->data);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_t, https_data);
  double download_bytes = 0;
  double upload_bytes = 0;
  uint64_t total_time_ms = 0;

  if (_this->timeout_ms != -1) {
    curl_easy_getinfo(_this->curl_easy_handle, CURLINFO_SIZE_DOWNLOAD,
                      &download_bytes);
    curl_easy_getinfo(_this->curl_easy_handle, CURLINFO_SIZE_UPLOAD,
                      &upload_bytes);
    total_time_ms = uv_now(_this->loop);
    double total_bytes = download_bytes + upload_bytes;

    if (_this->last_bytes_num == total_bytes) {
      if (total_time_ms >
          ((uint64_t)_this->timeout_ms + _this->last_bytes_time)) {
        if (!_this->request_done) {
          iotjs_https_jcallback(https_data, IOTJS_MAGIC_STRING_ONTIMEOUT,
                                iotjs_jargs_get_empty(), false);
        }
        uv_timer_stop(&(_this->socket_timeout));
      }
    } else {
      _this->last_bytes_num = total_bytes;
      _this->last_bytes_time = total_time_ms;
    }
  }
}

//--------------https_poll Functions------------------
iotjs_https_poll_t* iotjs_https_poll_create(uv_loop_t* loop,
                                            curl_socket_t sockfd,
                                            iotjs_https_t* https_data) {
  iotjs_https_poll_t* poll_data = IOTJS_ALLOC(iotjs_https_poll_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_https_poll_t, poll_data);
  _this->sockfd = sockfd;
  _this->poll_handle.data = poll_data;
  _this->https_data = https_data;
  _this->closing = false;
  _this->next = NULL;
  uv_poll_init_socket(loop, &_this->poll_handle, sockfd);
  return poll_data;
}

void iotjs_https_poll_append(iotjs_https_poll_t* head,
                             iotjs_https_poll_t* poll_data) {
  iotjs_https_poll_t* current = head;
  iotjs_https_poll_t* next = iotjs_https_poll_get_next(current);
  while (next != NULL) {
    current = next;
    next = iotjs_https_poll_get_next(current);
  }
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_poll_t, current);
  _this->next = poll_data;
}

iotjs_https_poll_t* iotjs_https_poll_get_next(iotjs_https_poll_t* poll_data) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_poll_t, poll_data);
  return _this->next;
}

uv_poll_t* iotjs_https_poll_get_poll_handle(iotjs_https_poll_t* poll_data) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_poll_t, poll_data);
  return &_this->poll_handle;
}

void iotjs_https_poll_close(iotjs_https_poll_t* poll_data) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_https_poll_t, poll_data);
  if (_this->closing == false) {
    _this->closing = true;
    uv_poll_stop(&_this->poll_handle);
    _this->poll_handle.data = _this->https_data;
    uv_close((uv_handle_t*)&_this->poll_handle, iotjs_https_uv_close_callback);
  }
  return;
}

void iotjs_https_poll_close_all(iotjs_https_poll_t* head) {
  iotjs_https_poll_t* current = head;
  while (current != NULL) {
    iotjs_https_poll_close(current);
    current = iotjs_https_poll_get_next(current);
  }
}

void iotjs_https_poll_destroy(iotjs_https_poll_t* poll_data) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_https_poll_t, poll_data);
  if (_this->next != NULL) {
    iotjs_https_poll_destroy(_this->next);
  }
  IOTJS_RELEASE(poll_data);
}

// ------------JHANDLERS----------------

JHANDLER_FUNCTION(createRequest) {
  DJHANDLER_CHECK_THIS(object);
  DJHANDLER_CHECK_ARGS(1, object);

  const iotjs_jval_t* jthis = JHANDLER_GET_ARG(0, object);

  iotjs_jval_t jhost = iotjs_jval_get_property(jthis, IOTJS_MAGIC_STRING_HOST);
  iotjs_string_t host = iotjs_jval_as_string(&jhost);
  iotjs_jval_destroy(&jhost);

  iotjs_jval_t jmethod =
      iotjs_jval_get_property(jthis, IOTJS_MAGIC_STRING_METHOD);
  iotjs_string_t method = iotjs_jval_as_string(&jmethod);
  iotjs_jval_destroy(&jmethod);

  iotjs_jval_t jca = iotjs_jval_get_property(jthis, IOTJS_MAGIC_STRING_CA);
  iotjs_string_t ca = iotjs_jval_as_string(&jca);
  iotjs_jval_destroy(&jca);

  iotjs_jval_t jcert = iotjs_jval_get_property(jthis, IOTJS_MAGIC_STRING_CERT);
  iotjs_string_t cert = iotjs_jval_as_string(&jcert);
  iotjs_jval_destroy(&jcert);

  iotjs_jval_t jkey = iotjs_jval_get_property(jthis, IOTJS_MAGIC_STRING_KEY);
  iotjs_string_t key = iotjs_jval_as_string(&jkey);
  iotjs_jval_destroy(&jkey);

  iotjs_jval_t jreject_unauthorized =
      iotjs_jval_get_property(jthis, IOTJS_MAGIC_STRING_REJECTUNAUTHORIZED);
  const bool reject_unauthorized = iotjs_jval_as_boolean(&jreject_unauthorized);

  if (curl_global_init(CURL_GLOBAL_SSL)) {
    return;
  }
  iotjs_https_t* https_data =
      iotjs_https_create(iotjs_string_data(&host), iotjs_string_data(&method),
                         iotjs_string_data(&ca), iotjs_string_data(&cert),
                         iotjs_string_data(&key), reject_unauthorized, jthis);

  iotjs_https_initialize_curl_opts(https_data);

  iotjs_string_destroy(&host);
  iotjs_string_destroy(&method);
  iotjs_string_destroy(&ca);
  iotjs_string_destroy(&cert);
  iotjs_string_destroy(&key);
  iotjs_jhandler_return_null(jhandler);
}

JHANDLER_FUNCTION(addHeader) {
  DJHANDLER_CHECK_THIS(object);

  DJHANDLER_CHECK_ARGS(2, string, object);
  iotjs_string_t header = JHANDLER_GET_ARG(0, string);
  const char* char_header = iotjs_string_data(&header);

  const iotjs_jval_t* jthis = JHANDLER_GET_ARG(1, object);
  iotjs_https_t* https_data =
      (iotjs_https_t*)iotjs_jval_get_object_native_handle(jthis);
  iotjs_https_add_header(https_data, char_header);

  iotjs_string_destroy(&header);
  iotjs_jhandler_return_null(jhandler);
}

JHANDLER_FUNCTION(sendRequest) {
  DJHANDLER_CHECK_THIS(object);

  DJHANDLER_CHECK_ARG(0, object);
  const iotjs_jval_t* jthis = JHANDLER_GET_ARG(0, object);
  iotjs_https_t* https_data =
      (iotjs_https_t*)iotjs_jval_get_object_native_handle(jthis);
  iotjs_https_send_request(https_data);
  iotjs_jhandler_return_null(jhandler);
}

JHANDLER_FUNCTION(setTimeout) {
  DJHANDLER_CHECK_THIS(object);
  DJHANDLER_CHECK_ARGS(2, number, object);

  double ms = JHANDLER_GET_ARG(0, number);
  const iotjs_jval_t* jthis = JHANDLER_GET_ARG(1, object);

  iotjs_https_t* https_data =
      (iotjs_https_t*)iotjs_jval_get_object_native_handle(jthis);
  iotjs_https_set_timeout((long)ms, https_data);

  iotjs_jhandler_return_null(jhandler);
}

JHANDLER_FUNCTION(_write) {
  DJHANDLER_CHECK_THIS(object);
  DJHANDLER_CHECK_ARGS(2, object, string);
  // Argument 3 can be null, so not checked directly, checked later
  DJHANDLER_CHECK_ARG(3, function);

  const iotjs_jval_t* jthis = JHANDLER_GET_ARG(0, object);
  iotjs_string_t read_chunk = JHANDLER_GET_ARG(1, string);

  const iotjs_jval_t* callback = iotjs_jhandler_get_arg(jhandler, 2);
  const iotjs_jval_t* onwrite = JHANDLER_GET_ARG(3, function);

  iotjs_https_t* https_data =
      (iotjs_https_t*)iotjs_jval_get_object_native_handle(jthis);
  iotjs_https_data_to_write(https_data, read_chunk, callback, onwrite);

  // readchunk was copied to https_data, hence not destroyed.
  iotjs_jhandler_return_null(jhandler);
}

JHANDLER_FUNCTION(finishRequest) {
  DJHANDLER_CHECK_THIS(object);
  DJHANDLER_CHECK_ARG(0, object);

  const iotjs_jval_t* jthis = JHANDLER_GET_ARG(0, object);
  iotjs_https_t* https_data =
      (iotjs_https_t*)iotjs_jval_get_object_native_handle(jthis);
  iotjs_https_finish_request(https_data);

  iotjs_jhandler_return_null(jhandler);
}

JHANDLER_FUNCTION(Abort) {
  DJHANDLER_CHECK_THIS(object);
  DJHANDLER_CHECK_ARG(0, object);

  const iotjs_jval_t* jthis = JHANDLER_GET_ARG(0, object);
  iotjs_https_t* https_data =
      (iotjs_https_t*)iotjs_jval_get_object_native_handle(jthis);
  iotjs_https_cleanup(https_data);

  iotjs_jhandler_return_null(jhandler);
}

iotjs_jval_t InitHttps() {
  iotjs_jval_t https = iotjs_jval_create_object();

  iotjs_jval_set_method(&https, IOTJS_MAGIC_STRING_CREATEREQUEST,
                        createRequest);
  iotjs_jval_set_method(&https, IOTJS_MAGIC_STRING_ADDHEADER, addHeader);
  iotjs_jval_set_method(&https, IOTJS_MAGIC_STRING_SENDREQUEST, sendRequest);
  iotjs_jval_set_method(&https, IOTJS_MAGIC_STRING_SETTIMEOUT, setTimeout);
  iotjs_jval_set_method(&https, IOTJS_MAGIC_STRING__WRITE, _write);
  iotjs_jval_set_method(&https, IOTJS_MAGIC_STRING_FINISHREQUEST,
                        finishRequest);
  iotjs_jval_set_method(&https, IOTJS_MAGIC_STRING_ABORT, Abort);

  return https;
}
