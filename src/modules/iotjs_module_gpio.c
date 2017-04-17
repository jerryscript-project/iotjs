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


static void iotjs_gpio_destroy(iotjs_gpio_t* gpio);
static iotjs_gpio_t* iotjs_gpio_instance_from_jval(const iotjs_jval_t* jgpio);
IOTJS_DEFINE_NATIVE_HANDLE_INFO(gpio);


static iotjs_gpio_t* iotjs_gpio_create(const iotjs_jval_t* jgpio) {
  iotjs_gpio_t* gpio = IOTJS_ALLOC(iotjs_gpio_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_gpio_t, gpio);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, jgpio, &gpio_native_info);
  return gpio;
}


static void iotjs_gpio_destroy(iotjs_gpio_t* gpio) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_gpio_t, gpio);
  iotjs_jobjectwrap_destroy(&_this->jobjectwrap);
  IOTJS_RELEASE(gpio);
}


#define THIS iotjs_gpio_reqwrap_t* gpio_reqwrap


static iotjs_gpio_reqwrap_t* iotjs_gpio_reqwrap_create(
    const iotjs_jval_t* jcallback, const iotjs_jval_t* jgpio, GpioOp op) {
  iotjs_gpio_reqwrap_t* gpio_reqwrap = IOTJS_ALLOC(iotjs_gpio_reqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_gpio_reqwrap_t, gpio_reqwrap);

  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);

  _this->req_data.op = op;
  _this->gpio_instance = iotjs_gpio_instance_from_jval(jgpio);
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


void iotjs_gpio_after_worker(uv_work_t* work_req, int status) {
  iotjs_gpio_reqwrap_t* req_wrap = iotjs_gpio_reqwrap_from_request(work_req);
  iotjs_gpio_reqdata_t* req_data = iotjs_gpio_reqwrap_data(req_wrap);
  iotjs_jargs_t jargs = iotjs_jargs_create(2);

  if (status) {
    iotjs_jargs_append_error(&jargs, "GPIO System Error");
  } else {
    switch (req_data->op) {
      case kGpioOpOpen:
        if (req_data->result < 0) {
          iotjs_jargs_append_error(&jargs, "GPIO Open Error");
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      case kGpioOpWrite:
        if (req_data->result < 0) {
          iotjs_jargs_append_error(&jargs, "GPIO Write Error");
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      case kGpioOpRead:
        if (req_data->result < 0) {
          iotjs_jargs_append_error(&jargs, "GPIO Read Error");
        } else {
          iotjs_jargs_append_null(&jargs);
          iotjs_jargs_append_bool(&jargs, req_data->value);
        }
        break;
      case kGpioOpClose:
        if (req_data->result < 0) {
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
}


#define GPIO_ASYNC(op)                                                 \
  do {                                                                 \
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get()); \
    uv_work_t* req = iotjs_gpio_reqwrap_req(req_wrap);                 \
    uv_queue_work(loop, req, iotjs_gpio_##op##_worker,                 \
                  iotjs_gpio_after_worker);                            \
  } while (0)


// write(value, callback)
JHANDLER_FUNCTION(Write) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(2, boolean, function);

  bool value = JHANDLER_GET_ARG(0, boolean);
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(1, function);

  iotjs_gpio_reqwrap_t* req_wrap =
      iotjs_gpio_reqwrap_create(jcallback, JHANDLER_GET_THIS(object),
                                kGpioOpWrite);

  iotjs_gpio_reqdata_t* req_data = iotjs_gpio_reqwrap_data(req_wrap);
  req_data->value = value;

  GPIO_ASYNC(write);

  iotjs_jhandler_return_null(jhandler);
}


// writeSync(value)
JHANDLER_FUNCTION(WriteSync) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, boolean);

  const iotjs_jval_t* jgpio = JHANDLER_GET_THIS(object);
  iotjs_gpio_t* gpio = iotjs_gpio_instance_from_jval(jgpio);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);

  bool value = JHANDLER_GET_ARG(0, boolean);

  if (iotjs_gpio_write(_this->pin, value)) {
    iotjs_jhandler_return_null(jhandler);
  } else {
    iotjs_jval_t jerror = iotjs_jval_create_error("GPIO WriteSync Error");
    iotjs_jhandler_throw(jhandler, &jerror);
    iotjs_jval_destroy(&jerror);
  }
}


