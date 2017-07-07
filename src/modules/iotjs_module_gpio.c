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
#include "iotjs_objectwrap.h"
#include <stdio.h>


static iotjs_gpio_t* iotjs_gpio_instance_from_jval(const iotjs_jval_t* jgpio);
IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(gpio);


static iotjs_gpio_t* iotjs_gpio_create(const iotjs_jval_t* jgpio) {
  iotjs_gpio_t* gpio = IOTJS_ALLOC(iotjs_gpio_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_gpio_t, gpio);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, jgpio,
                               &this_module_native_info);

  iotjs_gpio_platform_create(_this);
  return gpio;
}


static void iotjs_gpio_destroy(iotjs_gpio_t* gpio) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_gpio_t, gpio);
  iotjs_gpio_platform_destroy(_this);
  iotjs_jobjectwrap_destroy(&_this->jobjectwrap);
  IOTJS_RELEASE(gpio);
}


#define THIS iotjs_gpio_reqwrap_t* gpio_reqwrap


static iotjs_gpio_reqwrap_t* iotjs_gpio_reqwrap_create(
    const iotjs_jval_t* jcallback, iotjs_gpio_t* gpio, GpioOp op) {
  iotjs_gpio_reqwrap_t* gpio_reqwrap = IOTJS_ALLOC(iotjs_gpio_reqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_gpio_reqwrap_t, gpio_reqwrap);

  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);

  _this->req_data.op = op;
  _this->gpio_instance = gpio;
  return gpio_reqwrap;
}


static void iotjs_gpio_reqwrap_destroy(THIS) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_gpio_reqwrap_t, gpio_reqwrap);
  iotjs_reqwrap_destroy(&_this->reqwrap);
  IOTJS_RELEASE(gpio_reqwrap);
}


static void iotjs_gpio_reqwrap_dispatched(THIS) {
  IOTJS_VALIDATABLE_STRUCT_METHOD_VALIDATE(iotjs_gpio_reqwrap_t, gpio_reqwrap);
  iotjs_gpio_reqwrap_destroy(gpio_reqwrap);
}


static uv_work_t* iotjs_gpio_reqwrap_req(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_reqwrap_t, gpio_reqwrap);
  return &_this->req;
}


static const iotjs_jval_t* iotjs_gpio_reqwrap_jcallback(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_reqwrap_t, gpio_reqwrap);
  return iotjs_reqwrap_jcallback(&_this->reqwrap);
}


static iotjs_gpio_t* iotjs_gpio_instance_from_jval(const iotjs_jval_t* jgpio) {
  uintptr_t handle = iotjs_jval_get_object_native_handle(jgpio);
  return (iotjs_gpio_t*)handle;
}


iotjs_gpio_reqwrap_t* iotjs_gpio_reqwrap_from_request(uv_work_t* req) {
  return (iotjs_gpio_reqwrap_t*)(iotjs_reqwrap_from_request((uv_req_t*)req));
}


iotjs_gpio_reqdata_t* iotjs_gpio_reqwrap_data(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_reqwrap_t, gpio_reqwrap);
  return &_this->req_data;
}


iotjs_gpio_t* iotjs_gpio_instance_from_reqwrap(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_reqwrap_t, gpio_reqwrap);
  return _this->gpio_instance;
}


#undef THIS


static void iotjs_gpio_write_worker(uv_work_t* work_req) {
  GPIO_WORKER_INIT;

  if (!iotjs_gpio_write(gpio, req_data->value)) {
    req_data->result = false;
    return;
  }

  req_data->result = true;
}


static void iotjs_gpio_read_worker(uv_work_t* work_req) {
  GPIO_WORKER_INIT;

  int result = iotjs_gpio_read(gpio);
  if (result < 0) {
    req_data->result = false;
    return;
  }
  req_data->result = true;
  req_data->value = (bool)result;
}


static void iotjs_gpio_close_worker(uv_work_t* work_req) {
  GPIO_WORKER_INIT;

  if (!iotjs_gpio_close(gpio)) {
    req_data->result = false;
    return;
  }

  req_data->result = true;
}


static void iotjs_gpio_after_worker(uv_work_t* work_req, int status) {
  iotjs_gpio_reqwrap_t* req_wrap = iotjs_gpio_reqwrap_from_request(work_req);
  iotjs_gpio_reqdata_t* req_data = iotjs_gpio_reqwrap_data(req_wrap);
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
          iotjs_jargs_append_null(&jargs);
          iotjs_jargs_append_bool(&jargs, req_data->value);
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

  const iotjs_jval_t* jcallback = iotjs_gpio_reqwrap_jcallback(req_wrap);
  iotjs_make_callback(jcallback, iotjs_jval_get_undefined(), &jargs);

  iotjs_jargs_destroy(&jargs);

  iotjs_gpio_reqwrap_dispatched(req_wrap);
}


