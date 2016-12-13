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


#ifndef IOTJS_MODULE_BLE_H
#define IOTJS_MODULE_BLE_H

#include "iotjs_def.h"
#include "iotjs_objectwrap.h"
#include "iotjs_reqwrap.h"

#define BLE_MAX_UUID_NUM 7

typedef enum {
  kBleOpInit,
  kBleOpStartAdvertising,
  kBleOpStopAdvertising,
  kBleOpSetServices,
  kBleOpRunBleLoop,
} BleOp;

typedef enum {
  kBleErrOk = 0,
  kBleErrInit = -1,
  kBleErrStart = -2,
  kBleErrStop = -3,
  kBleErrSetServices = -4,
} BleError;

typedef enum {
  kBleStatePoweredOn,
  kBleStatePoweredOff,
  kBleStateUnauthorized,
  kBleStateUnsupported,
  kBleStateUnknown,
} BleState;

typedef struct {
  BleOp op;
  BleError error;
  BleState state;

  char advertisement_data[256];
  uint8_t advertisement_len;
  char scan_data[256];
  uint8_t scan_len;
} iotjs_ble_reqdata_t;


typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_work_t req;
  iotjs_ble_reqdata_t req_data;
} IOTJS_VALIDATED_STRUCT(iotjs_ble_reqwrap_t);


#define THIS iotjs_ble_reqwrap_t* ble_reqwrap
iotjs_ble_reqwrap_t* iotjs_ble_reqwrap_create(const iotjs_jval_t* jcallback,
                                              BleOp op);
void iotjs_ble_reqwrap_dispatched(THIS);
uv_work_t* iotjs_ble_reqwrap_req(THIS);
const iotjs_jval_t* iotjs_ble_reqwrap_jcallback(THIS);
iotjs_ble_reqwrap_t* iotjs_ble_reqwrap_from_request(uv_work_t* req);
iotjs_ble_reqdata_t* iotjs_ble_reqwrap_data(THIS);
#undef THIS


// This Ble class provides interfaces for BLE operation.
typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;
} IOTJS_VALIDATED_STRUCT(iotjs_ble_t);

iotjs_ble_t* iotjs_ble_create(const iotjs_jval_t* jble);
const iotjs_jval_t* iotjs_ble_get_jble();
iotjs_ble_t* iotjs_ble_get_instance();

void InitWorker(uv_work_t* work_req);
void StartAdvertisingWorker(uv_work_t* work_req);
void StopAdvertisingWorker(uv_work_t* work_req);
void SetServicesWorker(uv_work_t* work_req);
void RunBleLoopWorker(uv_work_t* work_req);

#endif /* IOTJS_MODULE_BLE_H */
