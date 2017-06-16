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


IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(spi);

static iotjs_spi_t* iotjs_spi_create(const iotjs_jval_t* jspi) {
  iotjs_spi_t* spi = IOTJS_ALLOC(iotjs_spi_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_spi_t, spi);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, jspi,
                               &this_module_native_info);

#if defined(__linux__)
  _this->device = iotjs_string_create("");
#endif

  return spi;
}


static void iotjs_spi_destroy(iotjs_spi_t* spi) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_spi_t, spi);
  iotjs_jobjectwrap_destroy(&_this->jobjectwrap);

#if defined(__linux__)
  iotjs_string_destroy(&_this->device);
#endif

  IOTJS_RELEASE(spi);
}


#define THIS iotjs_spi_reqwrap_t* spi_reqwrap


static iotjs_spi_reqwrap_t* iotjs_spi_reqwrap_create(
    const iotjs_jval_t* jcallback, iotjs_spi_t* spi, SpiOp op) {
  iotjs_spi_reqwrap_t* spi_reqwrap = IOTJS_ALLOC(iotjs_spi_reqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_spi_reqwrap_t, spi_reqwrap);

  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);

  _this->req_data.op = op;
  _this->spi_instance = spi;

  return spi_reqwrap;
}


static void iotjs_spi_reqwrap_destroy(THIS) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_spi_reqwrap_t, spi_reqwrap);
  iotjs_reqwrap_destroy(&_this->reqwrap);
  IOTJS_RELEASE(spi_reqwrap);
}


static void iotjs_spi_reqwrap_dispatched(THIS) {
  IOTJS_VALIDATABLE_STRUCT_METHOD_VALIDATE(iotjs_spi_reqwrap_t, spi_reqwrap);
  iotjs_spi_reqwrap_destroy(spi_reqwrap);
}


static uv_work_t* iotjs_spi_reqwrap_req(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_reqwrap_t, spi_reqwrap);
  return &_this->req;
}


static const iotjs_jval_t* iotjs_spi_reqwrap_jcallback(THIS) {
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


iotjs_spi_t* iotjs_spi_instance_from_reqwrap(THIS) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_reqwrap_t, spi_reqwrap);
  return _this->spi_instance;
}


#undef THIS


static int iotjs_spi_get_array_data(char** buf, const iotjs_jval_t* jarray) {
  iotjs_jval_t jlength =
      iotjs_jval_get_property(jarray, IOTJS_MAGIC_STRING_LENGTH);
  IOTJS_ASSERT(!iotjs_jval_is_undefined(&jlength));

  size_t length = iotjs_jval_as_number(&jlength);
  IOTJS_ASSERT((int)length >= 0);
  *buf = iotjs_buffer_allocate(length);

  for (size_t i = 0; i < length; i++) {
    iotjs_jval_t jdata = iotjs_jval_get_property_by_index(jarray, i);
    (*buf)[i] = iotjs_jval_as_number(&jdata);
    iotjs_jval_destroy(&jdata);
  }

  iotjs_jval_destroy(&jlength);

  return (int)length;
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


static void iotjs_spi_set_configuration(iotjs_spi_t* spi,
                                        const iotjs_jval_t* joptions) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);

#if defined(__linux__)
  iotjs_jval_t jdevice =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_DEVICE);
  _this->device = iotjs_jval_as_string(&jdevice);
  iotjs_jval_destroy(&jdevice);
#elif defined(__NUTTX__) || defined(__TIZENRT__)
  iotjs_jval_t jbus = iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_BUS);
  _this->bus = iotjs_jval_as_number(&jbus);
  iotjs_jval_destroy(&jbus);
#endif
  iotjs_jval_t jmode =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_MODE);
  _this->mode = (SpiMode)iotjs_jval_as_number(&jmode);
  iotjs_jval_destroy(&jmode);

  iotjs_jval_t jchip_select =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_CHIPSELECT);
  _this->chip_select = (SpiChipSelect)iotjs_jval_as_number(&jchip_select);
  iotjs_jval_destroy(&jchip_select);

  iotjs_jval_t jmax_speed =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_MAXSPEED);
  _this->max_speed = iotjs_jval_as_number(&jmax_speed);
  iotjs_jval_destroy(&jmax_speed);

  iotjs_jval_t jbits_per_word =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_BITSPERWORD);
  _this->bits_per_word = (SpiOrder)iotjs_jval_as_number(&jbits_per_word);
  iotjs_jval_destroy(&jbits_per_word);

  iotjs_jval_t jbit_order =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_BITORDER);
  _this->bit_order = (SpiOrder)iotjs_jval_as_number(&jbit_order);
  iotjs_jval_destroy(&jbit_order);

  iotjs_jval_t jloopback =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_LOOPBACK);
  _this->loopback = iotjs_jval_as_boolean(&jloopback);
  iotjs_jval_destroy(&jloopback);
}


