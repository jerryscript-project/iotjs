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


#ifndef IOTJS_MODULE_ADC_H
#define IOTJS_MODULE_ADC_H

#include "iotjs_def.h"
#include "iotjs_objectwrap.h"
#include "iotjs_reqwrap.h"


typedef enum {
  kAdcOpOpen,
  kAdcOpRead,
  kAdcOpClose,
} AdcOp;


typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;

#if defined(__linux__)
  iotjs_string_t device;
#elif defined(__NUTTX__) || defined(__TIZENRT__)
  uint32_t pin;
#endif
  int32_t device_fd;
} IOTJS_VALIDATED_STRUCT(iotjs_adc_t);


typedef struct {
  int32_t value;

  bool result;
  AdcOp op;
} iotjs_adc_reqdata_t;


typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_work_t req;
  iotjs_adc_reqdata_t req_data;
  iotjs_adc_t* adc_instance;
} IOTJS_VALIDATED_STRUCT(iotjs_adc_reqwrap_t);


#define THIS iotjs_adc_reqwrap_t* adc_reqwrap

iotjs_adc_reqwrap_t* iotjs_adc_reqwrap_from_request(uv_work_t* req);
iotjs_adc_reqdata_t* iotjs_adc_reqwrap_data(THIS);

iotjs_adc_t* iotjs_adc_instance_from_reqwrap(THIS);

#undef THIS


#define ADC_WORKER_INIT                                                     \
  iotjs_adc_reqwrap_t* req_wrap = iotjs_adc_reqwrap_from_request(work_req); \
  iotjs_adc_reqdata_t* req_data = iotjs_adc_reqwrap_data(req_wrap);         \
  iotjs_adc_t* adc = iotjs_adc_instance_from_reqwrap(req_wrap);


int32_t iotjs_adc_read(iotjs_adc_t* adc);
bool iotjs_adc_close(iotjs_adc_t* adc);
void iotjs_adc_open_worker(uv_work_t* work_req);


#endif /* IOTJS_MODULE_ADC_H */
