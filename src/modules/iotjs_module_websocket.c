/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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

#include <stdlib.h>
#include <time.h>

#include "iotjs_def.h"
#include "iotjs_module_buffer.h"
#include "iotjs_module_crypto.h"
#include "iotjs_module_websocket.h"


IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(wsclient);

static void iotjs_wsclient_destroy(iotjs_wsclient_t *wsclient) {
  IOTJS_RELEASE(wsclient->tcp_buff.buffer);
  IOTJS_RELEASE(wsclient->ws_buff.data);
  IOTJS_RELEASE(wsclient->generated_key);
  IOTJS_RELEASE(wsclient);
}

iotjs_wsclient_t *iotjs_wsclient_create(const jerry_value_t jobject) {
  iotjs_wsclient_t *wsclient = IOTJS_ALLOC(iotjs_wsclient_t);

  jerry_set_object_native_pointer(jobject, wsclient, &this_module_native_info);
  return wsclient;
}

static const char WS_GUID[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

/**
 * The protocol is as follows:
 * method + USER_ENDPOINT + protocol
 * host + USER_HOST + line_end
 * upgrade
 * connection
 * sec_websocket_key + line_end
 * sec_websocket_ver
 */
static const char method[] = "GET ";
static const char protocol[] = " HTTP/1.1\r\n";
static const char host[] = "Host: ";
static const char line_end[] = "\r\n";
static const char upgrade[] = "Upgrade: websocket\r\n";
static const char connection[] = "Connection: Upgrade\r\n";
static const char sec_websocket_key[] = "Sec-WebSocket-Key: ";
static const char sec_websocket_ver[] = "Sec-WebSocket-Version: 13\r\n\r\n";
static const char handshake_response[] =
    "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: "
    "Upgrade\r\nSec-WebSocket-Accept: ";
static size_t header_fixed_size =
    sizeof(method) + sizeof(protocol) + sizeof(host) + sizeof(upgrade) +
    sizeof(connection) + sizeof(sec_websocket_key) + sizeof(sec_websocket_ver) -
    9; // 9 is for every \0


void iotjs_websocket_create_callback(jerry_value_t jsref, jerry_value_t jmsg,
                                     char *name, jerry_value_t client) {
  jerry_value_t args[2];
  args[0] = jmsg;
  args[1] = client;
  jerry_value_t fn = iotjs_jval_get_property(jsref, name);
  iotjs_invoke_callback(fn, jsref, args, 2);

  jerry_release_value(fn);
}


static unsigned char *ws_generate_key(jerry_value_t jsref, size_t *key_len) {
  unsigned char *key = IOTJS_CALLOC(16, unsigned char);
  for (int i = 0; i < 16; i++) {
    key[i] = rand() % 256;
  }

  unsigned char *ret_val = NULL;

  if (!(*key_len = iotjs_base64_encode(&ret_val, key, 16))) {
    jerry_value_t ret_str =
        jerry_create_string((jerry_char_t *)"mbedtls base64 encode failed");
    iotjs_websocket_create_callback(jsref, ret_str, IOTJS_MAGIC_STRING_ONERROR,
                                    ret_str);
    jerry_release_value(ret_str);
  }
  IOTJS_RELEASE(key);

  return ret_val;
}


static char *iotjs_ws_write_header(char *dst, const char *src) {
  memcpy(dst, src, strlen(src));
  return dst + strlen(src);
}


static char *iotjs_ws_write_data(char *buff, void *data, size_t size) {
  memcpy(buff, data, size);
  return buff + size;
}


static unsigned char *iotjs_make_handshake_key(char *client_key,
                                               size_t *key_len) {
  unsigned char *out_buff = NULL;

  size_t ws_guid_size = strlen(WS_GUID);
  size_t client_key_size = strnlen((char *)client_key, 24);
  size_t concatenated_size = ws_guid_size + client_key_size;

  unsigned char concatenated[concatenated_size + 1];
  memcpy(concatenated, client_key, client_key_size);
  memcpy(concatenated + client_key_size, WS_GUID, ws_guid_size);
  concatenated[concatenated_size] = '\0';
  size_t out_buff_size =
      iotjs_sha1_encode(&out_buff, concatenated, concatenated_size);
  unsigned char *key_out = NULL;
  if (!(*key_len = iotjs_base64_encode(&key_out, out_buff, out_buff_size))) {
    key_out = NULL;
  }

  IOTJS_RELEASE(out_buff);
  return key_out;
}

static bool iotjs_check_handshake_key(char *server_key, jerry_value_t jsref) {
  bool ret_val = true;
  void *native_p;
  if (!jerry_get_object_native_pointer(jsref, &native_p,
                                       &this_module_native_info)) {
    ret_val = false;
  }

  iotjs_wsclient_t *wsclient = (iotjs_wsclient_t *)native_p;
  size_t key_len = 0;
  unsigned char *key;
  if (!(key = iotjs_make_handshake_key((char *)wsclient->generated_key,
                                       &key_len))) {
    ret_val = false;
  }

  if (key && strncmp(server_key, (const char *)key, key_len)) {
    ret_val = false;
  }

  IOTJS_RELEASE(wsclient->generated_key);
  IOTJS_RELEASE(key);

  return ret_val;
}


static jerry_value_t iotjs_websocket_encode_frame(uint8_t opcode, bool mask,
                                                  bool compress, char *payload,
                                                  size_t payload_len) {
  uint8_t header[2] = { 0 };

  uint64_t buffer_size = payload_len + sizeof(header);

  header[0] |= WS_FIN_BIT;
  header[0] |= opcode;

  if (compress) {
    header[0] |= 0x40;
  }

  if (mask) {
    header[1] |= WS_MASK_BIT;
    buffer_size += 4; // mask key size is 32 bits
  }

  uint8_t extended_len_size = 0;
  if (payload_len <= WS_ONE_BYTE_LENGTH) {
    header[1] |= payload_len;
  } else if (payload_len <= UINT16_MAX) {
    header[1] |= WS_TWO_BYTES_LENGTH;
    extended_len_size = 2;
  } else {
    header[1] |= WS_THREE_BYTES_LENGTH;
    extended_len_size = 8;
  }

  buffer_size += extended_len_size;

  jerry_value_t jframe = iotjs_bufferwrap_create_buffer(buffer_size);
  iotjs_bufferwrap_t *frame_wrap = iotjs_bufferwrap_from_jbuffer(jframe);
  char *buff_ptr = frame_wrap->buffer;

  *buff_ptr++ = (char)header[0];
  *buff_ptr++ = (char)header[1];

  if (extended_len_size) {
    if (extended_len_size == 2) {
      uint16_t len = payload_len;
      *buff_ptr++ = *((char *)&len + 1);
      *buff_ptr++ = *((char *)&len);
    } else {
      uint64_t len = payload_len;
      for (int8_t i = sizeof(uint64_t) - 1; i >= 0; i--) {
        *buff_ptr++ = *((char *)&len + i);
      }
    }
  }

  if (payload != NULL) {
    if (mask) {
      uint8_t key[4];
      for (uint8_t i = 0; i < sizeof(key); i++) {
        key[i] = rand() % 256;
      }

      buff_ptr = iotjs_ws_write_data(buff_ptr, key, sizeof(key));

      for (size_t i = 0; i < payload_len; i++) {
        payload[i] ^= key[i % 4];
      }
    }

    buff_ptr = iotjs_ws_write_data(buff_ptr, payload, payload_len);
  }

  return jframe;
}


static void iotjs_websocket_create_buffer_and_cb(char **buff_ptr,
                                                 uint32_t payload_len,
                                                 char *cb_type,
                                                 jerry_value_t jsref,
                                                 jerry_value_t client) {
  if (payload_len > 0) {
    jerry_value_t ret_buff = iotjs_bufferwrap_create_buffer(payload_len);
    iotjs_bufferwrap_t *buff_wrap = iotjs_bufferwrap_from_jbuffer(ret_buff);
    iotjs_ws_write_data(buff_wrap->buffer, *buff_ptr, payload_len);
    *buff_ptr += payload_len;
    iotjs_websocket_create_callback(jsref, ret_buff, cb_type, client);
    jerry_release_value(ret_buff);

    return;
  }
  iotjs_websocket_create_callback(jsref, jerry_create_undefined(), cb_type,
                                  client);
}


static jerry_value_t iotjs_websocket_check_error(uint8_t code) {
  switch (code) {
    case WS_ERR_INVALID_UTF8: {
      return JS_CREATE_ERROR(COMMON, "Invalid UTF8 string in UTF8 message");
    }

    case WS_ERR_INVALID_TERMINATE_CODE: {
      return JS_CREATE_ERROR(COMMON, "Invalid terminate code received");
    }

    case WS_ERR_UNKNOWN_OPCODE: {
      return JS_CREATE_ERROR(COMMON, "Uknown opcode received");
    }

    case WS_ERR_NATIVE_POINTER_ERR: {
      return JS_CREATE_ERROR(COMMON, "WebSocket native pointer unavailable");
    }

    case WS_ERR_FRAME_SIZE_LIMIT: {
      return JS_CREATE_ERROR(COMMON, "Frame size received exceeds limit");
    }

    default: { return jerry_create_undefined(); };
  }
}


JS_FUNCTION(prepare_handshake_request) {
  DJS_CHECK_THIS();

  jerry_value_t jsref = JS_GET_ARG(0, object);
  jerry_value_t jhost = JS_GET_ARG(1, any);
  jerry_value_t jendpoint = JS_GET_ARG(2, any);

  iotjs_string_t l_host;
  iotjs_string_t l_endpoint;
  if (!(iotjs_jbuffer_as_string(jendpoint, &l_endpoint)) ||
      !(iotjs_jbuffer_as_string(jhost, &l_host))) {
    return JS_CREATE_ERROR(COMMON, "Invalid host and/or path arguments!");
  };

  iotjs_wsclient_t *wsclient = NULL;
  if (!jerry_get_object_native_pointer(jsref, (void **)&wsclient,
                                       &this_module_native_info)) {
    return iotjs_websocket_check_error(WS_ERR_NATIVE_POINTER_ERR);
  }

  size_t generated_key_len = 0;
  wsclient->generated_key = ws_generate_key(jsref, &generated_key_len);

  jerry_value_t jfinal = iotjs_bufferwrap_create_buffer(
      header_fixed_size + iotjs_string_size(&l_endpoint) +
      iotjs_string_size(&l_host) + (sizeof(line_end) * 2) + generated_key_len);

  iotjs_bufferwrap_t *final_wrap = iotjs_bufferwrap_from_jbuffer(jfinal);

  char *buff_ptr = final_wrap->buffer;
  buff_ptr = iotjs_ws_write_header(buff_ptr, method);
  memcpy(buff_ptr, iotjs_string_data(&l_endpoint),
         iotjs_string_size(&l_endpoint));
  buff_ptr += iotjs_string_size(&l_endpoint);
  buff_ptr = iotjs_ws_write_header(buff_ptr, protocol);
  buff_ptr = iotjs_ws_write_header(buff_ptr, host);
  memcpy(buff_ptr, iotjs_string_data(&l_host), iotjs_string_size(&l_host));
  buff_ptr += iotjs_string_size(&l_host);
  buff_ptr = iotjs_ws_write_header(buff_ptr, line_end);
  buff_ptr = iotjs_ws_write_header(buff_ptr, upgrade);
  buff_ptr = iotjs_ws_write_header(buff_ptr, connection);
  buff_ptr = iotjs_ws_write_header(buff_ptr, sec_websocket_key);
  memcpy(buff_ptr, wsclient->generated_key, generated_key_len);
  buff_ptr += generated_key_len;
  buff_ptr = iotjs_ws_write_header(buff_ptr, line_end);
  buff_ptr = iotjs_ws_write_header(buff_ptr, sec_websocket_ver);

  iotjs_string_destroy(&l_endpoint);
  iotjs_string_destroy(&l_host);

  return jfinal;
}


/**
 * HTTP/1.1 101 Switching Protocols
 * Upgrade: websocket
 * Connection: Upgrade
 * Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=
 */
JS_FUNCTION(receive_handshake_data) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, string);

  iotjs_string_t client_key = JS_GET_ARG(0, string);

  size_t key_len = 0;
  unsigned char *key;
  if (!(key = iotjs_make_handshake_key((char *)iotjs_string_data(&client_key),
                                       &key_len))) {
    return JS_CREATE_ERROR(COMMON, "mbedtls base64 encode failed");
  }

  jerry_value_t jfinal = iotjs_bufferwrap_create_buffer(
      sizeof(handshake_response) - 1 + key_len + sizeof(line_end) * 2);

  iotjs_bufferwrap_t *final_wrap = iotjs_bufferwrap_from_jbuffer(jfinal);
  char *buff_ptr = final_wrap->buffer;
  buff_ptr = iotjs_ws_write_header(buff_ptr, handshake_response);
  memcpy(buff_ptr, key, key_len);
  buff_ptr += key_len;
  buff_ptr = iotjs_ws_write_header(buff_ptr, line_end);
  buff_ptr = iotjs_ws_write_header(buff_ptr, line_end);

  iotjs_string_destroy(&client_key);
  IOTJS_RELEASE(key);
  return jfinal;
}

