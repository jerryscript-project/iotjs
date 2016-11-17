/* Copyright 2016 Samsung Electronics Co., Ltd.
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


#include "iotjs_def.h"
#include "module/iotjs_module_pwm.h"


int PwmInitializePwmPath(iotjs_pwm_reqdata_t* req_data) {
  IOTJS_ASSERT(!"Not implemented");
  return 0;
}


void ExportWorker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}


void SetPeriodWorker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}


void SetDutyCycleWorker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}


void SetEnableWorker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}


void UnexportWorker(uv_work_t* work_req) {
  IOTJS_ASSERT(!"Not implemented");
}


#endif // __NUTTX__
