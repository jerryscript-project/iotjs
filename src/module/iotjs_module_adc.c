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

#include "iotjs_def.h"
#include "iotjs_module_adc.h"
#include "iotjs_objectwrap.h"


#define THIS iotjs_adc_reqwrap_t* adc_reqwrap

iotjs_adc_reqwrap_t* iotjs_adc_reqwrap_create(const iotjs_jval_t* jcallback,
                                              AdcOp op) {
  iotjs_adc_reqwrap_t* adc_reqwrap = IOTJS_ALLOC(iotjs_adc_reqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_adc_reqwrap_t, adc_reqwrap);

  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);

  _this->req_data.op = op;

  return adc_reqwrap;
}


static void iotjs_adc_reqwrap_destroy(THIS) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_adc_reqwrap_t, adc_reqwrap);
  iotjs_reqwrap_destroy(&_this->reqwrap);
  IOTJS_RELEASE(adc_reqwrap);
}


void iotjs_adc_reqwrap_dispatched(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_reqwrap_t, adc_reqwrap);
  iotjs_adc_reqwrap_destroy(adc_reqwrap);
}


uv_work_t* iotjs_adc_reqwrap_req(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_reqwrap_t, adc_reqwrap);
  return &_this->req;
}


const iotjs_jval_t* iotjs_adc_reqwrap_jcallback(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_reqwrap_t, adc_reqwrap);
  return iotjs_reqwrap_jcallback(&_this->reqwrap);
}


iotjs_adc_reqwrap_t* iotjs_adc_reqwrap_from_request(uv_work_t* req) {
  return (iotjs_adc_reqwrap_t*)(iotjs_reqwrap_from_request((uv_req_t*)req));
}


iotjs_adc_reqdata_t* iotjs_adc_reqwrap_data(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_adc_reqwrap_t, adc_reqwrap);
  return &_this->req_data;
}

#undef THIS


static void iotjs_adc_destroy(iotjs_adc_t* adc);


iotjs_adc_t* iotjs_adc_create(const iotjs_jval_t* jadc) {
  iotjs_adc_t* adc = IOTJS_ALLOC(iotjs_adc_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_adc_t, adc);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, jadc,
                               (JFreeHandlerType)iotjs_adc_destroy);
  return adc;
}


static void iotjs_adc_destroy(iotjs_adc_t* adc) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_adc_t, adc);
  iotjs_jobjectwrap_destroy(&_this->jobjectwrap);
  IOTJS_RELEASE(adc);
}


const iotjs_jval_t* iotjs_adc_get_jadc() {
  return iotjs_module_get(MODULE_ADC);
}


iotjs_adc_t* iotjs_adc_get_instance() {
  const iotjs_jval_t* jadc = iotjs_adc_get_jadc();
  iotjs_jobjectwrap_t* jobjectwrap = iotjs_jobjectwrap_from_jobject(jadc);
  return (iotjs_adc_t*)jobjectwrap;
}


static void iotjs_adc_append_error(iotjs_jargs_t* jargs, const char* msg) {
  iotjs_jval_t error = iotjs_jval_create_error(msg);
  iotjs_jargs_append_jval(jargs, &error);
  iotjs_jval_destroy(&error);
}


