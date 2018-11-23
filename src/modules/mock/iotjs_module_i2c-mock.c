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

#include "modules/iotjs_module_i2c.h"

struct iotjs_i2c_platform_data_s {
  int bus;
};


void iotjs_i2c_create_platform_data(iotjs_i2c_t* i2c) {
  i2c->platform_data = IOTJS_ALLOC(iotjs_i2c_platform_data_t);
}


void iotjs_i2c_destroy_platform_data(iotjs_i2c_platform_data_t* platform_data) {
  IOTJS_ASSERT(platform_data);
  IOTJS_RELEASE(platform_data);
}

jerry_value_t iotjs_i2c_set_platform_config(iotjs_i2c_t* i2c,
                                            const jerry_value_t jconfig) {
  return jerry_create_undefined();
}

bool iotjs_i2c_open(iotjs_i2c_t* i2c) {
  return true;
}

bool iotjs_i2c_close(iotjs_i2c_t* i2c) {
  return true;
}

bool iotjs_i2c_write(iotjs_i2c_t* i2c) {
  IOTJS_RELEASE(i2c->buf_data);
  return true;
}

bool iotjs_i2c_read(iotjs_i2c_t* i2c) {
  i2c->buf_data = iotjs_buffer_allocate(i2c->buf_len);
  return true;
}
