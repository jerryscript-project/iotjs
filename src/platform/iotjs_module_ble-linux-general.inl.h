/* The MIT License (MIT)
 *
 * Copyright (c) 2013 Sandeep Mistry
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* Copyright 2016-present Samsung Electronics Co., Ltd. and other contributors
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

/* This file include some APIs in 'bleno'.
 * (https://github.com/sandeepmistry/bleno)
 */

#ifndef IOTJS_MODULE_BLE_LINUX_GENERAL_INL_H
#define IOTJS_MODULE_BLE_LINUX_GENERAL_INL_H

#include <errno.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "module/iotjs_module_ble.h"


#define BLE_WORKER_INIT_TEMPLATE                                            \
  iotjs_ble_reqwrap_t* req_wrap = iotjs_ble_reqwrap_from_request(work_req); \
  iotjs_ble_reqdata_t* req_data = iotjs_ble_reqwrap_data(req_wrap);


int hciDeviceId, hciSocket;
struct hci_dev_info hciDevInfo;
char address[18];

int previousAdapterState = -1;
int currentAdapterState;
BleState adapterState = kBleStatePoweredOff;

char advertisementDataBuf[256];
uint8_t advertisementDataLen = 0;
char scanDataBuf[256];
uint8_t scanDataLen = 0;

int hci_le_set_advertising_data(int dd, uint8_t* data, uint8_t length, int to) {
  struct hci_request rq;
  le_set_advertising_data_cp data_cp;
  uint8_t status;

  memset(&data_cp, 0, sizeof(data_cp));
  data_cp.length =
      length <= sizeof(data_cp.data) ? length : sizeof(data_cp.data);
  memcpy(&data_cp.data, data, data_cp.length);

  memset(&rq, 0, sizeof(rq));
  rq.ogf = OGF_LE_CTL;
  rq.ocf = OCF_LE_SET_ADVERTISING_DATA;
  rq.cparam = &data_cp;
  rq.clen = LE_SET_ADVERTISING_DATA_CP_SIZE;
  rq.rparam = &status;
  rq.rlen = 1;

  if (hci_send_req(dd, &rq, to) < 0)
    return -1;

  if (status) {
    errno = EIO;
    return -1;
  }

  return 0;
}

int hci_le_set_scan_response_data(int dd, uint8_t* data, uint8_t length,
                                  int to) {
  struct hci_request rq;
  le_set_scan_response_data_cp data_cp;
  uint8_t status;

  memset(&data_cp, 0, sizeof(data_cp));
  data_cp.length =
      length <= sizeof(data_cp.data) ? length : sizeof(data_cp.data);
  memcpy(&data_cp.data, data, data_cp.length);

  memset(&rq, 0, sizeof(rq));
  rq.ogf = OGF_LE_CTL;
  rq.ocf = OCF_LE_SET_SCAN_RESPONSE_DATA;
  rq.cparam = &data_cp;
  rq.clen = LE_SET_SCAN_RESPONSE_DATA_CP_SIZE;
  rq.rparam = &status;
  rq.rlen = 1;

  if (hci_send_req(dd, &rq, to) < 0)
    return -1;

  if (status) {
    errno = EIO;
    return -1;
  }

  return 0;
}

// Set advertising interval to 100 ms
// Note: 0x00A0 * 0.625ms = 100ms
int hci_le_set_advertising_parameters(int dd, int to) {
  struct hci_request rq;
  le_set_advertising_parameters_cp adv_params_cp;
  uint8_t status;

  memset(&adv_params_cp, 0, sizeof(adv_params_cp));
  adv_params_cp.min_interval = htobs(0x00A0);
  adv_params_cp.max_interval = htobs(0x00A0);
  adv_params_cp.chan_map = 7;

  memset(&rq, 0, sizeof(rq));
  rq.ogf = OGF_LE_CTL;
  rq.ocf = OCF_LE_SET_ADVERTISING_PARAMETERS;
  rq.cparam = &adv_params_cp;
  rq.clen = LE_SET_ADVERTISING_PARAMETERS_CP_SIZE;
  rq.rparam = &status;
  rq.rlen = 1;

  if (hci_send_req(dd, &rq, to) < 0)
    return -1;

  if (status) {
    errno = EIO;
    return -1;
  }

  return 0;
}

