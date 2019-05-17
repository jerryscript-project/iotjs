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
#include "iotjs_uv_request.h"


IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(adc);

IOTJS_DEFINE_PERIPH_CREATE_FUNCTION(adc);

static void iotjs_adc_destroy(iotjs_adc_t* adc) {
  iotjs_adc_destroy_platform_data(adc->platform_data);
  IOTJS_RELEASE(adc);
}

static void adc_worker(uv_work_t* work_req) {
  iotjs_periph_data_t* worker_data =
      (iotjs_periph_data_t*)IOTJS_UV_REQUEST_EXTRA_DATA(work_req);
  iotjs_adc_t* adc = (iotjs_adc_t*)worker_data->data;

  switch (worker_data->op) {
    case kAdcOpOpen:
      worker_data->result = iotjs_adc_open(adc);
      break;
    case kAdcOpRead:
      worker_data->result = iotjs_adc_read(adc);
      break;
    case kAdcOpClose:
      worker_data->result = iotjs_adc_close(adc);
      break;
    default:
      IOTJS_ASSERT(!"Invalid Adc Operation");
  }
}

JS_FUNCTION(adc_constructor) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, object);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  // Create ADC object
  const jerry_value_t jadc = JS_GET_THIS();
  iotjs_adc_t* adc = adc_create(jadc);

  void* adc_native = NULL;
  IOTJS_ASSERT(jerry_get_object_native_pointer(jadc, &adc_native,
                                               &this_module_native_info) &&
               adc == adc_native);

  jerry_value_t jconfig;
  JS_GET_REQUIRED_ARG_VALUE(0, jconfig, IOTJS_MAGIC_STRING_CONFIG, object);

  jerry_value_t config_res = iotjs_adc_set_platform_config(adc, jconfig);
  if (jerry_value_is_error(config_res)) {
    return config_res;
  }
  IOTJS_ASSERT(jerry_value_is_undefined(config_res));

  jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  // If the callback doesn't exist, it is completed synchronously.
  // Otherwise, it will be executed asynchronously.
  if (!jerry_value_is_null(jcallback)) {
    iotjs_periph_call_async(adc, jcallback, kAdcOpOpen, adc_worker);
  } else if (!iotjs_adc_open(adc)) {
    return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kAdcOpOpen));
  }

  return jerry_create_undefined();
}


JS_FUNCTION(adc_read) {
  JS_DECLARE_THIS_PTR(adc, adc);
  DJS_CHECK_ARG_IF_EXIST(0, function);

  iotjs_periph_call_async(adc, JS_GET_ARG_IF_EXIST(0, function), kAdcOpRead,
                          adc_worker);

  return jerry_create_undefined();
}

JS_FUNCTION(adc_read_sync) {
  JS_DECLARE_THIS_PTR(adc, adc);

  if (!iotjs_adc_read(adc)) {
    return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kAdcOpRead));
  }

  return jerry_create_number(adc->value);
}

JS_FUNCTION(adc_close) {
  JS_DECLARE_THIS_PTR(adc, adc);
  DJS_CHECK_ARG_IF_EXIST(0, function);

  iotjs_periph_call_async(adc, JS_GET_ARG_IF_EXIST(0, function), kAdcOpClose,
                          adc_worker);

  return jerry_create_undefined();
}

JS_FUNCTION(adc_close_sync) {
  JS_DECLARE_THIS_PTR(adc, adc);

  bool ret = iotjs_adc_close(adc);
  if (!ret) {
    return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kAdcOpClose));
  }

  return jerry_create_undefined();
}

jerry_value_t iotjs_init_adc(void) {
  jerry_value_t jadc_cons = jerry_create_external_function(adc_constructor);
  jerry_value_t jprototype = jerry_create_object();

  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_READ, adc_read);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_READSYNC, adc_read_sync);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_CLOSE, adc_close);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_CLOSESYNC,
                        adc_close_sync);

  iotjs_jval_set_property_jval(jadc_cons, IOTJS_MAGIC_STRING_PROTOTYPE,
                               jprototype);

  jerry_release_value(jprototype);

  return jadc_cons;
}
