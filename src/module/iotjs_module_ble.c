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


#include "iotjs_def.h"
#include "iotjs_module_ble.h"
#include "iotjs_objectwrap.h"


#define THIS iotjs_ble_reqwrap_t* ble_reqwrap


iotjs_ble_reqwrap_t* iotjs_ble_reqwrap_create(const iotjs_jval_t* jcallback,
                                              BleOp op) {
  iotjs_ble_reqwrap_t* ble_reqwrap = IOTJS_ALLOC(iotjs_ble_reqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_ble_reqwrap_t, ble_reqwrap);

  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);

  _this->req_data.op = op;

  return ble_reqwrap;
}


static void iotjs_ble_reqwrap_destroy(THIS) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_ble_reqwrap_t, ble_reqwrap);
  iotjs_reqwrap_destroy(&_this->reqwrap);
  IOTJS_RELEASE(ble_reqwrap);
}


void iotjs_ble_reqwrap_dispatched(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_ble_reqwrap_t, ble_reqwrap);
  iotjs_ble_reqwrap_destroy(ble_reqwrap);
}


uv_work_t* iotjs_ble_reqwrap_req(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_ble_reqwrap_t, ble_reqwrap);
  return &_this->req;
}


const iotjs_jval_t* iotjs_ble_reqwrap_jcallback(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_ble_reqwrap_t, ble_reqwrap);
  return iotjs_reqwrap_jcallback(&_this->reqwrap);
}


iotjs_ble_reqwrap_t* iotjs_ble_reqwrap_from_request(uv_work_t* req) {
  return (iotjs_ble_reqwrap_t*)(iotjs_reqwrap_from_request((uv_req_t*)req));
}


iotjs_ble_reqdata_t* iotjs_ble_reqwrap_data(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_ble_reqwrap_t, ble_reqwrap);
  return &_this->req_data;
}


#undef THIS


static void iotjs_ble_destroy(iotjs_ble_t* ble);


iotjs_ble_t* iotjs_ble_create(const iotjs_jval_t* jble) {
  iotjs_ble_t* ble = IOTJS_ALLOC(iotjs_ble_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_ble_t, ble);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, jble,
                               (JFreeHandlerType)iotjs_ble_destroy);
  return ble;
}


static void iotjs_ble_destroy(iotjs_ble_t* ble) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_ble_t, ble);
  iotjs_jobjectwrap_destroy(&_this->jobjectwrap);
  IOTJS_RELEASE(ble);
}


const iotjs_jval_t* iotjs_ble_get_jble() {
  return iotjs_module_get(MODULE_BLE);
}


iotjs_ble_t* iotjs_ble_get_instance() {
  const iotjs_jval_t* jble = iotjs_ble_get_jble();
  iotjs_jobjectwrap_t* jobjectwrap = iotjs_jobjectwrap_from_jobject(jble);
  return (iotjs_ble_t*)jobjectwrap;
}


void AfterBLEWork(uv_work_t* work_req, int status) {
  iotjs_ble_t* ble = iotjs_ble_get_instance();

  iotjs_ble_reqwrap_t* req_wrap = iotjs_ble_reqwrap_from_request(work_req);
  iotjs_ble_reqdata_t* req_data = iotjs_ble_reqwrap_data(req_wrap);

  iotjs_jargs_t jargs = iotjs_jargs_create(1);

  if (status) {
    iotjs_jval_t error = iotjs_jval_create_error("System error");
    iotjs_jargs_append_jval(&jargs, &error);
    iotjs_jval_destroy(&error);
  } else {
    switch (req_data->op) {
      case kBleOpInit: {
        if (req_data->state == kBleStatePoweredOn) {
          iotjs_jargs_append_string_raw(&jargs, "poweredOn");
        } else if (req_data->state == kBleStatePoweredOff) {
          iotjs_jargs_append_string_raw(&jargs, "poweredOff");
        } else if (req_data->state == kBleStateUnauthorized) {
          iotjs_jargs_append_string_raw(&jargs, "unauthorized");
        } else if (req_data->state == kBleStateUnsupported) {
          iotjs_jargs_append_string_raw(&jargs, "unsupported");
        } else {
          iotjs_jargs_append_string_raw(&jargs, "unknown");
        }
        break;
      }
      case kBleOpStartAdvertising: {
        iotjs_jargs_append_null(&jargs);
        break;
      }
      case kBleOpStopAdvertising: {
        iotjs_jargs_append_null(&jargs);
        break;
      }
      case kBleOpRunBleLoop: {
        if (req_data->state == kBleStatePoweredOn) {
          iotjs_jargs_append_string_raw(&jargs, "poweredOn");
        } else if (req_data->state == kBleStatePoweredOff) {
          iotjs_jargs_append_string_raw(&jargs, "poweredOff");
        } else if (req_data->state == kBleStateUnauthorized) {
          iotjs_jargs_append_string_raw(&jargs, "unauthorized");
        } else if (req_data->state == kBleStateUnsupported) {
          iotjs_jargs_append_string_raw(&jargs, "unsupported");
        } else {
          iotjs_jargs_append_string_raw(&jargs, "unknown");
        }
        break;
      }
      case kBleOpSetServices: {
        iotjs_jargs_append_null(&jargs);
        break;
      }
      default: {
        IOTJS_ASSERT(!"Unreachable");
        break;
      }
    }
  }

  const iotjs_jval_t* jcallback = iotjs_ble_reqwrap_jcallback(req_wrap);
  const iotjs_jval_t* jble = iotjs_ble_get_jble();
  iotjs_make_callback(jcallback, jble, &jargs);

  iotjs_jargs_destroy(&jargs);
  iotjs_ble_reqwrap_dispatched(req_wrap);
}