/*
 * SPI worker function
 */
static void iotjs_spi_transfer_worker(uv_work_t* work_req) {
  SPI_WORKER_INIT;

  if (!iotjs_spi_transfer(spi)) {
    req_data->result = false;
    return;
  }

  req_data->result = true;
}


static void iotjs_spi_close_worker(uv_work_t* work_req) {
  SPI_WORKER_INIT;

  if (!iotjs_spi_close(spi)) {
    req_data->result = false;
    return;
  }

  req_data->result = true;
}


static void iotjs_spi_after_work(uv_work_t* work_req, int status) {
  iotjs_spi_reqwrap_t* req_wrap = iotjs_spi_reqwrap_from_request(work_req);
  iotjs_spi_reqdata_t* req_data = iotjs_spi_reqwrap_data(req_wrap);
  iotjs_spi_t* spi = iotjs_spi_instance_from_reqwrap(req_wrap);

  iotjs_jargs_t jargs = iotjs_jargs_create(2);

  bool result = req_data->result;

  if (status) {
    iotjs_jargs_append_error(&jargs, "System error");
  } else {
    switch (req_data->op) {
      case kSpiOpOpen:
        if (!result) {
          iotjs_jargs_append_error(&jargs, "Failed to export SPI device");
        } else {
          iotjs_jargs_append_null(&jargs);
        }
        break;
      case kSpiOpTransferArray:
      case kSpiOpTransferBuffer:
        if (!result) {
          iotjs_jargs_append_error(&jargs, "Cannot transfer from SPI device");
        } else {
          iotjs_jargs_append_null(&jargs);

          IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);

          // Append read data
          iotjs_jval_t result_data =
              iotjs_jval_create_byte_array(_this->buf_len, _this->rx_buf_data);
          iotjs_jargs_append_jval(&jargs, &result_data);
          iotjs_jval_destroy(&result_data);
        }

        if (req_data->op == kSpiOpTransferArray)
          iotjs_spi_release_buffer(spi);

        break;
      case kSpiOpClose:
        if (!result) {
          iotjs_jargs_append_error(&jargs, "Failed to unexport SPI device");
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


iotjs_spi_t* iotjs_spi_get_instance(const iotjs_jval_t* jspi) {
  uintptr_t handle = iotjs_jval_get_object_native_handle(jspi);
  return (iotjs_spi_t*)(handle);
}


#define SPI_ASYNC(call, this, jcallback, op)                                   \
  do {                                                                         \
    uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());         \
    iotjs_spi_reqwrap_t* req_wrap =                                            \
        iotjs_spi_reqwrap_create(jcallback, this, op);                         \
    uv_work_t* req = iotjs_spi_reqwrap_req(req_wrap);                          \
    uv_queue_work(loop, req, iotjs_spi_##call##_worker, iotjs_spi_after_work); \
  } while (0)


JHANDLER_FUNCTION(SpiConstructor) {
  DJHANDLER_CHECK_THIS(object);
  DJHANDLER_CHECK_ARGS(2, object, function);

  // Create SPI object
  const iotjs_jval_t* jspi = JHANDLER_GET_THIS(object);
  iotjs_spi_t* spi = iotjs_spi_create(jspi);
  IOTJS_ASSERT(spi == iotjs_spi_get_instance(jspi));

  // Set configuration
  const iotjs_jval_t* jconfiguration = JHANDLER_GET_ARG(0, object);
  iotjs_spi_set_configuration(spi, jconfiguration);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG(1, function);
  SPI_ASYNC(open, spi, jcallback, kSpiOpOpen);
}


// FIXME: do not need transferArray if array buffer is implemented.
JHANDLER_FUNCTION(TransferArray) {
  JHANDLER_DECLARE_THIS_PTR(spi, spi);

  DJHANDLER_CHECK_ARGS(2, array, array);
  DJHANDLER_CHECK_ARG_IF_EXIST(2, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(2, function);

  iotjs_spi_set_array_buffer(spi, JHANDLER_GET_ARG(0, array),
                             JHANDLER_GET_ARG(1, array));

  if (jcallback) {
    SPI_ASYNC(transfer, spi, jcallback, kSpiOpTransferArray);
  } else {
    if (!iotjs_spi_transfer(spi)) {
      JHANDLER_THROW(COMMON, "SPI Transfer Error");
    } else {
      IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);

      iotjs_jval_t result =
          iotjs_jval_create_byte_array(_this->buf_len, _this->rx_buf_data);
      iotjs_jhandler_return_jval(jhandler, &result);
      iotjs_jval_destroy(&result);
    }

    iotjs_spi_release_buffer(spi);
  }
}


JHANDLER_FUNCTION(TransferBuffer) {
  JHANDLER_DECLARE_THIS_PTR(spi, spi);

  DJHANDLER_CHECK_ARGS(2, object, object);
  DJHANDLER_CHECK_ARG_IF_EXIST(2, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(2, function);

  iotjs_spi_set_buffer(spi, JHANDLER_GET_ARG(0, object),
                       JHANDLER_GET_ARG(1, object));

  if (jcallback) {
    SPI_ASYNC(transfer, spi, jcallback, kSpiOpTransferBuffer);
  } else {
    if (!iotjs_spi_transfer(spi)) {
      JHANDLER_THROW(COMMON, "SPI Transfer Error");
    } else {
      IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);

      iotjs_jval_t result =
          iotjs_jval_create_byte_array(_this->buf_len, _this->rx_buf_data);
      iotjs_jhandler_return_jval(jhandler, &result);
      iotjs_jval_destroy(&result);
    }
  }
}


JHANDLER_FUNCTION(Close) {
  JHANDLER_DECLARE_THIS_PTR(spi, spi);

  DJHANDLER_CHECK_ARG_IF_EXIST(0, function);

  const iotjs_jval_t* jcallback = JHANDLER_GET_ARG_IF_EXIST(0, function);

  if (jcallback) {
    SPI_ASYNC(close, spi, jcallback, kSpiOpClose);
  } else {
    if (!iotjs_spi_close(spi)) {
      JHANDLER_THROW(COMMON, "SPI Close Error");
    }
  }

  iotjs_jhandler_return_null(jhandler);
}


iotjs_jval_t InitSpi() {
  iotjs_jval_t jspi = iotjs_jval_create_object();
  iotjs_jval_t jspiConstructor =
      iotjs_jval_create_function_with_dispatch(SpiConstructor);
  iotjs_jval_set_property_jval(&jspi, IOTJS_MAGIC_STRING_SPI, &jspiConstructor);

  iotjs_jval_t prototype = iotjs_jval_create_object();
  iotjs_jval_set_method(&prototype, IOTJS_MAGIC_STRING_TRANSFERARRAY,
                        TransferArray);
  iotjs_jval_set_method(&prototype, IOTJS_MAGIC_STRING_TRANSFERBUFFER,
                        TransferBuffer);
  iotjs_jval_set_method(&prototype, IOTJS_MAGIC_STRING_CLOSE, Close);
  iotjs_jval_set_property_jval(&jspiConstructor, IOTJS_MAGIC_STRING_PROTOTYPE,
                               &prototype);
  iotjs_jval_destroy(&prototype);
  iotjs_jval_destroy(&jspiConstructor);

  // SPI mode properties
  iotjs_jval_t jmode = iotjs_jval_create_object();
  iotjs_jval_set_property_number(&jmode, IOTJS_MAGIC_STRING_0, kSpiMode_0);
  iotjs_jval_set_property_number(&jmode, IOTJS_MAGIC_STRING_1, kSpiMode_1);
  iotjs_jval_set_property_number(&jmode, IOTJS_MAGIC_STRING_2, kSpiMode_2);
  iotjs_jval_set_property_number(&jmode, IOTJS_MAGIC_STRING_3, kSpiMode_3);
  iotjs_jval_set_property_jval(&jspi, IOTJS_MAGIC_STRING_MODE_U, &jmode);
  iotjs_jval_destroy(&jmode);

  // SPI mode properties
  iotjs_jval_t jcs = iotjs_jval_create_object();
  iotjs_jval_set_property_number(&jcs, IOTJS_MAGIC_STRING_NONE, kSpiCsNone);
  iotjs_jval_set_property_number(&jcs, IOTJS_MAGIC_STRING_HIGH, kSpiCsHigh);
  iotjs_jval_set_property_jval(&jspi, IOTJS_MAGIC_STRING_CHIPSELECT_U, &jcs);
  iotjs_jval_destroy(&jcs);

  // SPI order properties
  iotjs_jval_t jbit_order = iotjs_jval_create_object();
  iotjs_jval_set_property_number(&jbit_order, IOTJS_MAGIC_STRING_MSB,
                                 kSpiOrderMsb);
  iotjs_jval_set_property_number(&jbit_order, IOTJS_MAGIC_STRING_LSB,
                                 kSpiOrderLsb);
  iotjs_jval_set_property_jval(&jspi, IOTJS_MAGIC_STRING_BITORDER_U,
                               &jbit_order);
  iotjs_jval_destroy(&jbit_order);

  return jspi;
}
