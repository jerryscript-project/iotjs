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

#if !defined(__NUTTX__)
#error "Module __FILE__ is for nuttx only"
#endif

#include <nuttx/i2c/i2c_master.h>

#include "iotjs_systemio-nuttx.h"

#include "modules/iotjs_module_i2c.h"


#define I2C_DEFAULT_FREQUENCY 400000
#define I2C_DEFAULT_ADDRESS_LENGTH 7

struct iotjs_i2c_platform_data_s {
  int bus;
  struct i2c_master_s* i2c_master;
  struct i2c_config_s config;
};

void iotjs_i2c_create_platform_data(iotjs_i2c_t* i2c) {
  i2c->platform_data = IOTJS_ALLOC(iotjs_i2c_platform_data_t);

  i2c->platform_data->i2c_master = NULL;
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
  if (!platform_data->i2c_master) {                              \
    DLOG("%s: I2C is not opened", __func__);                     \
    return false;                                                \
  }

bool iotjs_i2c_open(iotjs_i2c_t* i2c) {
  iotjs_i2c_platform_data_t* platform_data = i2c->platform_data;
  IOTJS_ASSERT(platform_data);

  platform_data->config.address = i2c->address;
  platform_data->config.addrlen = I2C_DEFAULT_ADDRESS_LENGTH;

  platform_data->i2c_master = iotjs_i2c_config_nuttx(platform_data->bus);
  if (!platform_data->i2c_master) {
    DLOG("%s : cannot open", __func__);
    return false;
  }

  platform_data->config.frequency = I2C_DEFAULT_FREQUENCY;

  return true;
}

bool iotjs_i2c_close(iotjs_i2c_t* i2c) {
  I2C_METHOD_HEADER(i2c);

  if (iotjs_i2c_unconfig_nuttx(platform_data->i2c_master) < 0) {
    DLOG("%s : cannot close", __func__);
    return false;
  }

  return true;
}

bool iotjs_i2c_write(iotjs_i2c_t* i2c) {
  I2C_METHOD_HEADER(i2c);

  uint8_t len = i2c->buf_len;
  uint8_t* data = (uint8_t*)i2c->buf_data;
  IOTJS_ASSERT(len > 0);

  int ret =
      i2c_write(platform_data->i2c_master, &platform_data->config, data, len);

  IOTJS_RELEASE(i2c->buf_data);

  if (ret < 0) {
    DLOG("%s : cannot write - %d", __func__, ret);
    return false;
  }
  return true;
}

bool iotjs_i2c_read(iotjs_i2c_t* i2c) {
  I2C_METHOD_HEADER(i2c);

  uint8_t len = i2c->buf_len;
  i2c->buf_data = iotjs_buffer_allocate(len);
  IOTJS_ASSERT(len > 0);

  int ret = i2c_read(platform_data->i2c_master, &platform_data->config,
                     (uint8_t*)i2c->buf_data, len);
  if (ret != 0) {
    DLOG("%s : cannot read - %d", __func__, ret);
    return false;
  }

  return true;
}
