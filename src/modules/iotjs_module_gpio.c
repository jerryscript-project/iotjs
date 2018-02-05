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

#include "iotjs_def.h"
#include "iotjs_module_gpio.h"


IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(gpio);

IOTJS_DEFINE_PERIPH_CREATE_FUNCTION(gpio);

static void iotjs_gpio_destroy(iotjs_gpio_t* gpio) {
  iotjs_gpio_destroy_platform_data(gpio->platform_data);
  IOTJS_RELEASE(gpio);
}

static void gpio_worker(uv_work_t* work_req) {
  iotjs_periph_reqwrap_t* req_wrap =
      (iotjs_periph_reqwrap_t*)(iotjs_reqwrap_from_request(
          (uv_req_t*)work_req));
  iotjs_gpio_t* gpio = (iotjs_gpio_t*)req_wrap->data;

  switch (req_wrap->op) {
    case kGpioOpOpen:
      req_wrap->result = iotjs_gpio_open(gpio);
      break;
    case kGpioOpWrite:
      req_wrap->result = iotjs_gpio_write(gpio);
      break;
    case kGpioOpRead:
      req_wrap->result = iotjs_gpio_read(gpio);
      break;
    case kGpioOpClose:
      req_wrap->result = iotjs_gpio_close(gpio);
      break;
    default:
      IOTJS_ASSERT(!"Invalid Operation");
  }
}

static jerry_value_t gpio_set_configuration(iotjs_gpio_t* gpio,
                                            jerry_value_t jconfigurable) {
  jerry_value_t jpin =
      iotjs_jval_get_property(jconfigurable, IOTJS_MAGIC_STRING_PIN);
  gpio->pin = iotjs_jval_as_number(jpin);
  jerry_release_value(jpin);

  // Direction
  jerry_value_t jdirection =
      iotjs_jval_get_property(jconfigurable, IOTJS_MAGIC_STRING_DIRECTION);

  if (jerry_value_is_undefined(jdirection)) {
    gpio->direction = kGpioDirectionOut;
  } else {
    if (jerry_value_is_number(jdirection)) {
      gpio->direction = (GpioDirection)iotjs_jval_as_number(jdirection);
    } else {
      gpio->direction = __kGpioDirectionMax;
    }
    if (gpio->direction >= __kGpioDirectionMax) {
      return JS_CREATE_ERROR(
          TYPE, "Bad arguments - gpio.direction should be DIRECTION.IN or OUT");
    }
  }
  jerry_release_value(jdirection);

  // Mode
  jerry_value_t jmode =
      iotjs_jval_get_property(jconfigurable, IOTJS_MAGIC_STRING_MODE);

  if (jerry_value_is_undefined(jmode)) {
    gpio->mode = kGpioModeNone;
  } else {
    if (jerry_value_is_number(jmode)) {
      gpio->mode = (GpioMode)iotjs_jval_as_number(jmode);
    } else {
      gpio->mode = __kGpioModeMax;
    }
    if (gpio->mode >= __kGpioModeMax) {
      return JS_CREATE_ERROR(TYPE,
                             "Bad arguments - gpio.mode should be MODE.NONE, "
                             "PULLUP, PULLDOWN, FLOAT, PUSHPULL or OPENDRAIN");

    } else if (gpio->direction == kGpioDirectionIn &&
               gpio->mode != kGpioModeNone && gpio->mode != kGpioModePullup &&
               gpio->mode != kGpioModePulldown) {
      return JS_CREATE_ERROR(TYPE,
                             "Bad arguments - DIRECTION.IN only supports "
                             "MODE.NONE, PULLUP and PULLDOWN");

    } else if (gpio->direction == kGpioDirectionOut &&
               gpio->mode != kGpioModeNone && gpio->mode != kGpioModeFloat &&
               gpio->mode != kGpioModePushpull &&
               gpio->mode != kGpioModeOpendrain) {
      return JS_CREATE_ERROR(TYPE,
                             "Bad arguments - DIRECTION.OUT only supports "
                             "MODE.NONE, FLOAT, PUSHPULL and OPENDRAIN");
    }
  }
  jerry_release_value(jmode);

  // Edge
  jerry_value_t jedge =
      iotjs_jval_get_property(jconfigurable, IOTJS_MAGIC_STRING_EDGE);

  if (jerry_value_is_undefined(jedge)) {
    gpio->edge = kGpioEdgeNone;
  } else {
    if (jerry_value_is_number(jedge)) {
      gpio->edge = (GpioEdge)iotjs_jval_as_number(jedge);
    } else {
      gpio->edge = __kGpioEdgeMax;
    }
    if (gpio->edge >= __kGpioEdgeMax) {
      return JS_CREATE_ERROR(TYPE,
                             "Bad arguments - gpio.edge should be EDGE.NONE, "
                             "RISING, FALLING or BOTH");
    }
  }
  jerry_release_value(jedge);

  return jerry_create_undefined();
}

JS_FUNCTION(GpioCons) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, object);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  // Create GPIO object
  const jerry_value_t jgpio = JS_GET_THIS();
  iotjs_gpio_t* gpio = gpio_create(jgpio);

  jerry_value_t config_res =
      gpio_set_configuration(gpio, JS_GET_ARG(0, object));
  if (jerry_value_has_error_flag(config_res)) {
    return config_res;
  }
  IOTJS_ASSERT(jerry_value_is_undefined(config_res));

  const jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  // If the callback doesn't exist, it is completed synchronously.
  // Otherwise, it will be executed asynchronously.
  if (!jerry_value_is_null(jcallback)) {
    iotjs_periph_call_async(gpio, jcallback, kGpioOpOpen, gpio_worker);
  } else if (!iotjs_gpio_open(gpio)) {
    return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kGpioOpOpen));
  }

  return jerry_create_undefined();
}

JS_FUNCTION(Close) {
  JS_DECLARE_THIS_PTR(gpio, gpio);
  DJS_CHECK_ARG_IF_EXIST(0, function);

  iotjs_periph_call_async(gpio, JS_GET_ARG_IF_EXIST(0, function), kGpioOpClose,
                          gpio_worker);

  return jerry_create_undefined();
}

JS_FUNCTION(CloseSync) {
  JS_DECLARE_THIS_PTR(gpio, gpio);

  if (!iotjs_gpio_close(gpio)) {
    return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kGpioOpClose));
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
    return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kGpioOpWrite));
  }

  DJS_CHECK_ARG_IF_EXIST(1, function);

  gpio->value = value;

  iotjs_periph_call_async(gpio, JS_GET_ARG_IF_EXIST(1, function), kGpioOpWrite,
                          gpio_worker);

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
    return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kGpioOpWrite));
  }

  return jerry_create_undefined();
}

JS_FUNCTION(Read) {
  JS_DECLARE_THIS_PTR(gpio, gpio);
  DJS_CHECK_ARG_IF_EXIST(0, function);

  iotjs_periph_call_async(gpio, JS_GET_ARG_IF_EXIST(0, function), kGpioOpRead,
                          gpio_worker);

  return jerry_create_undefined();
}

JS_FUNCTION(ReadSync) {
  JS_DECLARE_THIS_PTR(gpio, gpio);

  if (!iotjs_gpio_read(gpio)) {
    return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kGpioOpRead));
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
