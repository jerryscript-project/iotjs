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
#ifndef IOTJS_MODULE_WEBSOCKET_H
#define IOTJS_MODULE_WEBSOCKET_H

enum {
  WS_OP_CONTINUE = 0x00,
  WS_OP_UTF8 = 0x01,
  WS_OP_BINARY = 0x02,
  WS_OP_TERMINATE = 0x08,
  WS_OP_PING = 0x09,
  WS_OP_PONG = 0x0a,
} iotjs_websocket_opcodes;

enum {
  WS_FIN_BIT = 0x80,
  WS_MASK_BIT = WS_FIN_BIT,
} iotjs_websocket_header_bits;

enum {
  WS_ERR_INVALID_UTF8 = 1,
  WS_ERR_INVALID_TERMINATE_CODE = 2,
  WS_ERR_UNKNOWN_OPCODE = 3,
  WS_ERR_NATIVE_POINTER_ERR = 4,
  WS_ERR_FRAME_SIZE_LIMIT = 5,
} iotjs_websocket_err_codes;

enum {
  WS_ONE_BYTE_LENGTH = 125,
  WS_TWO_BYTES_LENGTH,
  WS_THREE_BYTES_LENGTH,
} iotjs_websocket_frame_len_types;


typedef struct {
  struct {
    uint32_t length;
    char *buffer;
  } tcp_buff;

  struct {
    uint32_t length;
    char *data;
    char first_byte;
    bool masked;
  } ws_buff;
  unsigned char *generated_key;
} iotjs_wsclient_t;

#endif /* IOTJS_MODULE_WEBSOCKET_H */
