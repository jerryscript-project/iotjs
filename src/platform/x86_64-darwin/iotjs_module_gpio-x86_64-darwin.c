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


#if defined(__APPLE__)

#include "module/iotjs_module_gpio.h"
#include "iotjs_module_unimplemented.inl.h"

void iotjs_gpio_open_worker(uv_work_t* work_req) IOTJS_MODULE_UNIMPLEMENTED();
void iotjs_gpio_write_worker(uv_work_t* work_req) IOTJS_MODULE_UNIMPLEMENTED();
void iotjs_gpio_read_worker(uv_work_t* work_req) IOTJS_MODULE_UNIMPLEMENTED();
void iotjs_gpio_close_worker(uv_work_t* work_req) IOTJS_MODULE_UNIMPLEMENTED();
bool iotjs_gpio_write(int32_t pin, bool value)
    IOTJS_MODULE_UNIMPLEMENTED(false);
int iotjs_gpio_read(int32_t pin) IOTJS_MODULE_UNIMPLEMENTED(0);
bool iotjs_gpio_close(int32_t pin) IOTJS_MODULE_UNIMPLEMENTED(false);

#endif // __APPLE__
