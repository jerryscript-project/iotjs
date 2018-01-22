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
#include "iotjs_module_adc.h"


static JNativeInfoType this_module_native_info = {.free_cb = NULL };


static iotjs_adc_t* adc_create(const jerry_value_t jadc) {
  iotjs_adc_t* adc = IOTJS_ALLOC(iotjs_adc_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_adc_t, adc);
  iotjs_adc_create_platform_data(adc);
  _this->jobject = jadc;
  jerry_set_object_native_pointer(jadc, adc, &this_module_native_info);

  return adc;
}


static void adc_destroy(iotjs_adc_t* adc) {
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_adc_t, adc);
  iotjs_adc_destroy_platform_data(_this->platform_data);
  IOTJS_RELEASE(adc);
}


static iotjs_adc_reqwrap_t* adc_reqwrap_create(const jerry_value_t jcallback,
                                               iotjs_adc_t* adc, AdcOp op) {
  iotjs_adc_reqwrap_t* adc_reqwrap = IOTJS_ALLOC(iotjs_adc_reqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_adc_reqwrap_t, adc_reqwrap);

  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);

  _this->req_data.op = op;
  _this->adc_data = adc;
  return adc_reqwrap;
}

static void adc_reqwrap_destroy(iotjs_adc_reqwrap_t* adc_reqwrap) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_adc_reqwrap_t, adc_reqwrap);
  iotjs_reqwrap_destroy(&_this->reqwrap);
  IOTJS_RELEASE(adc_reqwrap);
}


static void adc_reqwrap_dispatched(iotjs_adc_reqwrap_t* adc_reqwrap) {
  IOTJS_VALIDATABLE_STRUCT_METHOD_VALIDATE(iotjs_adc_reqwrap_t, adc_reqwrap);
  adc_reqwrap_destroy(adc_reqwrap);
}


static uv_work_t* adc_reqwrap_req(iotjs_adc_reqwrap_t* adc_reqwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_reqwrap_t, adc_reqwrap);
  return &_this->req;
}


static jerry_value_t adc_reqwrap_jcallback(iotjs_adc_reqwrap_t* adc_reqwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_reqwrap_t, adc_reqwrap);
  return iotjs_reqwrap_jcallback(&_this->reqwrap);
}


static iotjs_adc_t* adc_instance_from_jval(const jerry_value_t jadc) {
  uintptr_t handle = iotjs_jval_get_object_native_handle(jadc);
  return (iotjs_adc_t*)handle;
}


static iotjs_adc_reqwrap_t* adc_reqwrap_from_request(uv_work_t* req) {
  return (iotjs_adc_reqwrap_t*)(iotjs_reqwrap_from_request((uv_req_t*)req));
}


static iotjs_adc_reqdata_t* adc_reqwrap_data(iotjs_adc_reqwrap_t* adc_reqwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_reqwrap_t, adc_reqwrap);
  return &_this->req_data;
}


static iotjs_adc_t* adc_instance_from_reqwrap(
    iotjs_adc_reqwrap_t* adc_reqwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_reqwrap_t, adc_reqwrap);
  return _this->adc_data;
}


static void adc_worker(uv_work_t* work_req) {
  iotjs_adc_reqwrap_t* req_wrap = adc_reqwrap_from_request(work_req);
  iotjs_adc_reqdata_t* req_data = adc_reqwrap_data(req_wrap);
  iotjs_adc_t* adc = adc_instance_from_reqwrap(req_wrap);

  switch (req_data->op) {
    case kAdcOpOpen:
      req_data->result = iotjs_adc_open(adc);
      break;
    case kAdcOpRead:
      req_data->result = iotjs_adc_read(adc);
      break;
    case kAdcOpClose:
      req_data->result = iotjs_adc_close(adc);
      break;
    default:
      IOTJS_ASSERT(!"Invalid Adc Operation");
  }
}


static const char* adc_error_string(uint8_t op) {
  switch (op) {
    case kAdcOpClose:
      return "Close error, cannot close ADC";
    case kAdcOpOpen:
      return "Open error, cannot open ADC";
    case kAdcOpRead:
      return "Read error, cannot read ADC";
    default:
      return "Unknown ADC error";
  }
}


