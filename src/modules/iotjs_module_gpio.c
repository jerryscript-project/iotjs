/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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

#include <string.h>

#include "iotjs_def.h"
#include "iotjs_module_gpio.h"
#include <stdio.h>


IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(gpio);

static iotjs_gpio_t* gpio_create(jerry_value_t jgpio) {
  iotjs_gpio_t* gpio = IOTJS_ALLOC(iotjs_gpio_t);
  iotjs_gpio_create_platform_data(gpio);
  gpio->jobject = jgpio;
  jerry_set_object_native_pointer(jgpio, gpio, &this_module_native_info);

  return gpio;
}


static iotjs_gpio_reqwrap_t* gpio_reqwrap_create(jerry_value_t jcallback,
                                                 iotjs_gpio_t* gpio,
                                                 GpioOp op) {
  iotjs_gpio_reqwrap_t* gpio_reqwrap = IOTJS_ALLOC(iotjs_gpio_reqwrap_t);

  iotjs_reqwrap_initialize(&gpio_reqwrap->reqwrap, jcallback,
                           (uv_req_t*)&gpio_reqwrap->req);

  gpio_reqwrap->req_data.op = op;
  gpio_reqwrap->gpio_data = gpio;
  return gpio_reqwrap;
}


static void gpio_reqwrap_destroy(iotjs_gpio_reqwrap_t* gpio_reqwrap) {
  iotjs_reqwrap_destroy(&gpio_reqwrap->reqwrap);
  IOTJS_RELEASE(gpio_reqwrap);
}


static void gpio_reqwrap_dispatched(iotjs_gpio_reqwrap_t* gpio_reqwrap) {
  gpio_reqwrap_destroy(gpio_reqwrap);
}


static uv_work_t* gpio_reqwrap_req(iotjs_gpio_reqwrap_t* gpio_reqwrap) {
  return &gpio_reqwrap->req;
}


static jerry_value_t gpio_reqwrap_jcallback(
    iotjs_gpio_reqwrap_t* gpio_reqwrap) {
  return iotjs_reqwrap_jcallback(&gpio_reqwrap->reqwrap);
}


static iotjs_gpio_reqwrap_t* gpio_reqwrap_from_request(uv_work_t* req) {
  return (iotjs_gpio_reqwrap_t*)(iotjs_reqwrap_from_request((uv_req_t*)req));
}


static iotjs_gpio_reqdata_t* gpio_reqwrap_data(
    iotjs_gpio_reqwrap_t* gpio_reqwrap) {
  return &gpio_reqwrap->req_data;
}


static iotjs_gpio_t* gpio_instance_from_reqwrap(
    iotjs_gpio_reqwrap_t* gpio_reqwrap) {
  return gpio_reqwrap->gpio_data;
}


static void iotjs_gpio_destroy(iotjs_gpio_t* gpio) {
  iotjs_gpio_destroy_platform_data(gpio->platform_data);
  IOTJS_RELEASE(gpio);
}


static iotjs_gpio_t* gpio_instance_from_jval(const jerry_value_t jgpio) {
  uintptr_t handle = iotjs_jval_get_object_native_handle(jgpio);
  return (iotjs_gpio_t*)handle;
}


static void gpio_worker(uv_work_t* work_req) {
  iotjs_gpio_reqwrap_t* req_wrap = gpio_reqwrap_from_request(work_req);
  iotjs_gpio_reqdata_t* req_data = gpio_reqwrap_data(req_wrap);
  iotjs_gpio_t* gpio = gpio_instance_from_reqwrap(req_wrap);

  switch (req_data->op) {
    case kGpioOpOpen:
      req_data->result = iotjs_gpio_open(gpio);
      break;
    case kGpioOpWrite:
      req_data->result = iotjs_gpio_write(gpio);
      break;
    case kGpioOpRead:
      req_data->result = iotjs_gpio_read(gpio);
      break;
    case kGpioOpClose:
      req_data->result = iotjs_gpio_close(gpio);
      break;
    default:
      IOTJS_ASSERT(!"Invalid Gpio Operation");
  }
}


