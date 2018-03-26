/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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

#if !defined(__TIZEN__)
#error "Module __FILE__ is for Tizen only"
#endif

#include <peripheral_io.h>

#include "modules/iotjs_module_i2c.h"


struct iotjs_i2c_platform_data_s {
  int bus;
  peripheral_i2c_h i2c_h;
};

void iotjs_i2c_create_platform_data(iotjs_i2c_t* i2c) {
  i2c->platform_data = IOTJS_ALLOC(iotjs_i2c_platform_data_t);

  i2c->platform_data->i2c_h = NULL;
}

void iotjs_i2c_destroy_platform_data(iotjs_i2c_platform_data_t* pdata) {
  IOTJS_RELEASE(pdata);
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
  if (!platform_data->i2c_h) {                                   \
    DLOG("%s: I2C is not opened", __func__);                     \
    return false;                                                \
  }

bool iotjs_i2c_open(iotjs_i2c_t* i2c) {
  iotjs_i2c_platform_data_t* platform_data = i2c->platform_data;
  IOTJS_ASSERT(platform_data);

  int ret = peripheral_i2c_open(platform_data->bus, i2c->address,
                                &platform_data->i2c_h);
  if (ret != PERIPHERAL_ERROR_NONE) {
    DLOG("%s : cannot open(%d)", __func__, ret);
    return false;
  }

  return true;
}

bool iotjs_i2c_close(iotjs_i2c_t* i2c) {
  I2C_METHOD_HEADER(i2c);

  int ret = peripheral_i2c_close(platform_data->i2c_h);
  if (ret != PERIPHERAL_ERROR_NONE) {
    DLOG("%s : cannot close(%d)", __func__, ret);
    return false;
  }

  return true;
}

bool iotjs_i2c_write(iotjs_i2c_t* i2c) {
  I2C_METHOD_HEADER(i2c);

  IOTJS_ASSERT(i2c->buf_len > 0);

  int ret = peripheral_i2c_write(platform_data->i2c_h, (uint8_t*)i2c->buf_data,
                                 i2c->buf_len);

  IOTJS_RELEASE(i2c->buf_data);

  if (ret != PERIPHERAL_ERROR_NONE) {
    DLOG("%s : cannot write(%d)", __func__, ret);
    return false;
  }
  return true;
}

bool iotjs_i2c_read(iotjs_i2c_t* i2c) {
  I2C_METHOD_HEADER(i2c);

  uint8_t len = i2c->buf_len;
  i2c->buf_data = iotjs_buffer_allocate(len);
  IOTJS_ASSERT(len > 0);

  int ret =
      peripheral_i2c_read(platform_data->i2c_h, (uint8_t*)i2c->buf_data, len);
  if (ret != PERIPHERAL_ERROR_NONE) {
    DLOG("%s : cannot read(%d)", __func__, ret);
    return false;
  }

  return true;
}
