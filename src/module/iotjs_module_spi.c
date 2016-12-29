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
#include "iotjs_module_spi.h"
#include "iotjs_module_buffer.h"
#include "iotjs_objectwrap.h"
#include <unistd.h>


/*
 * SPI instance function
 */
static void iotjs_spi_destroy(iotjs_spi_t* spi);


iotjs_spi_t* iotjs_spi_create(const iotjs_jval_t* jspi) {
  iotjs_spi_t* spi = IOTJS_ALLOC(iotjs_spi_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_spi_t, spi);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, jspi,
                               (JFreeHandlerType)iotjs_spi_destroy);

  _this->device_fd = -1;

  return spi;
}


static void iotjs_spi_destroy(iotjs_spi_t* spi) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_spi_t, spi);
  iotjs_jobjectwrap_destroy(&_this->jobjectwrap);
  IOTJS_RELEASE(spi);
}


iotjs_spi_t* iotjs_spi_get_instance(const iotjs_jval_t* jspi) {
  uintptr_t handle = iotjs_jval_get_object_native_handle(jspi);
  return (iotjs_spi_t*)(handle);
}


/*
 * SPI worker reqwrap function
 */
#define THIS iotjs_spi_reqwrap_t* spi_reqwrap

iotjs_spi_reqwrap_t* iotjs_spi_reqwrap_create(const iotjs_jval_t* jcallback,
                                              SpiOp op) {
  iotjs_spi_reqwrap_t* spi_reqwrap = IOTJS_ALLOC(iotjs_spi_reqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_spi_reqwrap_t, spi_reqwrap);

  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);

  _this->req_data.op = op;
  _this->req_data.selected_option = -1;

  return spi_reqwrap;
}


static void iotjs_spi_reqwrap_destroy(THIS) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_spi_reqwrap_t, spi_reqwrap);
  iotjs_reqwrap_destroy(&_this->reqwrap);
  IOTJS_RELEASE(spi_reqwrap);
}


void iotjs_spi_reqwrap_dispatched(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_reqwrap_t, spi_reqwrap);
  iotjs_spi_reqwrap_destroy(spi_reqwrap);
}


uv_work_t* iotjs_spi_reqwrap_req(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_reqwrap_t, spi_reqwrap);
  return &_this->req;
}


const iotjs_jval_t* iotjs_spi_reqwrap_jcallback(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_reqwrap_t, spi_reqwrap);
  return iotjs_reqwrap_jcallback(&_this->reqwrap);
}


iotjs_spi_reqwrap_t* iotjs_spi_reqwrap_from_request(uv_work_t* req) {
  return (iotjs_spi_reqwrap_t*)(iotjs_reqwrap_from_request((uv_req_t*)req));
}


iotjs_spi_reqdata_t* iotjs_spi_reqwrap_data(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_reqwrap_t, spi_reqwrap);
  return &_this->req_data;
}

#undef THIS


void iotjs_spi_set_device_fd(iotjs_spi_t* spi, int32_t fd) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);
  _this->device_fd = fd;
}


int32_t iotjs_spi_get_device_fd(iotjs_spi_t* spi) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);
  return _this->device_fd;
}


static int iotjs_spi_get_array_data(char** buf, const iotjs_jval_t* jarray) {
  iotjs_jval_t jlength = iotjs_jval_get_property(jarray, "length");
  IOTJS_ASSERT(!iotjs_jval_is_undefined(&jlength));

  int length = iotjs_jval_as_number(&jlength);
  *buf = iotjs_buffer_allocate(length);

  for (int i = 0; i < length; i++) {
    iotjs_jval_t jdata = iotjs_jval_get_property_by_index(jarray, i);
    (*buf)[i] = iotjs_jval_as_number(&jdata);
    iotjs_jval_destroy(&jdata);
  }

  iotjs_jval_destroy(&jlength);

  return length;
}


static void iotjs_spi_set_array_buffer(iotjs_spi_t* spi,
                                       const iotjs_jval_t* jtx_buf,
                                       const iotjs_jval_t* jrx_buf) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);

  int tx_buf_len = iotjs_spi_get_array_data(&_this->tx_buf_data, jtx_buf);
  int rx_buf_len = iotjs_spi_get_array_data(&_this->rx_buf_data, jrx_buf);

  IOTJS_ASSERT(_this->tx_buf_data != NULL && _this->rx_buf_data != NULL);
  IOTJS_ASSERT(tx_buf_len > 0 && rx_buf_len > 0 && tx_buf_len == rx_buf_len);

  _this->buf_len = tx_buf_len;
}