static void gpio_after_worker(uv_work_t* work_req, int status) {
  iotjs_gpio_reqwrap_t* req_wrap = gpio_reqwrap_from_request(work_req);
  iotjs_gpio_reqdata_t* req_data = gpio_reqwrap_data(req_wrap);

  iotjs_jargs_t jargs = iotjs_jargs_create(2);
  bool result = req_data->result;

  if (status) {
    iotjs_jargs_append_error(&jargs, "GPIO System Error");
  } else {
    switch (req_data->op) {
      case kGpioOpOpen:
        if (!result) {
          iotjs_jargs_append_error(&jargs, "GPIO Open Error");
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      case kGpioOpWrite:
        if (!result) {
          iotjs_jargs_append_error(&jargs, "GPIO Write Error");
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      case kGpioOpRead:
        if (!result) {
          iotjs_jargs_append_error(&jargs, "GPIO Read Error");
        } else {
          iotjs_gpio_t* gpio = gpio_instance_from_reqwrap(req_wrap);

          iotjs_jargs_append_null(&jargs);
          iotjs_jargs_append_bool(&jargs, gpio->value);
        }
        break;
      case kGpioOpClose:
        if (!result) {
          iotjs_jargs_append_error(&jargs, "GPIO Close Error");
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      default:
        IOTJS_ASSERT(!"Unreachable");
        break;
    }
  }

  const jerry_value_t jcallback = gpio_reqwrap_jcallback(req_wrap);
  if (jerry_value_is_function(jcallback)) {
    iotjs_make_callback(jcallback, jerry_create_undefined(), &jargs);
  }

  iotjs_jargs_destroy(&jargs);
  gpio_reqwrap_dispatched(req_wrap);
}


static void gpio_set_configurable(iotjs_gpio_t* gpio,
                                  jerry_value_t jconfigurable) {
  jerry_value_t jpin =
      iotjs_jval_get_property(jconfigurable, IOTJS_MAGIC_STRING_PIN);
  gpio->pin = iotjs_jval_as_number(jpin);
  jerry_release_value(jpin);

  // Direction
  jerry_value_t jdirection =
      iotjs_jval_get_property(jconfigurable, IOTJS_MAGIC_STRING_DIRECTION);

  if (!jerry_value_is_undefined(jdirection)) {
    gpio->direction = (GpioDirection)iotjs_jval_as_number(jdirection);
  } else {
    gpio->direction = kGpioDirectionOut;
  }
  jerry_release_value(jdirection);

  // Mode
  jerry_value_t jmode =
      iotjs_jval_get_property(jconfigurable, IOTJS_MAGIC_STRING_MODE);

  if (!jerry_value_is_undefined(jmode)) {
    gpio->mode = (GpioMode)iotjs_jval_as_number(jmode);
  } else {
    gpio->mode = kGpioModeNone;
  }
  jerry_release_value(jmode);

  // Edge
  jerry_value_t jedge =
      iotjs_jval_get_property(jconfigurable, IOTJS_MAGIC_STRING_EDGE);

  if (!jerry_value_is_undefined(jedge)) {
    gpio->edge = (GpioEdge)iotjs_jval_as_number(jedge);
  } else {
    gpio->edge = kGpioEdgeNone;
  }
  jerry_release_value(jedge);
}


#define GPIO_CALL_ASYNC(op, jcallback)                                         \
  do {                                                                         \
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());         \
    iotjs_gpio_reqwrap_t* req_wrap = gpio_reqwrap_create(jcallback, gpio, op); \
    uv_work_t* req = gpio_reqwrap_req(req_wrap);                               \
    uv_queue_work(loop, req, gpio_worker, gpio_after_worker);                  \
  } while (0)


JS_FUNCTION(GpioCons) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, object);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  // Create GPIO object
  const jerry_value_t jgpio = JS_GET_THIS();
  iotjs_gpio_t* gpio = gpio_create(jgpio);
  IOTJS_ASSERT(gpio == gpio_instance_from_jval(jgpio));

  gpio_set_configurable(gpio, JS_GET_ARG(0, object));

  const jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  // If the callback doesn't exist, it is completed synchronously.
  // Otherwise, it will be executed asynchronously.
  if (!jerry_value_is_null(jcallback)) {
    GPIO_CALL_ASYNC(kGpioOpOpen, jcallback);
  } else if (!iotjs_gpio_open(gpio)) {
    return JS_CREATE_ERROR(COMMON, "GPIO Error: cannot open GPIO");
  }

  return jerry_create_undefined();
}


JS_FUNCTION(Close) {
  JS_DECLARE_THIS_PTR(gpio, gpio);
  DJS_CHECK_ARG_IF_EXIST(0, function);

  GPIO_CALL_ASYNC(kGpioOpClose, JS_GET_ARG_IF_EXIST(0, function));

  return jerry_create_undefined();
}


JS_FUNCTION(CloseSync) {
  JS_DECLARE_THIS_PTR(gpio, gpio);

  if (!iotjs_gpio_close(gpio)) {
    return JS_CREATE_ERROR(COMMON, "GPIO CloseSync Error");
  }

  return jerry_create_undefined();
}


JS_FUNCTION(Write) {
  JS_DECLARE_THIS_PTR(gpio, gpio);

  bool value;
  if (jerry_value_is_number(jargv[0])) {
    value = (bool)jerry_get_number_value(jargv[0]);
  } else if (jerry_value_is_boolean(jargv[0])) {
    value = jerry_get_boolean_value(jargv[0]);
  } else {
    return JS_CREATE_ERROR(COMMON, "GPIO Write Error - Wrong argument type");
  }

  DJS_CHECK_ARG_IF_EXIST(1, function);

  gpio->value = value;

  GPIO_CALL_ASYNC(kGpioOpWrite, JS_GET_ARG_IF_EXIST(1, function));

  return jerry_create_undefined();
}