void iotjs_adc_after_work(uv_work_t* work_req, int status) {
  iotjs_adc_t* adc = iotjs_adc_get_instance();

  iotjs_adc_reqwrap_t* req_wrap = iotjs_adc_reqwrap_from_request(work_req);
  iotjs_adc_reqdata_t* req_data = iotjs_adc_reqwrap_data(req_wrap);

  iotjs_jargs_t jargs = iotjs_jargs_create(2);
  AdcError result = req_data->result;

  if (status) {
    iotjs_jval_t error = iotjs_jval_create_error("System error");
    iotjs_jargs_append_jval(&jargs, &error);
    iotjs_jval_destroy(&error);
  } else {
    switch (req_data->op) {
      case kAdcOpExport:
        if (result == kAdcErrExport) {
          iotjs_adc_append_error(&jargs, "Failed to export ADC device");
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      case kAdcOpRead:
        if (result == kAdcErrRead) {
          iotjs_adc_append_error(&jargs, "Cannot read from ADC device");
        } else {
          iotjs_jargs_append_null(&jargs);
          iotjs_jargs_append_number(&jargs, req_data->value);
        }
        break;
      case kAdcOpUnexport:
        if (result == kAdcErrUnexport) {
          iotjs_adc_append_error(&jargs, "Cannot unexport ADC device");
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      default: {
        IOTJS_ASSERT(!"Unreachable");
        break;
      }
    }
  }

  const iotjs_jval_t* jcallback = iotjs_adc_reqwrap_jcallback(req_wrap);
  const iotjs_jval_t* jadc = iotjs_adc_get_jadc();
  iotjs_make_callback(jcallback, jadc, &jargs);

  iotjs_jargs_destroy(&jargs);

  iotjs_adc_reqwrap_dispatched(req_wrap);
}


#define ADC_ASYNC(op)                                                        \
  do {                                                                       \
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());       \
    uv_work_t* req = iotjs_adc_reqwrap_req(req_wrap);                        \
    uv_queue_work(loop, req, iotjs_adc_##op##_worker, iotjs_adc_after_work); \
  } while (0)


JHANDLER_FUNCTION(Export) {
  JHANDLER_CHECK_ARGS(2, number, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(1, function);

  iotjs_adc_reqwrap_t* req_wrap =
      iotjs_adc_reqwrap_create(jcallback, kAdcOpExport);

  iotjs_adc_reqdata_t* req_data = iotjs_adc_reqwrap_data(req_wrap);
  req_data->pin = JHANDLER_GET_ARG(0, number);

  ADC_ASYNC(export);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Read) {
  JHANDLER_CHECK_ARGS(2, number, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(1, function);

  iotjs_adc_reqwrap_t* req_wrap =
      iotjs_adc_reqwrap_create(jcallback, kAdcOpRead);

  iotjs_adc_reqdata_t* req_data = iotjs_adc_reqwrap_data(req_wrap);
  req_data->pin = JHANDLER_GET_ARG(0, number);

  ADC_ASYNC(read);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(ReadSync) {
  JHANDLER_CHECK_ARGS(1, number);

  int32_t pin = JHANDLER_GET_ARG(0, number);
  int32_t value = iotjs_adc_read_sync(pin);

  iotjs_jhandler_return_number(jhandler, value);
}

JHANDLER_FUNCTION(Unexport) {
  JHANDLER_CHECK_ARGS(2, number, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(1, function);

  iotjs_adc_reqwrap_t* req_wrap =
      iotjs_adc_reqwrap_create(jcallback, kAdcOpUnexport);

  iotjs_adc_reqdata_t* req_data = iotjs_adc_reqwrap_data(req_wrap);
  req_data->pin = JHANDLER_GET_ARG(0, number);

  ADC_ASYNC(unexport);

  iotjs_jhandler_return_null(jhandler);
}


iotjs_jval_t InitAdc() {
  iotjs_jval_t jadc = iotjs_jval_create_object();

  iotjs_jval_set_method(&jadc, "export", Export);
  iotjs_jval_set_method(&jadc, "read", Read);
  iotjs_jval_set_method(&jadc, "readSync", ReadSync);
  iotjs_jval_set_method(&jadc, "unexport", Unexport);


#define SET_ADC_CONSTANT(object, constant)                       \
  do {                                                           \
    iotjs_jval_set_property_number(object, #constant, constant); \
  } while (0)

  SET_ADC_CONSTANT(&jadc, kAdcErrOk);
  SET_ADC_CONSTANT(&jadc, kAdcErrExport);
  SET_ADC_CONSTANT(&jadc, kAdcErrUnexport);
  SET_ADC_CONSTANT(&jadc, kAdcErrRead);

#undef SET_ADC_CONSTANT

  iotjs_adc_t* adc = iotjs_adc_create(&jadc);
  IOTJS_ASSERT(adc ==
               (iotjs_adc_t*)(iotjs_jval_get_object_native_handle(&jadc)));

  return jadc;
}