// read(callback)
JHANDLER_FUNCTION(Read) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(0, function);
  iotjs_gpio_reqwrap_t* req_wrap =
      iotjs_gpio_reqwrap_create(jcallback, JHANDLER_GET_THIS(object),
                                kGpioOpRead);

  GPIO_ASYNC(read);

  iotjs_jhandler_return_null(jhandler);
}


// readSync()
JHANDLER_FUNCTION(ReadSync) {
  JHANDLER_CHECK_THIS(object);

  const iotjs_jval_t* jgpio = JHANDLER_GET_THIS(object);
  iotjs_gpio_t* gpio = iotjs_gpio_instance_from_jval(jgpio);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);

  int value = iotjs_gpio_read(_this->pin);

  if (value >= 0) {
    iotjs_jhandler_return_boolean(jhandler, value);
  } else {
    iotjs_jval_t jerror = iotjs_jval_create_error("GPIO ReadSync Error");
    iotjs_jhandler_throw(jhandler, &jerror);
    iotjs_jval_destroy(&jerror);
  }
}


// close(callback)
JHANDLER_FUNCTION(Close) {
  JHANDLER_CHECK_ARGS(1, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(0, function);
  iotjs_gpio_reqwrap_t* req_wrap =
      iotjs_gpio_reqwrap_create(jcallback, JHANDLER_GET_THIS(object),
                                kGpioOpClose);

  GPIO_ASYNC(close);

  iotjs_jhandler_return_null(jhandler);
}


// closeSync()
JHANDLER_FUNCTION(CloseSync) {
  JHANDLER_CHECK_THIS(object);

  const iotjs_jval_t* jgpio = JHANDLER_GET_THIS(object);
  iotjs_gpio_t* gpio = iotjs_gpio_instance_from_jval(jgpio);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_gpio_t, gpio);

  if (iotjs_gpio_close(_this->pin)) {
    iotjs_jhandler_return_null(jhandler);
  } else {
    iotjs_jval_t jerror = iotjs_jval_create_error("GPIO CloseSync Error");
    iotjs_jhandler_throw(jhandler, &jerror);
    iotjs_jval_destroy(&jerror);
  }
}


// Constructor(configurable, callback)
JHANDLER_FUNCTION(GpioConstructor) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(2, object, function);

  // Create GPIO object
  const iotjs_jval_t* jgpio = JHANDLER_GET_THIS(object);
  iotjs_gpio_t* gpio = iotjs_gpio_create(jgpio);
  IOTJS_ASSERT(gpio == iotjs_gpio_instance_from_jval(jgpio));

  gpio_set_configurable(gpio, JHANDLER_GET_ARG(0, object));

  // Create reqwrap
  iotjs_gpio_reqwrap_t* req_wrap =
      iotjs_gpio_reqwrap_create(JHANDLER_GET_ARG(1, function), jgpio,
                                kGpioOpOpen);

  GPIO_ASYNC(open);
}


iotjs_jval_t InitGpio() {
  iotjs_jval_t jgpio = iotjs_jval_create_object();
  iotjs_jval_t jgpioConstructor =
      iotjs_jval_create_function_with_dispatch(GpioConstructor);
  iotjs_jval_set_property_jval(&jgpio, IOTJS_MAGIC_STRING_GPIO,
                               &jgpioConstructor);

  iotjs_jval_t jprototype = iotjs_jval_create_object();
  iotjs_jval_set_method(&jprototype, IOTJS_MAGIC_STRING_WRITE, Write);
  iotjs_jval_set_method(&jprototype, IOTJS_MAGIC_STRING_WRITESYNC, WriteSync);
  iotjs_jval_set_method(&jprototype, IOTJS_MAGIC_STRING_READ, Read);
  iotjs_jval_set_method(&jprototype, IOTJS_MAGIC_STRING_READSYNC, ReadSync);
  iotjs_jval_set_method(&jprototype, IOTJS_MAGIC_STRING_CLOSE, Close);
  iotjs_jval_set_method(&jprototype, IOTJS_MAGIC_STRING_CLOSESYNC, CloseSync);
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
  iotjs_jval_set_property_jval(&jgpio, IOTJS_MAGIC_STRING_MODE_U, &jmode);
  iotjs_jval_destroy(&jmode);

  return jgpio;
}
