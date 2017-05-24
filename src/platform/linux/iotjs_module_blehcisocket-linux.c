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

/* Copyright (C) 2015 Sandeep Mistry sandeep.mistry@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef IOTJS_MODULE_BLE_HCI_SOCKET_LINUX_GENERAL_INL_H
#define IOTJS_MODULE_BLE_HCI_SOCKET_LINUX_GENERAL_INL_H

#include <errno.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "iotjs_def.h"
#include "modules/iotjs_module_blehcisocket.h"
#include "modules/iotjs_module_buffer.h"

#define BTPROTO_L2CAP 0
#define BTPROTO_HCI 1

#define SOL_HCI 0
#define HCI_FILTER 2

#define HCIGETDEVLIST _IOR('H', 210, int)
#define HCIGETDEVINFO _IOR('H', 211, int)

#define HCI_CHANNEL_RAW 0
#define HCI_CHANNEL_USER 1
#define HCI_CHANNEL_CONTROL 3

#define HCI_DEV_NONE 0xffff

#define HCI_MAX_DEV 16

#define ATT_CID 4

enum {
  HCI_UP,
  HCI_INIT,
  HCI_RUNNING,

  HCI_PSCAN,
  HCI_ISCAN,
  HCI_AUTH,
  HCI_ENCRYPT,
  HCI_INQUIRY,

  HCI_RAW,
};

struct sockaddr_hci {
  sa_family_t hci_family;
  unsigned short hci_dev;
  unsigned short hci_channel;
};

struct hci_dev_req {
  uint16_t dev_id;
  uint32_t dev_opt;
};

struct hci_dev_list_req {
  uint16_t dev_num;
  struct hci_dev_req dev_req[0];
};

typedef struct hci_dev_list_req hci_dev_list_req_type;

typedef struct { uint8_t b[6]; } __attribute__((packed)) bdaddr_t;

struct hci_dev_info {
  uint16_t dev_id;
  char name[8];

  bdaddr_t bdaddr;

  uint32_t flags;
  uint8_t type;

  uint8_t features[8];

  uint32_t pkt_type;
  uint32_t link_policy;
  uint32_t link_mode;

  uint16_t acl_mtu;
  uint16_t acl_pkts;
  uint16_t sco_mtu;
  uint16_t sco_pkts;

  // hci_dev_stats
  uint32_t err_rx;
  uint32_t err_tx;
  uint32_t cmd_tx;
  uint32_t evt_rx;
  uint32_t acl_tx;
  uint32_t acl_rx;
  uint32_t sco_tx;
  uint32_t sco_rx;
  uint32_t byte_rx;
  uint32_t byte_tx;
};

struct sockaddr_l2 {
  sa_family_t l2_family;
  unsigned short l2_psm;
  bdaddr_t l2_bdaddr;
  unsigned short l2_cid;
  uint8_t l2_bdaddr_type;
};


#define THIS iotjs_blehcisocket_t* blehcisocket


void iotjs_blehcisocket_initialize(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);

  _this->_l2socketCount = 0;

  _this->_socket = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);

  uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());
  uv_poll_init(loop, &(_this->_pollHandle), _this->_socket);

  _this->_pollHandle.data = blehcisocket;
}


void iotjs_blehcisocket_close(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);

  uv_close((uv_handle_t*)&(_this->_pollHandle), NULL);

  close(_this->_socket);
}


void iotjs_blehcisocket_start(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);

  uv_poll_start(&_this->_pollHandle, UV_READABLE, iotjs_blehcisocket_poll_cb);
}


int iotjs_blehcisocket_bindRaw(THIS, int* devId) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);

  struct sockaddr_hci a;
  struct hci_dev_info di;

  memset(&a, 0, sizeof(a));
  a.hci_family = AF_BLUETOOTH;
  a.hci_dev = iotjs_blehcisocket_devIdFor(blehcisocket, devId, true);
  a.hci_channel = HCI_CHANNEL_RAW;

  _this->_devId = a.hci_dev;
  _this->_mode = HCI_CHANNEL_RAW;

  if (bind(_this->_socket, (struct sockaddr*)&a, sizeof(a)) < 0) {
    DLOG("ERROR on binding: %s", strerror(errno));
    return _this->_devId;
  }

  // get the local address and address type
  memset(&di, 0x00, sizeof(di));
  di.dev_id = _this->_devId;
  memset(_this->_address, 0, sizeof(_this->_address));
  _this->_addressType = 0;

  if (ioctl(_this->_socket, HCIGETDEVINFO, (void*)&di) > -1) {
    memcpy(_this->_address, &di.bdaddr, sizeof(di.bdaddr));
    _this->_addressType = di.type;

    if (_this->_addressType == 3) {
      // 3 is a weird type, use 1 (public) instead
      _this->_addressType = 1;
    }
  }

  return _this->_devId;
}


int iotjs_blehcisocket_bindUser(THIS, int* devId) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);

  struct sockaddr_hci a;

  memset(&a, 0, sizeof(a));
  a.hci_family = AF_BLUETOOTH;
  a.hci_dev = iotjs_blehcisocket_devIdFor(blehcisocket, devId, false);
  a.hci_channel = HCI_CHANNEL_USER;

  _this->_devId = a.hci_dev;
  _this->_mode = HCI_CHANNEL_USER;

  if (bind(_this->_socket, (struct sockaddr*)&a, sizeof(a)) < 0) {
    DLOG("ERROR on binding: %s", strerror(errno));
  }

  return _this->_devId;
}


void iotjs_blehcisocket_bindControl(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);

  struct sockaddr_hci a;

  memset(&a, 0, sizeof(a));
  a.hci_family = AF_BLUETOOTH;
  a.hci_dev = HCI_DEV_NONE;
  a.hci_channel = HCI_CHANNEL_CONTROL;

  _this->_mode = HCI_CHANNEL_CONTROL;

  if (bind(_this->_socket, (struct sockaddr*)&a, sizeof(a)) < 0) {
    DLOG("ERROR on binding: %s", strerror(errno));
  }
}


bool iotjs_blehcisocket_isDevUp(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);

  struct hci_dev_info di;
  bool isUp = false;

  memset(&di, 0x00, sizeof(di));
  di.dev_id = _this->_devId;

  if (ioctl(_this->_socket, HCIGETDEVINFO, (void*)&di) > -1) {
    isUp = (di.flags & (1 << HCI_UP)) != 0;
  }

  return isUp;
}


void iotjs_blehcisocket_setFilter(THIS, char* data, size_t length) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);

  if (setsockopt(_this->_socket, SOL_HCI, HCI_FILTER, data, (socklen_t)length) <
      0) {
    iotjs_blehcisocket_emitErrnoError(blehcisocket);
  }
}


void iotjs_blehcisocket_poll(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);

  int length = 0;
  char data[1024];

  length = read(_this->_socket, data, sizeof(data));

  if (length > 0) {
    if (_this->_mode == HCI_CHANNEL_RAW) {
      if (iotjs_blehcisocket_kernelDisconnectWorkArounds(blehcisocket, length,
                                                         data) < 0) {
        return;
      }
    }

    iotjs_jval_t* jhcisocket = iotjs_jobjectwrap_jobject(&_this->jobjectwrap);
    iotjs_jval_t jemit = iotjs_jval_get_property(jhcisocket, "emit");
    IOTJS_ASSERT(iotjs_jval_is_function(&jemit));

    iotjs_jargs_t jargs = iotjs_jargs_create(2);
    iotjs_jval_t str = iotjs_jval_create_string_raw("data");
    IOTJS_ASSERT(length >= 0);
    iotjs_jval_t jbuf = iotjs_bufferwrap_create_buffer((size_t)length);
    iotjs_bufferwrap_t* buf_wrap = iotjs_bufferwrap_from_jbuffer(&jbuf);
    iotjs_bufferwrap_copy(buf_wrap, data, (size_t)length);
    iotjs_jargs_append_jval(&jargs, &str);
    iotjs_jargs_append_jval(&jargs, &jbuf);
    iotjs_jhelper_call_ok(&jemit, jhcisocket, &jargs);

    iotjs_jval_destroy(&str);
    iotjs_jval_destroy(&jbuf);
    iotjs_jargs_destroy(&jargs);
    iotjs_jval_destroy(&jemit);
  }
}


void iotjs_blehcisocket_stop(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);

  uv_poll_stop(&_this->_pollHandle);
}


void iotjs_blehcisocket_write(THIS, char* data, size_t length) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);

  if (write(_this->_socket, data, length) < 0) {
    iotjs_blehcisocket_emitErrnoError(blehcisocket);
  }
}


void iotjs_blehcisocket_emitErrnoError(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);

  iotjs_jval_t* jhcisocket = iotjs_jobjectwrap_jobject(&_this->jobjectwrap);
  iotjs_jval_t jemit = iotjs_jval_get_property(jhcisocket, "emit");
  IOTJS_ASSERT(iotjs_jval_is_function(&jemit));

  iotjs_jargs_t jargs = iotjs_jargs_create(2);
  iotjs_jval_t str = iotjs_jval_create_string_raw("error");
  iotjs_jval_t jerror = iotjs_jval_create_error(strerror(errno));
  iotjs_jargs_append_jval(&jargs, &str);
  iotjs_jargs_append_jval(&jargs, &jerror);
  iotjs_jhelper_call_ok(&jemit, jhcisocket, &jargs);

  iotjs_jval_destroy(&str);
  iotjs_jval_destroy(&jerror);
  iotjs_jargs_destroy(&jargs);
  iotjs_jval_destroy(&jemit);
}


int iotjs_blehcisocket_devIdFor(THIS, int* pDevId, bool isUp) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);

  int devId = 0; // default

  if (pDevId == NULL) {
    struct hci_dev_list_req* dl;
    struct hci_dev_req* dr;

    dl = (struct hci_dev_list_req*)calloc(HCI_MAX_DEV * sizeof(*dr) +
                                              sizeof(*dl),
                                          1);
    dr = dl->dev_req;

    dl->dev_num = HCI_MAX_DEV;

    if (ioctl(_this->_socket, HCIGETDEVLIST, dl) > -1) {
      for (int i = 0; i < dl->dev_num; i++, dr++) {
        bool devUp = dr->dev_opt & (1 << HCI_UP);
        bool match = isUp ? devUp : !devUp;

        if (match) {
          devId = dr->dev_id;
          break;
        }
      }
    }

    free(dl);
  } else {
    devId = *pDevId;
  }

  return devId;
}


int iotjs_blehcisocket_kernelDisconnectWorkArounds(THIS, int length,
                                                   char* data) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_blehcisocket_t, blehcisocket);

  if (length == 22 && data[0] == 0x04 && data[1] == 0x3e && data[2] == 0x13 &&
      data[3] == 0x01 && data[4] == 0x00) {
    int l2socket;
    struct sockaddr_l2 l2a;
    unsigned short l2cid;
    unsigned short handle = *((unsigned short*)(&data[5]));

#if __BYTE_ORDER == __LITTLE_ENDIAN
    l2cid = ATT_CID;
#elif __BYTE_ORDER == __BIG_ENDIAN
    l2cid = bswap_16(ATT_CID);
#else
#error "Unknown byte order"
#endif

    l2socket = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

    if (l2socket < 0) {
      DLOG("ERROR creating socket: %s", strerror(errno));
      return -1;
    }

    memset(&l2a, 0, sizeof(l2a));
    l2a.l2_family = AF_BLUETOOTH;
    l2a.l2_cid = l2cid;
    memcpy(&l2a.l2_bdaddr, _this->_address, sizeof(l2a.l2_bdaddr));
    l2a.l2_bdaddr_type = _this->_addressType;

    if (bind(l2socket, (struct sockaddr*)&l2a, sizeof(l2a)) < 0) {
      DLOG("ERROR on binding: %s", strerror(errno));
      close(l2socket);
      return -1;
    }

    memset(&l2a, 0, sizeof(l2a));
    l2a.l2_family = AF_BLUETOOTH;
    memcpy(&l2a.l2_bdaddr, &data[9], sizeof(l2a.l2_bdaddr));
    l2a.l2_cid = l2cid;
    l2a.l2_bdaddr_type = data[8] + 1;

    if (connect(l2socket, (struct sockaddr*)&l2a, sizeof(l2a)) < 0) {
      DLOG("ERROR connecting socket: %s", strerror(errno));
      close(l2socket);
      return -1;
    }

    _this->_l2sockets[handle] = l2socket;
    _this->_l2socketCount++;
  } else if (length == 7 && data[0] == 0x04 && data[1] == 0x05 &&
             data[2] == 0x04 && data[3] == 0x00) {
    unsigned short handle = *((unsigned short*)(&data[4]));

    if (_this->_l2socketCount > 0) {
      close(_this->_l2sockets[handle]);
      _this->_l2socketCount--;
    }
  }

  return 0;
}

#undef THIS

void iotjs_blehcisocket_poll_cb(uv_poll_t* handle, int status, int events) {
  iotjs_blehcisocket_t* blehcisocket = (iotjs_blehcisocket_t*)handle->data;

  iotjs_blehcisocket_poll(blehcisocket);
}

#endif /* IOTJS_MODULE_BLE_HCI_SOCKET_LINUX_GENERAL_INL_H */
