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


static iotjs_adc_t* iotjs_adc_instance_from_jval(const jerry_value_t jadc);


static iotjs_adc_t* iotjs_adc_create(const jerry_value_t jadc) {
  iotjs_adc_t* adc = IOTJS_ALLOC(iotjs_adc_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_adc_t, adc);
  _this->jobject = jadc;
  jerry_set_object_native_pointer(jadc, adc, &this_module_native_info);

  return adc;
}


static void iotjs_adc_destroy(iotjs_adc_t* adc) {
#if defined(__linux__)
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_adc_t, adc);
  iotjs_string_destroy(&_this->device);
#endif
  IOTJS_RELEASE(adc);
}


#define THIS iotjs_adc_reqwrap_t* adc_reqwrap


static iotjs_adc_reqwrap_t* iotjs_adc_reqwrap_create(
    const jerry_value_t jcallback, iotjs_adc_t* adc, AdcOp op) {
  iotjs_adc_reqwrap_t* adc_reqwrap = IOTJS_ALLOC(iotjs_adc_reqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_adc_reqwrap_t, adc_reqwrap);

  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);

  _this->req_data.op = op;
  _this->adc_instance = adc;
  return adc_reqwrap;
}

static void iotjs_adc_reqwrap_destroy(THIS) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_adc_reqwrap_t, adc_reqwrap);
  iotjs_reqwrap_destroy(&_this->reqwrap);
  IOTJS_RELEASE(adc_reqwrap);
}


static void iotjs_adc_reqwrap_dispatched(THIS) {
  IOTJS_VALIDATABLE_STRUCT_METHOD_VALIDATE(iotjs_adc_reqwrap_t, adc_reqwrap);
  iotjs_adc_reqwrap_destroy(adc_reqwrap);
}


static uv_work_t* iotjs_adc_reqwrap_req(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_reqwrap_t, adc_reqwrap);
  return &_this->req;
}


static jerry_value_t iotjs_adc_reqwrap_jcallback(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_reqwrap_t, adc_reqwrap);
  return iotjs_reqwrap_jcallback(&_this->reqwrap);
}


static iotjs_adc_t* iotjs_adc_instance_from_jval(const jerry_value_t jadc) {
  uintptr_t handle = iotjs_jval_get_object_native_handle(jadc);
  return (iotjs_adc_t*)handle;
}


iotjs_adc_reqwrap_t* iotjs_adc_reqwrap_from_request(uv_work_t* req) {
  return (iotjs_adc_reqwrap_t*)(iotjs_reqwrap_from_request((uv_req_t*)req));
}


iotjs_adc_reqdata_t* iotjs_adc_reqwrap_data(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_reqwrap_t, adc_reqwrap);
  return &_this->req_data;
}


iotjs_adc_t* iotjs_adc_instance_from_reqwrap(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_reqwrap_t, adc_reqwrap);
  return _this->adc_instance;
}


#undef THIS


