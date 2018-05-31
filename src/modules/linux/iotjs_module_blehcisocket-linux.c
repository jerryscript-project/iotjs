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


void iotjs_blehcisocket_initialize(iotjs_blehcisocket_t* blehcisocket) {
  blehcisocket->_l2socketCount = 0;

  blehcisocket->_socket =
      socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);

  uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());
  uv_poll_init(loop, &(blehcisocket->_pollHandle), blehcisocket->_socket);

  blehcisocket->_pollHandle.data = blehcisocket;
}


void iotjs_blehcisocket_close(iotjs_blehcisocket_t* blehcisocket) {
  uv_close((uv_handle_t*)&(blehcisocket->_pollHandle), NULL);

  close(blehcisocket->_socket);
}


void iotjs_blehcisocket_start(iotjs_blehcisocket_t* blehcisocket) {
  uv_poll_start(&blehcisocket->_pollHandle, UV_READABLE,
                iotjs_blehcisocket_poll_cb);
}


int iotjs_blehcisocket_bindRaw(iotjs_blehcisocket_t* blehcisocket, int* devId) {
  struct sockaddr_hci a;
  struct hci_dev_info di;

  memset(&a, 0, sizeof(a));
  a.hci_family = AF_BLUETOOTH;
  a.hci_dev = iotjs_blehcisocket_devIdFor(blehcisocket, devId, true);
  a.hci_channel = HCI_CHANNEL_RAW;

  blehcisocket->_devId = a.hci_dev;
  blehcisocket->_mode = HCI_CHANNEL_RAW;

  if (bind(blehcisocket->_socket, (struct sockaddr*)&a, sizeof(a)) < 0) {
    DLOG("ERROR on binding: %s", strerror(errno));
    return blehcisocket->_devId;
  }

  // get the local address and address type
  memset(&di, 0x00, sizeof(di));
  di.dev_id = blehcisocket->_devId;
  memset(blehcisocket->_address, 0, sizeof(blehcisocket->_address));
  blehcisocket->_addressType = 0;

  if (ioctl(blehcisocket->_socket, HCIGETDEVINFO, (void*)&di) > -1) {
    memcpy(blehcisocket->_address, &di.bdaddr, sizeof(di.bdaddr));
    blehcisocket->_addressType = di.type;

    if (blehcisocket->_addressType == 3) {
      // 3 is a weird type, use 1 (public) instead
      blehcisocket->_addressType = 1;
    }
  }

  return blehcisocket->_devId;
}


int iotjs_blehcisocket_bindUser(iotjs_blehcisocket_t* blehcisocket,
                                int* devId) {
  struct sockaddr_hci a;

  memset(&a, 0, sizeof(a));
  a.hci_family = AF_BLUETOOTH;
  a.hci_dev = iotjs_blehcisocket_devIdFor(blehcisocket, devId, false);
  a.hci_channel = HCI_CHANNEL_USER;

  blehcisocket->_devId = a.hci_dev;
  blehcisocket->_mode = HCI_CHANNEL_USER;

  if (bind(blehcisocket->_socket, (struct sockaddr*)&a, sizeof(a)) < 0) {
    DLOG("ERROR on binding: %s", strerror(errno));
  }

  return blehcisocket->_devId;
}


void iotjs_blehcisocket_bindControl(iotjs_blehcisocket_t* blehcisocket) {
  struct sockaddr_hci a;

  memset(&a, 0, sizeof(a));
  a.hci_family = AF_BLUETOOTH;
  a.hci_dev = HCI_DEV_NONE;
  a.hci_channel = HCI_CHANNEL_CONTROL;

  blehcisocket->_mode = HCI_CHANNEL_CONTROL;

  if (bind(blehcisocket->_socket, (struct sockaddr*)&a, sizeof(a)) < 0) {
    DLOG("ERROR on binding: %s", strerror(errno));
  }
}


bool iotjs_blehcisocket_isDevUp(iotjs_blehcisocket_t* blehcisocket) {
  struct hci_dev_info di;
  bool isUp = false;

  memset(&di, 0x00, sizeof(di));
  di.dev_id = blehcisocket->_devId;

  if (ioctl(blehcisocket->_socket, HCIGETDEVINFO, (void*)&di) > -1) {
    isUp = (di.flags & (1 << HCI_UP)) != 0;
  }

  return isUp;
}


void iotjs_blehcisocket_setFilter(iotjs_blehcisocket_t* blehcisocket,
                                  char* data, size_t length) {
  if (setsockopt(blehcisocket->_socket, SOL_HCI, HCI_FILTER, data,
                 (socklen_t)length) < 0) {
    iotjs_blehcisocket_emitErrnoError(blehcisocket);
  }
}