static void gpio_set_configurable(iotjs_gpio_t* gpio,
                                  const iotjs_jval_t* jconfigurable) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);

  iotjs_jval_t jpin =
      iotjs_jval_get_property(jconfigurable, IOTJS_MAGIC_STRING_PIN);
  _this->pin = iotjs_jval_as_number(&jpin);
  iotjs_jval_destroy(&jpin);

  iotjs_jval_t jdirection =
      iotjs_jval_get_property(jconfigurable, IOTJS_MAGIC_STRING_DIRECTION);
  _this->direction = (GpioDirection)iotjs_jval_as_number(&jdirection);
  iotjs_jval_destroy(&jdirection);

  iotjs_jval_t jmode =
      iotjs_jval_get_property(jconfigurable, IOTJS_MAGIC_STRING_MODE);
  _this->mode = (GpioMode)iotjs_jval_as_number(&jmode);
  iotjs_jval_destroy(&jmode);

  iotjs_jval_t jedge =
      iotjs_jval_get_property(jconfigurable, IOTJS_MAGIC_STRING_EDGE);
  _this->edge = (GpioMode)iotjs_jval_as_number(&jedge);
  iotjs_jval_destroy(&jedge);
}


#define GPIO_ASYNC(call, this, jcallback, op)                          \
  do {                                                                 \
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get()); \
    iotjs_gpio_reqwrap_t* req_wrap =                                   \
        iotjs_gpio_reqwrap_create(jcallback, this, op);                \
    uv_work_t* req = iotjs_gpio_reqwrap_req(req_wrap);                 \
    uv_queue_work(loop, req, iotjs_gpio_##call##_worker,               \
                  iotjs_gpio_after_worker);                            \
  } while (0)


#define GPIO_ASYNC_WITH_VALUE(call, this, jcallback, op, val)           \
  do {                                                                  \
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());  \
    iotjs_gpio_reqwrap_t* req_wrap =                                    \
        iotjs_gpio_reqwrap_create(jcallback, this, op);                 \
    uv_work_t* req = iotjs_gpio_reqwrap_req(req_wrap);                  \
    iotjs_gpio_reqdata_t* req_data = iotjs_gpio_reqwrap_data(req_wrap); \
    req_data->value = val;                                              \
    uv_queue_work(loop, req, iotjs_gpio_##call##_worker,                \
                  iotjs_gpio_after_worker);                             \
  } while (0)


JHANDLER_FUNCTION(GpioConstructor) {
  DJHANDLER_CHECK_THIS(object);
  DJHANDLER_CHECK_ARGS(2, object, function);

  // Create GPIO object
  const iotjs_jval_t* jgpio = JHANDLER_GET_THIS(object);
  iotjs_gpio_t* gpio = iotjs_gpio_create(jgpio);
  IOTJS_ASSERT(gpio == iotjs_gpio_instance_from_jval(jgpio));

  gpio_set_configurable(gpio, JHANDLER_GET_ARG(0, object));

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(1, function);
  GPIO_ASYNC(open, gpio, jcallback, kGpioOpOpen);
}


