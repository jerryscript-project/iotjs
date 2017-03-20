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

#include "module/iotjs_module_i2c.h"
#include "iotjs_module_unimplemented.inl.h"

void I2cSetAddress(iotjs_i2c_t* i2c, uint8_t address)
    IOTJS_MODULE_UNIMPLEMENTED();
void OpenWorker(uv_work_t* work_req) IOTJS_MODULE_UNIMPLEMENTED();
void I2cClose(iotjs_i2c_t* i2c) IOTJS_MODULE_UNIMPLEMENTED();
void WriteWorker(uv_work_t* work_req) IOTJS_MODULE_UNIMPLEMENTED();
void WriteByteWorker(uv_work_t* work_req) IOTJS_MODULE_UNIMPLEMENTED();
void WriteBlockWorker(uv_work_t* work_req) IOTJS_MODULE_UNIMPLEMENTED();
void ReadWorker(uv_work_t* work_req) IOTJS_MODULE_UNIMPLEMENTED();
void ReadByteWorker(uv_work_t* work_req) IOTJS_MODULE_UNIMPLEMENTED();
void ReadBlockWorker(uv_work_t* work_req) IOTJS_MODULE_UNIMPLEMENTED();

#endif // __APPLE__