JS_FUNCTION(parse_handshake_data) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(2, object, object);

  jerry_value_t jbuffer = JS_GET_ARG(0, object);
  iotjs_bufferwrap_t *buff_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);
  if (buff_wrap->length < 12 || strncmp(buff_wrap->buffer + 9, "101", 3)) {
    return JS_CREATE_ERROR(COMMON, "WebSocket connection failed");
  }
  jerry_value_t jsref = JS_GET_ARG(1, object);

  char ws_accept[] = "Sec-WebSocket-Accept: ";
  char *frame_end = strstr(buff_wrap->buffer, "\r\n\r\n");
  char *key_pos = strstr(buff_wrap->buffer, ws_accept) + strlen(ws_accept);
  char key[28] = { 0 };
  memcpy(key, key_pos, 28);

  frame_end += 4; // \r\n\r\n

  if (!iotjs_check_handshake_key(key, jsref)) {
    return JS_CREATE_ERROR(COMMON, "WebSocket handshake key comparison failed");
  }

  size_t header_size = (size_t)(frame_end - buff_wrap->buffer);
  if (buff_wrap->length > header_size) {
    size_t remaining_length = buff_wrap->length - header_size;
    jerry_value_t jdata = iotjs_bufferwrap_create_buffer(remaining_length);
    iotjs_bufferwrap_t *data_wrap = iotjs_bufferwrap_from_jbuffer(jdata);

    memcpy(data_wrap->buffer, buff_wrap->buffer + header_size,
           remaining_length);
    data_wrap->length = remaining_length;

    return jdata;
  }

  return jerry_create_undefined();
}