static void adc_after_worker(uv_work_t* work_req, int status) {
  iotjs_adc_reqwrap_t* req_wrap = adc_reqwrap_from_request(work_req);
  iotjs_adc_reqdata_t* req_data = adc_reqwrap_data(req_wrap);

  iotjs_jargs_t jargs = iotjs_jargs_create(2);
  bool result = req_data->result;

  if (status) {
    iotjs_jargs_append_error(&jargs, "ADC System Error");
  } else {
    switch (req_data->op) {
      case kAdcOpOpen:
        if (!result) {
          iotjs_jargs_append_error(&jargs, adc_error_string(req_data->op));
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      case kAdcOpRead:
        if (!result) {
          iotjs_jargs_append_error(&jargs, adc_error_string(req_data->op));
        } else {
          iotjs_adc_t* adc = adc_instance_from_reqwrap(req_wrap);
          IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_t, adc);

          iotjs_jargs_append_null(&jargs);
          iotjs_jargs_append_number(&jargs, _this->value);
        }
        break;
      case kAdcOpClose:
        if (!result) {
          iotjs_jargs_append_error(&jargs, adc_error_string(req_data->op));
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      default: {
        IOTJS_ASSERT(!"ADC after worker failed");
        break;
      }
    }
  }

  const jerry_value_t jcallback = adc_reqwrap_jcallback(req_wrap);
  if (jerry_value_is_function(jcallback)) {
    iotjs_make_callback(jcallback, jerry_create_undefined(), &jargs);
  }

  if (req_data->op == kAdcOpClose) {
    IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_reqwrap_t, req_wrap);
    adc_destroy(_this->adc_data);
  }

  iotjs_jargs_destroy(&jargs);
  adc_reqwrap_dispatched(req_wrap);
}


#define ADC_CALL_ASYNC(op, jcallback)                                       \
  do {                                                                      \
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());      \
    iotjs_adc_reqwrap_t* req_wrap = adc_reqwrap_create(jcallback, adc, op); \
    uv_work_t* req = adc_reqwrap_req(req_wrap);                             \
    uv_queue_work(loop, req, adc_worker, adc_after_worker);                 \
  } while (0)


JS_FUNCTION(AdcCons) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, object);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  // Create ADC object
  const jerry_value_t jadc = JS_GET_THIS();
  iotjs_adc_t* adc = adc_create(jadc);
  IOTJS_ASSERT(adc == adc_instance_from_jval(jadc));

  jerry_value_t jconfig;
  JS_GET_REQUIRED_ARG_VALUE(0, jconfig, IOTJS_MAGIC_STRING_CONFIG, object);

  jerry_value_t config_res = iotjs_adc_set_platform_config(adc, jconfig);
  if (jerry_value_has_error_flag(config_res)) {
    return config_res;
  }
  IOTJS_ASSERT(jerry_value_is_undefined(config_res));

  jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  // If the callback doesn't exist, it is completed synchronously.
  // Otherwise, it will be executed asynchronously.
  if (!jerry_value_is_null(jcallback)) {
    ADC_CALL_ASYNC(kAdcOpOpen, jcallback);
  } else if (!iotjs_adc_open(adc)) {
    return JS_CREATE_ERROR(COMMON, adc_error_string(kAdcOpOpen));
  }

  return jerry_create_undefined();
}


JS_FUNCTION(Read) {
  JS_DECLARE_THIS_PTR(adc, adc);
  DJS_CHECK_ARG_IF_EXIST(0, function);

  ADC_CALL_ASYNC(kAdcOpRead, JS_GET_ARG_IF_EXIST(0, function));

  return jerry_create_undefined();
}

JS_FUNCTION(ReadSync) {
  JS_DECLARE_THIS_PTR(adc, adc);

  if (!iotjs_adc_read(adc)) {
    return JS_CREATE_ERROR(COMMON, adc_error_string(kAdcOpRead));
  }

  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_t, adc);

  return jerry_create_number(_this->value);
}

JS_FUNCTION(Close) {
  JS_DECLARE_THIS_PTR(adc, adc);
  DJS_CHECK_ARG_IF_EXIST(0, function);

  ADC_CALL_ASYNC(kAdcOpClose, JS_GET_ARG_IF_EXIST(0, function));

  return jerry_create_undefined();
}

JS_FUNCTION(CloseSync) {
  JS_DECLARE_THIS_PTR(adc, adc);

  bool ret = iotjs_adc_close(adc);
  adc_destroy(adc);
  if (!ret) {
    return JS_CREATE_ERROR(COMMON, adc_error_string(kAdcOpClose));
  }

  return jerry_create_undefined();
}

jerry_value_t InitAdc() {
  jerry_value_t jadc_cons = jerry_create_external_function(AdcCons);
  jerry_value_t jprototype = jerry_create_object();

  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_READ, Read);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_READSYNC, ReadSync);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_CLOSE, Close);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_CLOSESYNC, CloseSync);

  iotjs_jval_set_property_jval(jadc_cons, IOTJS_MAGIC_STRING_PROTOTYPE,
                               jprototype);

  jerry_release_value(jprototype);

  return jadc_cons;
}