static void iotjs_spi_set_buffer(iotjs_spi_t* spi, const iotjs_jval_t* jtx_buf,
                                 const iotjs_jval_t* jrx_buf) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);

  iotjs_bufferwrap_t* tx_buf = iotjs_bufferwrap_from_jbuffer(jtx_buf);
  iotjs_bufferwrap_t* rx_buf = iotjs_bufferwrap_from_jbuffer(jrx_buf);

  _this->tx_buf_data = iotjs_bufferwrap_buffer(tx_buf);
  uint8_t tx_buf_len = iotjs_bufferwrap_length(tx_buf);
  _this->rx_buf_data = iotjs_bufferwrap_buffer(rx_buf);
  uint8_t rx_buf_len = iotjs_bufferwrap_length(rx_buf);

  IOTJS_ASSERT(_this->tx_buf_data != NULL && _this->rx_buf_data != NULL);
  IOTJS_ASSERT(tx_buf_len > 0 && rx_buf_len > 0 && tx_buf_len == rx_buf_len);

  _this->buf_len = tx_buf_len;
}


static void iotjs_spi_release_buffer(iotjs_spi_t* spi) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);

  iotjs_buffer_release(_this->tx_buf_data);
  iotjs_buffer_release(_this->rx_buf_data);
}


static uint32_t iotjs_spi_check_options(iotjs_spi_t* spi,
                                        const iotjs_jval_t* joptions) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);
  uint32_t selected_option = 0;