static void iotjs_websocket_concat_tcp_buffers(iotjs_wsclient_t *wsclient,
                                               iotjs_bufferwrap_t *buff_recv) {
  char *tmp_buf = wsclient->tcp_buff.buffer;
  wsclient->tcp_buff.buffer =
      IOTJS_CALLOC(wsclient->tcp_buff.length + buff_recv->length, char);
  memcpy(wsclient->tcp_buff.buffer, tmp_buf, wsclient->tcp_buff.length);
  memcpy(wsclient->tcp_buff.buffer + wsclient->tcp_buff.length,
         buff_recv->buffer, buff_recv->length);
  wsclient->tcp_buff.length += buff_recv->length;
  IOTJS_RELEASE(tmp_buf);
}


static uint8_t iotjs_websocket_decode_frame(iotjs_wsclient_t *wsclient,
                                            char *first_byte, char *buff_ptr,
                                            uint32_t payload_len,
                                            jerry_value_t jsref, bool mask,
                                            jerry_value_t client) {
  uint8_t fin_bit = (first_byte[0] >> 7) & 0x01;
  uint8_t opcode = first_byte[0] & 0x0F;

  uint32_t *mask_key = NULL;
  if (mask) {
    mask_key = (uint32_t *)buff_ptr;
    buff_ptr += sizeof(uint32_t);

    for (uint64_t i = 0; i < payload_len; i++) {
      buff_ptr[i] ^= ((unsigned char *)(mask_key))[i % 4];
    }
  }

  switch (opcode) {
    case WS_OP_CONTINUE: {
      if (wsclient->ws_buff.length == 0) {
        wsclient->ws_buff.masked = mask;
        wsclient->ws_buff.first_byte = first_byte[0];
        wsclient->ws_buff.data = IOTJS_CALLOC(payload_len, char);
        memcpy(wsclient->ws_buff.data, buff_ptr, payload_len);
        wsclient->ws_buff.length = payload_len;
        break;
      }

      char *tmp_ptr = wsclient->ws_buff.data;
      uint32_t tmp_len = wsclient->ws_buff.length;
      wsclient->ws_buff.data =
          IOTJS_CALLOC(wsclient->ws_buff.length + payload_len, char);
      memcpy(wsclient->ws_buff.data, tmp_ptr, tmp_len);
      memcpy(wsclient->ws_buff.data + tmp_len, buff_ptr, payload_len);
      wsclient->ws_buff.length += payload_len;
      IOTJS_RELEASE(tmp_ptr);


      if (fin_bit) {
        uint8_t ret_val =
            iotjs_websocket_decode_frame(wsclient,
                                         &wsclient->ws_buff.first_byte,
                                         wsclient->ws_buff.data,
                                         wsclient->ws_buff.length, jsref,
                                         wsclient->ws_buff.masked, client);

        IOTJS_RELEASE(wsclient->ws_buff.data);
        wsclient->ws_buff.length = 0;
        wsclient->ws_buff.first_byte = 0;

        return ret_val;
      }
      break;
    }

    case WS_OP_UTF8:
    case WS_OP_BINARY: {
      if (opcode == WS_OP_UTF8 &&
          !jerry_is_valid_utf8_string((unsigned char *)buff_ptr, payload_len)) {
        return WS_ERR_INVALID_UTF8;
      }
      iotjs_websocket_create_buffer_and_cb(&buff_ptr, payload_len,
                                           IOTJS_MAGIC_STRING_ONMESSAGE, jsref,
                                           client);
      break;
    }

    case WS_OP_TERMINATE: {
      if (payload_len > 0) {
        uint16_t ret_code = (uint16_t)((unsigned char)buff_ptr[0] << 8 |
                                       (unsigned char)buff_ptr[1]);
        if (ret_code > 4999 || ret_code < 1000) {
          return WS_ERR_INVALID_TERMINATE_CODE;
        }

        buff_ptr += 2;
        payload_len -= 2;
        size_t ret_code_str_size = 4;
        char ret_code_str[ret_code_str_size + 1];
        snprintf(ret_code_str, ret_code_str_size + 1, "%d", ret_code);

        jerry_value_t ret_buff =
            iotjs_bufferwrap_create_buffer(payload_len + ret_code_str_size);
        iotjs_bufferwrap_t *ret_wrap = iotjs_bufferwrap_from_jbuffer(ret_buff);
        char *local_ptr = ret_wrap->buffer;
        local_ptr =
            iotjs_ws_write_data(local_ptr, ret_code_str, ret_code_str_size);
        local_ptr = iotjs_ws_write_data(local_ptr, buff_ptr, payload_len);
        buff_ptr += payload_len;
        iotjs_websocket_create_callback(jsref, ret_buff,
                                        IOTJS_MAGIC_STRING_ONCLOSE, client);
        jerry_release_value(ret_buff);
        break;
      }
      iotjs_websocket_create_callback(jsref, jerry_create_undefined(),
                                      IOTJS_MAGIC_STRING_ONCLOSE, client);
      break;
    }

    case WS_OP_PING: {
      iotjs_websocket_create_buffer_and_cb(&buff_ptr, payload_len,
                                           IOTJS_MAGIC_STRING_PONG, jsref,
                                           client);
      break;
    }

    case WS_OP_PONG: {
      iotjs_websocket_create_buffer_and_cb(&buff_ptr, payload_len,
                                           IOTJS_MAGIC_STRING_ONPINGRESP, jsref,
                                           client);
      break;
    }

    default:
      return WS_ERR_UNKNOWN_OPCODE;
      break;
  }

  return 0;
}