JS_FUNCTION(WriteSync) {
  JS_DECLARE_THIS_PTR(gpio, gpio);

  bool value;
  if (jerry_value_is_number(jargv[0])) {
    value = (bool)jerry_get_number_value(jargv[0]);
  } else if (jerry_value_is_boolean(jargv[0])) {
    value = jerry_get_boolean_value(jargv[0]);
  } else {
    return JS_CREATE_ERROR(COMMON,
                           "GPIO WriteSync Error - Wrong argument type");
  }

  gpio->value = value;

  if (!iotjs_gpio_write(gpio)) {
    return JS_CREATE_ERROR(COMMON, "GPIO WriteSync Error");
  }

  return jerry_create_undefined();
}


JS_FUNCTION(Read) {
  JS_DECLARE_THIS_PTR(gpio, gpio);
  DJS_CHECK_ARG_IF_EXIST(0, function);

  GPIO_CALL_ASYNC(kGpioOpRead, JS_GET_ARG_IF_EXIST(0, function));

  return jerry_create_undefined();
}


JS_FUNCTION(ReadSync) {
  JS_DECLARE_THIS_PTR(gpio, gpio);

  if (!iotjs_gpio_read(gpio)) {
    return JS_CREATE_ERROR(COMMON, "GPIO ReadSync Error");
  }

  return jerry_create_boolean(gpio->value);
}


jerry_value_t InitGpio() {
  jerry_value_t jgpioConstructor = jerry_create_external_function(GpioCons);

  jerry_value_t jprototype = jerry_create_object();

  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_CLOSE, Close);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_CLOSESYNC, CloseSync);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_WRITE, Write);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_WRITESYNC, WriteSync);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_READ, Read);
  iotjs_jval_set_method(jprototype, IOTJS_MAGIC_STRING_READSYNC, ReadSync);

  iotjs_jval_set_property_jval(jgpioConstructor, IOTJS_MAGIC_STRING_PROTOTYPE,
                               jprototype);
  jerry_release_value(jprototype);

  // GPIO direction properties
  jerry_value_t jdirection = jerry_create_object();
  iotjs_jval_set_property_number(jdirection, IOTJS_MAGIC_STRING_IN,
                                 kGpioDirectionIn);
  iotjs_jval_set_property_number(jdirection, IOTJS_MAGIC_STRING_OUT_U,
                                 kGpioDirectionOut);
  iotjs_jval_set_property_jval(jgpioConstructor, IOTJS_MAGIC_STRING_DIRECTION_U,
                               jdirection);
  jerry_release_value(jdirection);


  // GPIO mode properties
  jerry_value_t jmode = jerry_create_object();
  iotjs_jval_set_property_number(jmode, IOTJS_MAGIC_STRING_NONE_U,
                                 kGpioModeNone);
#if defined(__NUTTX__)
  iotjs_jval_set_property_number(jmode, IOTJS_MAGIC_STRING_PULLUP_U,
                                 kGpioModePullup);
  iotjs_jval_set_property_number(jmode, IOTJS_MAGIC_STRING_PULLDOWN_U,
                                 kGpioModePulldown);
  iotjs_jval_set_property_number(jmode, IOTJS_MAGIC_STRING_FLOAT_U,
                                 kGpioModeFloat);
  iotjs_jval_set_property_number(jmode, IOTJS_MAGIC_STRING_PUSHPULL_U,
                                 kGpioModePushpull);
  iotjs_jval_set_property_number(jmode, IOTJS_MAGIC_STRING_OPENDRAIN_U,
                                 kGpioModeOpendrain);
#endif
  iotjs_jval_set_property_jval(jgpioConstructor, IOTJS_MAGIC_STRING_MODE_U,
                               jmode);
  jerry_release_value(jmode);

  // GPIO edge properties
  jerry_value_t jedge = jerry_create_object();
  iotjs_jval_set_property_number(jedge, IOTJS_MAGIC_STRING_NONE_U,
                                 kGpioEdgeNone);
  iotjs_jval_set_property_number(jedge, IOTJS_MAGIC_STRING_RISING_U,
                                 kGpioEdgeRising);
  iotjs_jval_set_property_number(jedge, IOTJS_MAGIC_STRING_FALLING_U,
                                 kGpioEdgeFalling);
  iotjs_jval_set_property_number(jedge, IOTJS_MAGIC_STRING_BOTH_U,
                                 kGpioEdgeBoth);
  iotjs_jval_set_property_jval(jgpioConstructor, IOTJS_MAGIC_STRING_EDGE_U,
                               jedge);
  jerry_release_value(jedge);

  return jgpioConstructor;
}