void iotjs_blehcisocket_poll(iotjs_blehcisocket_t* blehcisocket) {
  int length = 0;
  char data[1024];

  length = read(blehcisocket->_socket, data, sizeof(data));

  if (length > 0) {
    if (blehcisocket->_mode == HCI_CHANNEL_RAW) {
      if (iotjs_blehcisocket_kernelDisconnectWorkArounds(blehcisocket, length,
                                                         data) < 0) {
        return;
      }
    }

    jerry_value_t jhcisocket = blehcisocket->jobject;
    jerry_value_t jemit = iotjs_jval_get_property(jhcisocket, "emit");
    IOTJS_ASSERT(jerry_value_is_function(jemit));

    jerry_value_t str = jerry_create_string((const jerry_char_t*)"data");
    IOTJS_ASSERT(length >= 0);
    jerry_value_t jbuf = iotjs_bufferwrap_create_buffer((size_t)length);
    iotjs_bufferwrap_t* buf_wrap = iotjs_bufferwrap_from_jbuffer(jbuf);
    iotjs_bufferwrap_copy(buf_wrap, data, (size_t)length);
    jerry_value_t jargs[2] = { str, jbuf };
    jerry_value_t jres = jerry_call_function(jemit, jhcisocket, jargs, 2);
    IOTJS_ASSERT(!jerry_value_is_error(jres));

    jerry_release_value(jres);
    jerry_release_value(str);
    jerry_release_value(jbuf);
    jerry_release_value(jemit);
  }
}


void iotjs_blehcisocket_stop(iotjs_blehcisocket_t* blehcisocket) {
  uv_poll_stop(&blehcisocket->_pollHandle);
}


void iotjs_blehcisocket_write(iotjs_blehcisocket_t* blehcisocket, char* data,
                              size_t length) {
  if (write(blehcisocket->_socket, data, length) < 0) {
    iotjs_blehcisocket_emitErrnoError(blehcisocket);
  }
}


void iotjs_blehcisocket_emitErrnoError(iotjs_blehcisocket_t* blehcisocket) {
  jerry_value_t jhcisocket = blehcisocket->jobject;
  jerry_value_t jemit = iotjs_jval_get_property(jhcisocket, "emit");
  IOTJS_ASSERT(jerry_value_is_function(jemit));

  jerry_value_t str = jerry_create_string((const jerry_char_t*)"error");
  jerry_value_t jerror =
      iotjs_jval_create_error_without_error_flag(strerror(errno));
  jerry_value_t jargs[2] = { str, jerror };
  jerry_value_t jres = jerry_call_function(jemit, jhcisocket, jargs, 2);
  IOTJS_ASSERT(!jerry_value_is_error(jres));

  jerry_release_value(jres);
  jerry_release_value(str);
  jerry_release_value(jerror);
  jerry_release_value(jemit);
}


int iotjs_blehcisocket_devIdFor(iotjs_blehcisocket_t* blehcisocket, int* pDevId,
                                bool isUp) {
  int devId = 0; // default

  if (pDevId == NULL) {
    struct hci_dev_list_req* dl;
    struct hci_dev_req* dr;

    dl = (struct hci_dev_list_req*)calloc(HCI_MAX_DEV * sizeof(*dr) +
                                              sizeof(*dl),
                                          1);
    dr = dl->dev_req;

    dl->dev_num = HCI_MAX_DEV;

    if (ioctl(blehcisocket->_socket, HCIGETDEVLIST, dl) > -1) {
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


int iotjs_blehcisocket_kernelDisconnectWorkArounds(
    iotjs_blehcisocket_t* blehcisocket, int length, char* data) {
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
    memcpy(&l2a.l2_bdaddr, blehcisocket->_address, sizeof(l2a.l2_bdaddr));
    l2a.l2_bdaddr_type = blehcisocket->_addressType;

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

    blehcisocket->_l2sockets[handle] = l2socket;
    blehcisocket->_l2socketCount++;
  } else if (length == 7 && data[0] == 0x04 && data[1] == 0x05 &&
             data[2] == 0x04 && data[3] == 0x00) {
    unsigned short handle = *((unsigned short*)(&data[4]));

    if (blehcisocket->_l2socketCount > 0) {
      close(blehcisocket->_l2sockets[handle]);
      blehcisocket->_l2socketCount--;
    }
  }

  return 0;
}

void iotjs_blehcisocket_poll_cb(uv_poll_t* handle, int status, int events) {
  iotjs_blehcisocket_t* blehcisocket = (iotjs_blehcisocket_t*)handle->data;

  iotjs_blehcisocket_poll(blehcisocket);
}