JS_FUNCTION(ws_receive) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(3, object, object, object);

  jerry_value_t jsref = JS_GET_ARG(0, object);

  iotjs_wsclient_t *wsclient = NULL;
  if (!jerry_get_object_native_pointer(jsref, (void **)&wsclient,
                                       &this_module_native_info)) {
    return iotjs_websocket_check_error(WS_ERR_NATIVE_POINTER_ERR);
  }

  jerry_value_t jbuffer = JS_GET_ARG(1, object);
  iotjs_bufferwrap_t *buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);

  jerry_value_t client = JS_GET_ARG(2, object);

  if (buffer_wrap->length == 0) {
    return jerry_create_undefined();
  }

  char *current_buffer = buffer_wrap->buffer;
  char *current_buffer_end = current_buffer + buffer_wrap->length;

  if (wsclient->tcp_buff.length > 0) {
    iotjs_websocket_concat_tcp_buffers(wsclient, buffer_wrap);
    current_buffer = wsclient->tcp_buff.buffer;
    current_buffer_end = current_buffer + wsclient->tcp_buff.length;
  }


  while (true) {
    if (current_buffer >= current_buffer_end) {
      if (wsclient->tcp_buff.length > 0) {
        IOTJS_RELEASE(wsclient->tcp_buff.buffer);
        wsclient->tcp_buff.length = 0;
      }
      return jerry_create_undefined();
    }

    if (current_buffer + 2 > current_buffer_end) {
      break;
    }

    char *first_byte = current_buffer;
    uint8_t payload_byte = (current_buffer[1]) & WS_THREE_BYTES_LENGTH;
    uint8_t mask = (first_byte[1] >> 7) & 0x01;

    current_buffer += 2;

    uint32_t payload_len;
    if (!(payload_byte ^ WS_TWO_BYTES_LENGTH)) {
      if (current_buffer + sizeof(uint16_t) > current_buffer_end) {
        break;
      }
      unsigned char *len_buffer = (unsigned char *)current_buffer;
      payload_len = (uint16_t)(len_buffer[0] << 8 | len_buffer[1]);
      current_buffer += sizeof(uint16_t);
    } else if (!(payload_byte ^ WS_THREE_BYTES_LENGTH)) {
      uint64_t payload_64bit_len;
      if (current_buffer + sizeof(uint64_t) > current_buffer_end) {
        break;
      }
      for (uint8_t i = 0; i < sizeof(uint64_t); i++) {
        memcpy((uint8_t *)&payload_64bit_len + i,
               current_buffer + sizeof(uint64_t) - 1 - i, sizeof(uint8_t));
      }

      if (payload_64bit_len > UINT32_MAX) {
        return WS_ERR_FRAME_SIZE_LIMIT;
      }
      payload_len = (uint32_t)payload_64bit_len;

      current_buffer += sizeof(uint64_t);
    } else {
      payload_len = payload_byte;
    }

    if (mask && ((current_buffer + 4 > current_buffer_end) ||
                 (current_buffer + 4 + payload_len > current_buffer_end))) {
      break;
    } else if (!mask && (current_buffer + payload_len > current_buffer_end)) {
      break;
    }

    uint8_t ret_val =
        iotjs_websocket_decode_frame(wsclient, first_byte, current_buffer,
                                     payload_len, jsref, mask, client);
    if (ret_val) {
      return iotjs_websocket_check_error(ret_val);
    }

    current_buffer += mask ? 4 : 0;
    current_buffer += payload_len;
  }

  if (current_buffer == wsclient->tcp_buff.buffer) {
    return jerry_create_undefined();
  }

  uint32_t remaining_size = (uint32_t)(current_buffer_end - current_buffer);
  char *buffer = IOTJS_CALLOC(remaining_size, char);
  memcpy(buffer, current_buffer, remaining_size);

  if (wsclient->tcp_buff.buffer != NULL) {
    IOTJS_RELEASE(wsclient->tcp_buff.buffer);
  }

  wsclient->tcp_buff.buffer = buffer;
  wsclient->tcp_buff.length = remaining_size;

  return jerry_create_undefined();
}


