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

#if defined(__NUTTX__)


#include "modules/iotjs_module_spi.h"


bool iotjs_spi_transfer(iotjs_spi_t* spi) {
  IOTJS_ASSERT(!"Not implemented");
  return false;
}


bool iotjs_spi_close(iotjs_spi_t* spi) {
  IOTJS_ASSERT(!"Not implemented");
  return false;
}

void iotjs_spi_open_worker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}


void iotjs_spi_transfer_worker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}


void iotjs_spi_close_worker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}


#endif // __NUTTX__