static void GetBleArray(const iotjs_jval_t* jadvarray,
                        const iotjs_jval_t* jscanarray,
                        iotjs_ble_reqdata_t* req_data) {
  iotjs_jval_t jadvlength = iotjs_jval_get_property(jadvarray, "length");
  IOTJS_ASSERT(!iotjs_jval_is_undefined(&jadvlength));

  req_data->advertisement_len = iotjs_jval_as_number(&jadvlength);

  for (int i = 0; i < req_data->advertisement_len; i++) {
    iotjs_jval_t jdata = iotjs_jval_get_property_by_index(jadvarray, i);
    req_data->advertisement_data[i] = iotjs_jval_as_number(&jdata);
    iotjs_jval_destroy(&jdata);
  }

  iotjs_jval_destroy(&jadvlength);

  iotjs_jval_t jscanlength = iotjs_jval_get_property(jscanarray, "length");
  IOTJS_ASSERT(!iotjs_jval_is_undefined(&jscanlength));

  req_data->scan_len = iotjs_jval_as_number(&jscanlength);

  for (int i = 0; i < req_data->scan_len; i++) {
    iotjs_jval_t jdata = iotjs_jval_get_property_by_index(jscanarray, i);
    req_data->scan_data[i] = iotjs_jval_as_number(&jdata);
    iotjs_jval_destroy(&jdata);
  }

  iotjs_jval_destroy(&jscanlength);
}


#define BLE_ASYNC(op)                                                  \
  do {                                                                 \
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get()); \
    uv_work_t* req = iotjs_ble_reqwrap_req(req_wrap);                  \
    uv_queue_work(loop, req, op##Worker, AfterBLEWork);                \
  } while (0)


JHANDLER_FUNCTION(Init) {
  JHANDLER_CHECK_ARGS(1, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(0, function);

  iotjs_ble_reqwrap_t* req_wrap =
      iotjs_ble_reqwrap_create(jcallback, kBleOpInit);

  BLE_ASYNC(Init);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(StartAdvertising) {
  JHANDLER_CHECK_ARGS(3, array, array, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(2, function);

  iotjs_ble_reqwrap_t* req_wrap =
      iotjs_ble_reqwrap_create(jcallback, kBleOpStartAdvertising);

  iotjs_ble_reqdata_t* req_data = iotjs_ble_reqwrap_data(req_wrap);

  const iotjs_jval_t* jadvarray = JHANDLER_GET_ARG(0, array);
  const iotjs_jval_t* jscanarray = JHANDLER_GET_ARG(1, array);

  GetBleArray(jadvarray, jscanarray, req_data);

  BLE_ASYNC(StartAdvertising);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(StopAdvertising) {
  JHANDLER_CHECK_ARGS(1, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(0, function);

  iotjs_ble_reqwrap_t* req_wrap =
      iotjs_ble_reqwrap_create(jcallback, kBleOpStopAdvertising);

  iotjs_ble_reqdata_t* req_data = iotjs_ble_reqwrap_data(req_wrap);

  BLE_ASYNC(StopAdvertising);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(SetServices) {
  JHANDLER_CHECK_ARGS(2, array, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(1, function);

  iotjs_ble_reqwrap_t* req_wrap =
      iotjs_ble_reqwrap_create(jcallback, kBleOpSetServices);

  iotjs_ble_reqdata_t* req_data = iotjs_ble_reqwrap_data(req_wrap);

  BLE_ASYNC(SetServices);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(RunBleLoop) {
  JHANDLER_CHECK_ARGS(1, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(0, function);

  iotjs_ble_reqwrap_t* req_wrap =
      iotjs_ble_reqwrap_create(jcallback, kBleOpRunBleLoop);

  iotjs_ble_reqdata_t* req_data = iotjs_ble_reqwrap_data(req_wrap);

  BLE_ASYNC(RunBleLoop);

  iotjs_jhandler_return_null(jhandler);
}


iotjs_jval_t InitBle() {
  iotjs_jval_t jble = iotjs_jval_create_object();

  iotjs_jval_set_method(&jble, "init", Init);
  iotjs_jval_set_method(&jble, "startAdvertising", StartAdvertising);
  iotjs_jval_set_method(&jble, "stopAdvertising", StopAdvertising);
  iotjs_jval_set_method(&jble, "setServices", SetServices);
  iotjs_jval_set_method(&jble, "runBleLoop", RunBleLoop);

  iotjs_ble_t* ble = iotjs_ble_create(&jble);
  IOTJS_ASSERT(ble ==
               (iotjs_ble_t*)(iotjs_jval_get_object_native_handle(&jble)));

  return jble;
}