void get_adapter_state() {
  // get HCI dev info for adapter state
  ioctl(hciSocket, HCIGETDEVINFO, (void*)&hciDevInfo);
  currentAdapterState = hci_test_bit(HCI_UP, &hciDevInfo.flags);

  if (previousAdapterState != currentAdapterState) {
    previousAdapterState = currentAdapterState;

    if (!currentAdapterState) {
      adapterState = kBleStatePoweredOff;
    } else {
      hci_le_set_advertise_enable(hciSocket, 0, 1000);
      hci_le_set_advertise_enable(hciSocket, 1, 1000);

      if (hci_le_set_advertise_enable(hciSocket, 0, 1000) == -1) {
        if (EPERM == errno) {
          adapterState = kBleStateUnauthorized;
        } else if (EIO == errno) {
          adapterState = kBleStateUnsupported;
        } else {
          adapterState = kBleStateUnknown;
        }
      } else {
        adapterState = kBleStatePoweredOn;
      }
    }
    ba2str(&hciDevInfo.bdaddr, address);
  }
}

void RunBleLoopWorker(uv_work_t* work_req) {
  BLE_WORKER_INIT_TEMPLATE;

  get_adapter_state();
  req_data->state = adapterState;
}


void InitWorker(uv_work_t* work_req) {
  BLE_WORKER_INIT_TEMPLATE;

  memset(&hciDevInfo, 0x00, sizeof(hciDevInfo));

  hciDeviceId = hci_get_route(NULL);
  hciSocket = hci_open_dev(hciDeviceId);
  hciDevInfo.dev_id = hciDeviceId;

  if (hciSocket < 0) {
    req_data->state = kBleStateUnknown;
    return;
  }

  get_adapter_state();
  req_data->state = adapterState;
}


void StartAdvertisingWorker(uv_work_t* work_req) {
  BLE_WORKER_INIT_TEMPLATE;

  advertisementDataLen = req_data->advertisement_len;
  for (int i = 0; i < advertisementDataLen; i++) {
    advertisementDataBuf[i] = req_data->advertisement_data[i];
  }

  scanDataLen = req_data->scan_len;
  for (int i = 0; i < scanDataLen; i++) {
    scanDataBuf[i] = req_data->scan_data[i];
  }

  // stop advertising
  hci_le_set_advertise_enable(hciSocket, 0, 1000);

  // set scan data
  hci_le_set_scan_response_data(hciSocket, (uint8_t*)&scanDataBuf, scanDataLen,
                                1000);

  // set advertisement data
  hci_le_set_advertising_data(hciSocket, (uint8_t*)&advertisementDataBuf,
                              advertisementDataLen, 1000);

  // set advertisement parameters,
  // mostly to set the advertising interval to 100ms
  hci_le_set_advertising_parameters(hciSocket, 1000);

  // start advertising
  hci_le_set_advertise_enable(hciSocket, 1, 1000);

  // set scan data
  hci_le_set_scan_response_data(hciSocket, (uint8_t*)&scanDataBuf, scanDataLen,
                                1000);

  // set advertisement data
  hci_le_set_advertising_data(hciSocket, (uint8_t*)&advertisementDataBuf,
                              advertisementDataLen, 1000);
}


void StopAdvertisingWorker(uv_work_t* work_req) {
  BLE_WORKER_INIT_TEMPLATE;

  // stop advertising
  hci_le_set_advertise_enable(hciSocket, 0, 1000);
  close(hciSocket);
}


void SetServicesWorker(uv_work_t* work_req) {
  BLE_WORKER_INIT_TEMPLATE;
  IOTJS_ASSERT(!"Not implemented");
}


#endif /* IOTJS_MODULE_BLE_LINUX_GENERAL_INL_H */