#define SPI_CHECK_OPTIONS(option_name, optionName, OptionName, set_value, \
                          type)                                           \
  do {                                                                    \
    iotjs_jval_t j##option_name =                                         \
        iotjs_jval_get_property(joptions, #optionName);                   \
    if (!iotjs_jval_is_undefined(&j##option_name)) {                      \
      if (iotjs_jval_is_number(&j##option_name)) {                        \
        set_value = (type)iotjs_jval_as_number(&j##option_name);          \
      } else if (iotjs_jval_is_boolean(&j##option_name)) {                \
        set_value = iotjs_jval_as_boolean(&j##option_name);               \
      } else {                                                            \
        return -1;                                                        \
      }                                                                   \
      selected_option |= kSpiOption##OptionName;                          \
    }                                                                     \
    iotjs_jval_destroy(&j##option_name);                                  \
  } while (0);

  SPI_CHECK_OPTIONS(mode, mode, Mode, _this->mode, SpiMode);
  SPI_CHECK_OPTIONS(chip_select, chipSelect, ChipSelect, _this->chip_select,
                    SpiChipSelect);
  SPI_CHECK_OPTIONS(max_speed, maxSpeed, MaxSpeed, _this->max_speed, uint32_t);
  SPI_CHECK_OPTIONS(bits_per_word, bitsPerWord, BitsPerWord,
                    _this->bits_per_word, uint8_t);
  SPI_CHECK_OPTIONS(bit_order, bitOrder, BitOrder, _this->bit_order, uint8_t);
  SPI_CHECK_OPTIONS(loopback, loopback, Loopback, _this->loopback, bool);

#undef SPI_CHECK_OPTIONS

  return selected_option;
}


static void iotjs_spi_append_error(iotjs_jargs_t* jargs, const char* msg) {
  iotjs_jval_t error = iotjs_jval_create_error(msg);
  iotjs_jargs_append_jval(jargs, &error);
  iotjs_jval_destroy(&error);
}


/*
 * SPI worker callback function
 */
void iotjs_spi_after_work(uv_work_t* work_req, int status) {
  iotjs_spi_reqwrap_t* req_wrap = iotjs_spi_reqwrap_from_request(work_req);
  iotjs_spi_reqdata_t* req_data = iotjs_spi_reqwrap_data(req_wrap);
  iotjs_spi_t* spi = req_data->spi_instance;

  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);

  iotjs_jargs_t jargs = iotjs_jargs_create(2);
  SpiError result = req_data->result;

  if (status) {
    iotjs_spi_append_error(&jargs, "System error");
  } else {
    switch (req_data->op) {
      case kSpiOpExport:
        if (result == kSpiErrExport) {
          iotjs_spi_append_error(&jargs, "Failed to export SPI device");
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      case kSpiOpTransfer:
        if (result == kSpiErrTransfer) {
          iotjs_spi_append_error(&jargs, "Cannot transfer from SPI device");
        } else {
          iotjs_jargs_append_null(&jargs);

          // Append read data
          iotjs_jval_t result =
              iotjs_jval_create_byte_array(_this->buf_len, _this->rx_buf_data);
          iotjs_jargs_append_jval(&jargs, &result);
          iotjs_jval_destroy(&result);
        }
        iotjs_spi_release_buffer(spi);
        break;
      case kSpiOpUnexport:
        if (result == kSpiErrUnexport) {
          iotjs_spi_append_error(&jargs, "Failed to unexport SPI device");
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

  const iotjs_jval_t* jcallback = iotjs_spi_reqwrap_jcallback(req_wrap);
  iotjs_make_callback(jcallback, iotjs_jval_get_undefined(), &jargs);

  iotjs_jargs_destroy(&jargs);

  iotjs_spi_reqwrap_dispatched(req_wrap);
}


#define SPI_ASYNC(op)                                                        \
  do {                                                                       \
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());       \
    uv_work_t* req = iotjs_spi_reqwrap_req(req_wrap);                        \
    uv_queue_work(loop, req, iotjs_spi_##op##_worker, iotjs_spi_after_work); \
  } while (0)


JHANDLER_FUNCTION(SpiCons) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(4, number, number, object, function);

  // Create SPI object
  const iotjs_jval_t* jspi = JHANDLER_GET_THIS(object);
  iotjs_spi_t* spi = iotjs_spi_create(jspi);
  IOTJS_ASSERT(spi == iotjs_spi_get_instance(jspi));

  // Create reqwrap
  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(3, function);
  iotjs_spi_reqwrap_t* req_wrap =
      iotjs_spi_reqwrap_create(jcallback, kSpiOpExport);

  iotjs_spi_reqdata_t* req_data = iotjs_spi_reqwrap_data(req_wrap);
  int32_t device_number = JHANDLER_GET_ARG(0, number);
  int32_t cs_number = JHANDLER_GET_ARG(1, number);

  req_data->pin = SPI_DEVICE_NUMBER(device_number) | SPI_CS_NUMBER(cs_number);
  req_data->spi_instance = spi;

  // Check and set options
  const iotjs_jval_t* joptions = JHANDLER_GET_ARG(2, object);
  req_data->selected_option = iotjs_spi_check_options(spi, joptions);

  SPI_ASYNC(export);
}


// FIXME: do not need transferArray if array buffer is implemented.
JHANDLER_FUNCTION(TransferArray) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(3, array, array, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(2, function);
  iotjs_spi_reqwrap_t* req_wrap =
      iotjs_spi_reqwrap_create(jcallback, kSpiOpTransfer);

  iotjs_spi_reqdata_t* req_data = iotjs_spi_reqwrap_data(req_wrap);
  const iotjs_jval_t* jspi = JHANDLER_GET_THIS(object);
  iotjs_spi_t* spi = iotjs_spi_get_instance(jspi);
  req_data->spi_instance = spi;

  iotjs_spi_set_array_buffer(spi, JHANDLER_GET_ARG(0, array),
                             JHANDLER_GET_ARG(1, array));

  SPI_ASYNC(transfer);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(TransferBuffer) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(3, object, object, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(2, function);
  iotjs_spi_reqwrap_t* req_wrap =
      iotjs_spi_reqwrap_create(jcallback, kSpiOpTransfer);

  iotjs_spi_reqdata_t* req_data = iotjs_spi_reqwrap_data(req_wrap);
  const iotjs_jval_t* jspi = JHANDLER_GET_THIS(object);
  iotjs_spi_t* spi = iotjs_spi_get_instance(jspi);
  req_data->spi_instance = spi;

  iotjs_spi_set_buffer(spi, JHANDLER_GET_ARG(0, object),
                       JHANDLER_GET_ARG(1, object));

  SPI_ASYNC(transfer);

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(SetOption) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, object);

  const iotjs_jval_t* jspi = JHANDLER_GET_THIS(object);
  iotjs_spi_t* spi = iotjs_spi_get_instance(jspi);
  const iotjs_jval_t* joptions = JHANDLER_GET_ARG(0, object);
  uint32_t selected_option = iotjs_spi_check_options(spi, joptions);

  if (!iotjs_spi_set_options(spi, selected_option)) {
    iotjs_jval_t eobj = iotjs_jval_create_error("Cannot set options");
    iotjs_jhandler_return_jval(jhandler, &eobj);
    iotjs_jval_destroy(&eobj);
    printf("hs\n");
    return;
  }
  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Unexport) {
  JHANDLER_CHECK_THIS(object);
  JHANDLER_CHECK_ARGS(1, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(0, function);
  iotjs_spi_reqwrap_t* req_wrap =
      iotjs_spi_reqwrap_create(jcallback, kSpiOpUnexport);

  iotjs_spi_reqdata_t* req_data = iotjs_spi_reqwrap_data(req_wrap);
  const iotjs_jval_t* jspi = JHANDLER_GET_THIS(object);
  iotjs_spi_t* spi = iotjs_spi_get_instance(jspi);
  req_data->spi_instance = spi;

  SPI_ASYNC(unexport);

  iotjs_jhandler_return_null(jhandler);
}


iotjs_jval_t InitSpi() {
  iotjs_jval_t jSpiCons = iotjs_jval_create_function(SpiCons);

  iotjs_jval_t prototype = iotjs_jval_create_object();

  iotjs_jval_set_method(&prototype, "transferArray", TransferArray);
  iotjs_jval_set_method(&prototype, "transferBuffer", TransferBuffer);
  iotjs_jval_set_method(&prototype, "setOption", SetOption);
  iotjs_jval_set_method(&prototype, "unexport", Unexport);

  iotjs_jval_set_property_jval(&jSpiCons, "prototype", &prototype);

  iotjs_jval_destroy(&prototype);

  return jSpiCons;
}
