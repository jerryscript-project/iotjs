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

#include "module/iotjs_module_adc.h"
#include "iotjs_module_unimplemented.inl.h"

void iotjs_adc_export_worker(uv_work_t* work_req) IOTJS_MODULE_UNIMPLEMENTED();
void iotjs_adc_read_worker(uv_work_t* work_req) IOTJS_MODULE_UNIMPLEMENTED();
int32_t iotjs_adc_read_sync(int32_t pin) IOTJS_MODULE_UNIMPLEMENTED(0);
void iotjs_adc_unexport_worker(uv_work_t* work_req)
    IOTJS_MODULE_UNIMPLEMENTED();

#endif // __APPLE__