JS_FUNCTION(ws_init) {
  DJS_CHECK_THIS();

  const jerry_value_t jws = JS_GET_ARG(0, object);

  iotjs_wsclient_t *wsclient = iotjs_wsclient_create(jws);
  wsclient->tcp_buff.buffer = NULL;
  wsclient->tcp_buff.length = 0;

  wsclient->ws_buff.data = NULL;
  wsclient->ws_buff.length = 0;

  wsclient->generated_key = NULL;

  return jerry_create_undefined();
}


JS_FUNCTION(ws_close) {
  DJS_CHECK_THIS();

  bool masked = false;
  uint8_t opcode = WS_OP_TERMINATE;
  iotjs_string_t payload = iotjs_string_create();


  jerry_value_t jmsg = JS_GET_ARG(0, any);
  jerry_value_t jcode = JS_GET_ARG(1, any);

  iotjs_string_t msg = iotjs_string_create();
  // Client side close frame must be ALWAYS masked
  masked = iotjs_jbuffer_as_string(jmsg, &msg) || jerry_value_is_number(jcode);

  if (jerry_value_is_number(jcode)) {
    uint16_t code = jerry_get_number_value(jcode);
    iotjs_string_append(&payload, (const char *)&code + 1, 1);
    iotjs_string_append(&payload, (const char *)&code, 1);
  }

  if (!iotjs_string_is_empty(&msg)) {
    // We have no status code, but do have msg, append the status code
    if (iotjs_string_is_empty(&payload)) {
      uint16_t code = 1000;
      iotjs_string_append(&payload, (const char *)&code + 1, 1);
      iotjs_string_append(&payload, (const char *)&code, 1);
    }
    iotjs_string_append(&payload, iotjs_string_data(&msg),
                        iotjs_string_size(&msg));
    iotjs_string_destroy(&msg);
  }

  jerry_value_t ret_val =
      iotjs_websocket_encode_frame(opcode, masked, false,
                                   (char *)iotjs_string_data(&payload),
                                   iotjs_string_size(&payload));

  if (payload.data != NULL) {
    iotjs_string_destroy(&payload);
  }

  return ret_val;
}


