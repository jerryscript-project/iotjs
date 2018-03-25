/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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

#if !defined(__TIZENRT__)
#error "Module __FILE__ is for TizenRT only"
#endif

#include <tinyara/config.h>

#if !defined(CONFIG_I2C)
#error "\n\nTizenRT CONFIG_I2C configuration flag required for I2C module\n\n"
#endif

#include <iotbus_i2c.h>

#include "modules/iotjs_module_i2c.h"


struct iotjs_i2c_platform_data_s {
  int bus;
  iotbus_i2c_context_h i2c_context;
};


void iotjs_i2c_create_platform_data(iotjs_i2c_t* i2c) {
  i2c->platform_data = IOTJS_ALLOC(iotjs_i2c_platform_data_t);

  i2c->platform_data->i2c_context = NULL;
}


void iotjs_i2c_destroy_platform_data(iotjs_i2c_platform_data_t* platform_data) {
  IOTJS_ASSERT(platform_data);
  IOTJS_RELEASE(platform_data);
}


jerry_value_t iotjs_i2c_set_platform_config(iotjs_i2c_t* i2c,
                                            const jerry_value_t jconfig) {
  iotjs_i2c_platform_data_t* platform_data = i2c->platform_data;

  JS_GET_REQUIRED_CONF_VALUE(jconfig, platform_data->bus,
                             IOTJS_MAGIC_STRING_BUS, number);

  return jerry_create_undefined();
}


#define I2C_METHOD_HEADER(arg)                                   \
  iotjs_i2c_platform_data_t* platform_data = arg->platform_data; \
  IOTJS_ASSERT(platform_data);                                   \
  if (!platform_data->i2c_context) {                             \
    DLOG("%s: I2C is not opened", __func__);                     \
    return false;                                                \
  }


bool iotjs_i2c_open(iotjs_i2c_t* i2c) {
  iotjs_i2c_platform_data_t* platform_data = i2c->platform_data;
  IOTJS_ASSERT(platform_data);

  // Init i2c context
  platform_data->i2c_context = iotbus_i2c_init(platform_data->bus);
  if (!platform_data->i2c_context) {
    DLOG("%s: cannot open I2C", __func__);
    return false;
  }

  // Set i2c frequency
  int ret =
      iotbus_i2c_set_frequency(platform_data->i2c_context, IOTBUS_I2C_STD);
  if (ret < 0) {
    DLOG("%s: cannot set frequency", __func__);
    return false;
  }

  if (iotbus_i2c_set_address(platform_data->i2c_context, i2c->address) < 0) {
    DLOG("%s: cannot set address", __func__);
    return false;
  }

  return true;
}


bool iotjs_i2c_close(iotjs_i2c_t* i2c) {
  I2C_METHOD_HEADER(i2c);

  if (iotbus_i2c_stop(platform_data->i2c_context) < 0) {
    DLOG("%s: cannot close I2C", __func__);
    return false;
  }

  return true;
}


bool iotjs_i2c_write(iotjs_i2c_t* i2c) {
  I2C_METHOD_HEADER(i2c);

  uint8_t len = i2c->buf_len;
  IOTJS_ASSERT(len > 0);
  uint8_t* data = (uint8_t*)i2c->buf_data;

  int ret = iotbus_i2c_write(platform_data->i2c_context, data, len);

  IOTJS_RELEASE(i2c->buf_data);

  if (ret < 0) {
    DLOG("%s: cannot write data", __func__);
    return false;
  }
  return true;
}


bool iotjs_i2c_read(iotjs_i2c_t* i2c) {
  I2C_METHOD_HEADER(i2c);

  uint8_t len = i2c->buf_len;
  i2c->buf_data = iotjs_buffer_allocate(len);

  IOTJS_ASSERT(len > 0);

  if (iotbus_i2c_read(platform_data->i2c_context, (uint8_t*)i2c->buf_data,
                      len) < 0) {
    DLOG("%s: cannot read data", __func__);
    return false;
  }

  return true;
}
