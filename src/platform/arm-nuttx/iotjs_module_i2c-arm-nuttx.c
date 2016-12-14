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


#include "module/iotjs_module_i2c.h"


void I2cSetAddress(uint8_t address) {
  IOTJS_ASSERT(!"Not implemented");
}


void ScanWorker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}


void OpenWorker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}


void I2cClose() {
  IOTJS_ASSERT(!"Not implemented");
}


void WriteWorker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}


void WriteByteWorker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}


void WriteBlockWorker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}


void ReadWorker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}


void ReadByteWorker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}


void ReadBlockWorker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}


#endif // __NUTTX__