JHANDLER_FUNCTION(Write) {
  JHANDLER_DECLARE_THIS_PTR(gpio, gpio);
  DJHANDLER_CHECK_ARGS(1, boolean);
  DJHANDLER_CHECK_ARG_IF_EXIST(1, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(1, function);

  bool value = JHANDLER_GET_ARG(0, boolean);

  if (jcallback) {
    GPIO_ASYNC_WITH_VALUE(write, gpio, jcallback, kGpioOpWrite, value);
  } else {
    if (!iotjs_gpio_write(gpio, value)) {
      JHANDLER_THROW(COMMON, "GPIO WriteSync Error");
    }
  }

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Read) {
  JHANDLER_DECLARE_THIS_PTR(gpio, gpio);
  DJHANDLER_CHECK_ARGS(0);
  DJHANDLER_CHECK_ARG_IF_EXIST(0, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(0, function);

  if (jcallback) {
    GPIO_ASYNC(read, gpio, jcallback, kGpioOpRead);
    iotjs_jhandler_return_null(jhandler);
  } else {
    int value = iotjs_gpio_read(gpio);
    if (value < 0) {
      JHANDLER_THROW(COMMON, "GPIO ReadSync Error");
    }
    iotjs_jhandler_return_boolean(jhandler, value);
  }
}


JHANDLER_FUNCTION(Close) {
  JHANDLER_DECLARE_THIS_PTR(gpio, gpio)
  DJHANDLER_CHECK_ARG_IF_EXIST(0, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(0, function);

  if (jcallback) {
    GPIO_ASYNC(close, gpio, jcallback, kGpioOpClose);
  } else {
    if (!iotjs_gpio_close(gpio)) {
      JHANDLER_THROW(COMMON, "GPIO CloseSync Error");
    }
  }

  iotjs_jhandler_return_null(jhandler);
}


iotjs_jval_t InitGpio() {
  iotjs_jval_t jgpio = iotjs_jval_create_object();
  iotjs_jval_t jgpioConstructor =
      iotjs_jval_create_function_with_dispatch(GpioConstructor);
  iotjs_jval_set_property_jval(&jgpio, IOTJS_MAGIC_STRING_GPIO,
                               &jgpioConstructor);

  iotjs_jval_t jprototype = iotjs_jval_create_object();
  iotjs_jval_set_method(&jprototype, IOTJS_MAGIC_STRING_WRITE, Write);
  iotjs_jval_set_method(&jprototype, IOTJS_MAGIC_STRING_READ, Read);
  iotjs_jval_set_method(&jprototype, IOTJS_MAGIC_STRING_CLOSE, Close);
  iotjs_jval_set_property_jval(&jgpioConstructor, IOTJS_MAGIC_STRING_PROTOTYPE,
                               &jprototype);
  iotjs_jval_destroy(&jprototype);
  iotjs_jval_destroy(&jgpioConstructor);

  // GPIO direction properties
  iotjs_jval_t jdirection = iotjs_jval_create_object();
  iotjs_jval_set_property_number(&jdirection, IOTJS_MAGIC_STRING_IN,
                                 kGpioDirectionIn);
  iotjs_jval_set_property_number(&jdirection, IOTJS_MAGIC_STRING_OUT,
                                 kGpioDirectionOut);
  iotjs_jval_set_property_jval(&jgpio, IOTJS_MAGIC_STRING_DIRECTION_U,
                               &jdirection);
  iotjs_jval_destroy(&jdirection);


  // GPIO mode properties
  iotjs_jval_t jmode = iotjs_jval_create_object();
  iotjs_jval_set_property_number(&jmode, IOTJS_MAGIC_STRING_NONE,
                                 kGpioModeNone);
#if defined(__NUTTX__)
  iotjs_jval_set_property_number(&jmode, IOTJS_MAGIC_STRING_PULLUP,
                                 kGpioModePullup);
  iotjs_jval_set_property_number(&jmode, IOTJS_MAGIC_STRING_PULLDOWN,
                                 kGpioModePulldown);
  iotjs_jval_set_property_number(&jmode, IOTJS_MAGIC_STRING_FLOAT,
                                 kGpioModeFloat);
  iotjs_jval_set_property_number(&jmode, IOTJS_MAGIC_STRING_PUSHPULL,
                                 kGpioModePushpull);
  iotjs_jval_set_property_number(&jmode, IOTJS_MAGIC_STRING_OPENDRAIN,
                                 kGpioModeOpendrain);
#endif
  iotjs_jval_set_property_jval(&jgpio, IOTJS_MAGIC_STRING_MODE_U, &jmode);
  iotjs_jval_destroy(&jmode);

  // GPIO edge properties
  iotjs_jval_t jedge = iotjs_jval_create_object();
  iotjs_jval_set_property_number(&jedge, IOTJS_MAGIC_STRING_NONE,
                                 kGpioEdgeNone);
  iotjs_jval_set_property_number(&jedge, IOTJS_MAGIC_STRING_RISING_U,
                                 kGpioEdgeRising);
  iotjs_jval_set_property_number(&jedge, IOTJS_MAGIC_STRING_FALLING_U,
                                 kGpioEdgeFalling);
  iotjs_jval_set_property_number(&jedge, IOTJS_MAGIC_STRING_BOTH_U,
                                 kGpioEdgeBoth);
  iotjs_jval_set_property_jval(&jgpio, IOTJS_MAGIC_STRING_EDGE_U, &jedge);
  iotjs_jval_destroy(&jedge);

  return jgpio;
}