static void iotjs_adc_after_work(uv_work_t* work_req, int status) {
  iotjs_adc_reqwrap_t* req_wrap = iotjs_adc_reqwrap_from_request(work_req);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_reqwrap_t, req_wrap);

  iotjs_adc_reqdata_t* req_data = &_this->req_data;
  iotjs_jargs_t jargs = iotjs_jargs_create(2);
  bool result = req_data->result;

  if (status) {
    iotjs_jargs_append_error(&jargs, "System error");
  } else {
    switch (req_data->op) {
      case kAdcOpOpen:
        if (!result) {
          iotjs_jargs_append_error(&jargs, "Failed to open ADC device");
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      case kAdcOpRead:
        if (!result) {
          iotjs_jargs_append_error(&jargs, "Cannot read from ADC device");
        } else {
          iotjs_jargs_append_null(&jargs);
          iotjs_jargs_append_number(&jargs, req_data->value);
        }
        break;
      case kAdcOpClose:
        if (!result) {
          iotjs_jargs_append_error(&jargs, "Cannot close ADC device");
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      default: {
        IOTJS_ASSERT(!"Unreachable");
        break;
      }
    }
  }

  const jerry_value_t jcallback = iotjs_adc_reqwrap_jcallback(req_wrap);
  iotjs_make_callback(jcallback, jerry_create_undefined(), &jargs);

  if (req_data->op == kAdcOpClose) {
    iotjs_adc_destroy(_this->adc_instance);
  }

  iotjs_jargs_destroy(&jargs);
  iotjs_adc_reqwrap_dispatched(req_wrap);
}


static void iotjs_adc_read_worker(uv_work_t* work_req) {
  ADC_WORKER_INIT;
  int32_t value = iotjs_adc_read(adc);

  if (value < 0) {
    req_data->result = false;
    return;
  }

  req_data->value = value;
  req_data->result = true;
}


static void iotjs_adc_close_worker(uv_work_t* work_req) {
  ADC_WORKER_INIT;

  // Release driver
  if (!iotjs_adc_close(adc)) {
    req_data->result = false;
    return;
  }

  req_data->result = true;
}


#define ADC_ASYNC(call, this, jcallback, op)                                   \
  do {                                                                         \
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());         \
    iotjs_adc_reqwrap_t* req_wrap =                                            \
        iotjs_adc_reqwrap_create(jcallback, this, op);                         \
    uv_work_t* req = iotjs_adc_reqwrap_req(req_wrap);                          \
    uv_queue_work(loop, req, iotjs_adc_##call##_worker, iotjs_adc_after_work); \
  } while (0)

JS_FUNCTION(AdcConstructor) {
  DJS_CHECK_THIS();

  // Create ADC object
  const jerry_value_t jadc = JS_GET_THIS();
  iotjs_adc_t* adc = iotjs_adc_create(jadc);
  IOTJS_ASSERT(adc == iotjs_adc_instance_from_jval(jadc));
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_t, adc);

  const jerry_value_t jconfiguration = JS_GET_ARG_IF_EXIST(0, object);
  if (jerry_value_is_null(jconfiguration)) {
    return JS_CREATE_ERROR(TYPE,
                           "Bad arguments - configuration should be Object");
  }

#if defined(__linux__)
  DJS_GET_REQUIRED_CONF_VALUE(jconfiguration, _this->device,
                              IOTJS_MAGIC_STRING_DEVICE, string);
#elif defined(__NUTTX__) || defined(__TIZENRT__)
  DJS_GET_REQUIRED_CONF_VALUE(jconfiguration, _this->pin,
                              IOTJS_MAGIC_STRING_PIN, number);
#endif

  if (jargc > 1) {
    const jerry_value_t jcallback = jargv[1];
    if (jerry_value_is_function(jcallback)) {
      ADC_ASYNC(open, adc, jcallback, kAdcOpOpen);
    } else {
      return JS_CREATE_ERROR(TYPE,
                             "Bad arguments - callback should be Function");
    }
  } else {
    jerry_value_t jdummycallback =
        iotjs_jval_create_function(&iotjs_jval_dummy_function);
    ADC_ASYNC(open, adc, jdummycallback, kAdcOpOpen);
    jerry_release_value(jdummycallback);
  }

  return jerry_create_undefined();
}


JS_FUNCTION(Read) {
  JS_DECLARE_THIS_PTR(adc, adc);
  DJS_CHECK_ARG_IF_EXIST(0, function);

  jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(0, function);

  if (jerry_value_is_null(jcallback)) {
    return JS_CREATE_ERROR(TYPE, "Bad arguments - callback required");
  } else {
    ADC_ASYNC(read, adc, jcallback, kAdcOpRead);
  }

  return jerry_create_undefined();
}

JS_FUNCTION(ReadSync) {
  JS_DECLARE_THIS_PTR(adc, adc);

  int32_t value = iotjs_adc_read(adc);
  if (value < 0) {
    return JS_CREATE_ERROR(COMMON, "ADC Read Error");
  }

  return jerry_create_number(value);
}

JS_FUNCTION(Close) {
  JS_DECLARE_THIS_PTR(adc, adc);
  DJS_CHECK_ARG_IF_EXIST(0, function);

  jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(0, function);

  if (jerry_value_is_null(jcallback)) {
    jerry_value_t jdummycallback =
        iotjs_jval_create_function(&iotjs_jval_dummy_function);
    ADC_ASYNC(close, adc, jdummycallback, kAdcOpClose);
    jerry_release_value(jdummycallback);
  } else {
    ADC_ASYNC(close, adc, jcallback, kAdcOpClose);
  }

  return jerry_create_null();
}

JS_FUNCTION(CloseSync) {
  JS_DECLARE_THIS_PTR(adc, adc);

  bool ret = iotjs_adc_close(adc);
  iotjs_adc_destroy(adc);
  if (!ret) {
    return JS_CREATE_ERROR(COMMON, "ADC Close Error");
  }

  return jerry_create_null();
}

jerry_value_t InitAdc() {
  jerry_value_t jadc = jerry_create_object();
  jerry_value_t jadcConstructor =
      jerry_create_external_function(AdcConstructor);
  iotjs_jval_set_property_jval(jadc, IOTJS_MAGIC_STRING_ADC, jadcConstructor);

  jerry_value_t jprototype = jerry_create_object();
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_READ, Read);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_READSYNC, ReadSync);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_CLOSE, Close);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_CLOSESYNC, CloseSync);
  iotjs_jval_set_property_jval(jadcConstructor, IOTJS_MAGIC_STRING_PROTOTYPE,
                               jprototype);

  jerry_release_value(jprototype);
  jerry_release_value(jadcConstructor);

  return jadc;
}
