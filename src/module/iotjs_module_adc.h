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
  kAdcOpExport,
  kAdcOpRead,
  kAdcOpUnexport,
} AdcOp;

typedef enum {
  kAdcErrOk = 0,
  kAdcErrExport = -1,
  kAdcErrUnexport = -2,
  kAdcErrRead = -3,
} AdcError;


typedef struct {
  int32_t pin;
  int32_t value;

  AdcError result;
  AdcOp op;
} iotjs_adc_reqdata_t;


typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_work_t req;
  iotjs_adc_reqdata_t req_data;
} IOTJS_VALIDATED_STRUCT(iotjs_adc_reqwrap_t);


#define THIS iotjs_adc_reqwrap_t* adc_reqwrap
iotjs_adc_reqwrap_t* iotjs_adc_reqwrap_create(const iotjs_jval_t* jcallback,
                                              AdcOp op);
void iotjs_adc_reqwrap_dispatched(THIS);
uv_work_t* iotjs_adc_reqwrap_req(THIS);
const iotjs_jval_t* iotjs_adc_reqwrap_jcallback(THIS);
iotjs_adc_reqwrap_t* iotjs_adc_reqwrap_from_request(uv_work_t* req);
iotjs_adc_reqdata_t* iotjs_adc_reqwrap_data(THIS);
#undef THIS


// This Adc class provides interfaces for ADC operation.
typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;
} IOTJS_VALIDATED_STRUCT(iotjs_adc_t);

iotjs_adc_t* iotjs_adc_create(const iotjs_jval_t* jadc);
const iotjs_jval_t* iotjs_adc_get_jadc();
iotjs_adc_t* iotjs_adc_get_instance();
void iotjs_adc_initialize();

void iotjs_adc_export_worker(uv_work_t* work_req);
void iotjs_adc_read_worker(uv_work_t* work_req);
int32_t iotjs_adc_read_sync(int32_t pin);
void iotjs_adc_unexport_worker(uv_work_t* work_req);


#endif /* IOTJS_MODULE_ADC_H */