JS_FUNCTION(ws_send_data) {
  DJS_CHECK_THIS();

  jerry_value_t jmsg = JS_GET_ARG(0, any);
  iotjs_string_t msg = iotjs_string_create();

  if (!iotjs_jbuffer_as_string(jmsg, &msg)) {
    return jerry_create_undefined();
  }

  bool binary = jerry_get_boolean_value(jargv[1]);
  bool mask = jerry_get_boolean_value(jargv[2]);
  bool compress = jerry_get_boolean_value(jargv[3]);

  uint8_t opcode = binary ? WS_OP_BINARY : WS_OP_UTF8;

  jerry_value_t ret_val =
      iotjs_websocket_encode_frame(opcode, mask, compress,
                                   (char *)iotjs_string_data(&msg),
                                   iotjs_string_size(&msg));

  iotjs_string_destroy(&msg);
  return ret_val;
}


JS_FUNCTION(ws_ping_or_pong) {
  DJS_CHECK_THIS();

  uint8_t opcode =
      jerry_get_boolean_value(JS_GET_ARG(0, any)) ? WS_OP_PING : WS_OP_PONG;
  jerry_value_t jmsg = JS_GET_ARG(1, any);

  iotjs_string_t msg = iotjs_string_create();

  jerry_value_t ret_val;

  if (iotjs_jbuffer_as_string(jmsg, &msg)) {
    ret_val =
        iotjs_websocket_encode_frame(opcode, jerry_get_boolean_value(
                                                 JS_GET_ARG(2, any)),
                                     false, (char *)iotjs_string_data(&msg),
                                     iotjs_string_size(&msg));
    iotjs_string_destroy(&msg);
  } else {
    ret_val = iotjs_websocket_encode_frame(opcode, false, false, NULL, 0);
  }

  return ret_val;
}


jerry_value_t iotjs_init_websocket(void) {
  IOTJS_UNUSED(WS_GUID);

  jerry_value_t jws = jerry_create_object();
  iotjs_jval_set_method(jws, IOTJS_MAGIC_STRING_CLOSE, ws_close);
  iotjs_jval_set_method(jws, IOTJS_MAGIC_STRING_PARSEHANDSHAKEDATA,
                        parse_handshake_data);
  iotjs_jval_set_method(jws, IOTJS_MAGIC_STRING_PING, ws_ping_or_pong);
  iotjs_jval_set_method(jws, IOTJS_MAGIC_STRING_PREPAREHANDSHAKE,
                        prepare_handshake_request);
  iotjs_jval_set_method(jws, IOTJS_MAGIC_STRING_SEND, ws_send_data);
  iotjs_jval_set_method(jws, IOTJS_MAGIC_STRING_WSINIT, ws_init);
  iotjs_jval_set_method(jws, IOTJS_MAGIC_STRING_WSRECEIVE, ws_receive);
  iotjs_jval_set_method(jws, IOTJS_MAGIC_STRING_WSRECEIVEHANDSHAKEDATA,
                        receive_handshake_data);

  return jws;
}
