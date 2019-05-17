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
#include "iotjs_uv_request.h"


IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(spi);

IOTJS_DEFINE_PERIPH_CREATE_FUNCTION(spi);

static void iotjs_spi_destroy(iotjs_spi_t* spi) {
  iotjs_spi_destroy_platform_data(spi->platform_data);
  IOTJS_RELEASE(spi);
}

static int spi_get_array_data(char** buf, jerry_value_t jarray) {
  jerry_value_t jlength =
      iotjs_jval_get_property(jarray, IOTJS_MAGIC_STRING_LENGTH);
  IOTJS_ASSERT(!jerry_value_is_undefined(jlength));

  size_t length = iotjs_jval_as_number(jlength);
  IOTJS_ASSERT((int)length >= 0);
  *buf = iotjs_buffer_allocate(length);

  for (size_t i = 0; i < length; i++) {
    jerry_value_t jdata = iotjs_jval_get_property_by_index(jarray, i);
    (*buf)[i] = iotjs_jval_as_number(jdata);
    jerry_release_value(jdata);
  }

  jerry_release_value(jlength);

  return (int)length;
}

/* Default configuration:
 *{
 *  mode: spi.MODE[0],
 *  chipSelect: spi.CHIPSELECT.NONE,
 *  maxSpeed: 500000,
 *  bitsPerWord: 8,
 *  bitOrder: spi.BITORDER.MSB,
 *  loopback: false
 * }
 */
static jerry_value_t spi_set_configuration(iotjs_spi_t* spi,
                                           jerry_value_t joptions) {
  jerry_value_t jmode =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_MODE);
  if (jerry_value_is_undefined(jmode)) {
    spi->mode = kSpiMode_0;
  } else {
    if (jerry_value_is_number(jmode)) {
      spi->mode = (SpiMode)iotjs_jval_as_number(jmode);
    } else {
      spi->mode = __kSpiModeMax;
    }

    if (spi->mode >= __kSpiModeMax) {
      return JS_CREATE_ERROR(
          TYPE, "Bad arguments - mode should be MODE[0], [1], [2] or [3]");
    }
  }
  jerry_release_value(jmode);

  jerry_value_t jchip_select =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_CHIPSELECT);
  if (jerry_value_is_undefined(jchip_select)) {
    spi->chip_select = kSpiCsNone;
  } else {
    if (jerry_value_is_number(jchip_select)) {
      spi->chip_select = (SpiChipSelect)iotjs_jval_as_number(jchip_select);
    } else {
      spi->chip_select = __kSpiCsMax;
    }

    if (spi->chip_select >= __kSpiCsMax) {
      return JS_CREATE_ERROR(
          TYPE, "Bad arguments - chipSelect should be CHIPSELECT.NONE or HIGH");
    }
  }
  jerry_release_value(jchip_select);

  jerry_value_t jmax_speed =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_MAXSPEED);
  if (jerry_value_is_undefined(jmax_speed)) {
    spi->max_speed = 500000;
  } else {
    if (!jerry_value_is_number(jmax_speed)) {
      return JS_CREATE_ERROR(TYPE, "Bad arguments - maxSpeed should be Number");
    }
    spi->max_speed = iotjs_jval_as_number(jmax_speed);
  }
  jerry_release_value(jmax_speed);

  jerry_value_t jbits_per_word =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_BITSPERWORD);
  if (jerry_value_is_undefined(jbits_per_word)) {
    spi->bits_per_word = 8;
  } else {
    if (jerry_value_is_number(jbits_per_word)) {
      spi->bits_per_word = iotjs_jval_as_number(jbits_per_word);
    } else {
      spi->bits_per_word = 0;
    }

    if (spi->bits_per_word != 8 && spi->bits_per_word != 9) {
      return JS_CREATE_ERROR(TYPE,
                             "Bad arguments - bitsPerWord should be 8 or 9");
    }
  }
  jerry_release_value(jbits_per_word);

  jerry_value_t jbit_order =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_BITORDER);
  if (jerry_value_is_undefined(jbit_order)) {
    spi->bit_order = kSpiOrderMsb;
  } else {
    if (jerry_value_is_number(jbit_order)) {
      spi->bit_order = (SpiOrder)iotjs_jval_as_number(jbit_order);
    } else {
      spi->bit_order = __kSpiOrderMax;
    }

    if (spi->bit_order >= __kSpiOrderMax) {
      return JS_CREATE_ERROR(
          TYPE, "Bad arguments - bitOrder should be BITORDER.MSB or LSB");
    }
  }
  jerry_release_value(jbit_order);

  jerry_value_t jloopback =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_LOOPBACK);
  if (jerry_value_is_undefined(jloopback)) {
    spi->loopback = false;
  } else {
    if (!jerry_value_is_boolean(jloopback)) {
      return JS_CREATE_ERROR(TYPE,
                             "Bad arguments - loopback should be Boolean");
    }
    spi->loopback = iotjs_jval_as_boolean(jloopback);
  }
  jerry_release_value(jloopback);

  return jerry_create_undefined();
}

/*
 * SPI worker function
 */
static void spi_worker(uv_work_t* work_req) {
  iotjs_periph_data_t* worker_data =
      (iotjs_periph_data_t*)IOTJS_UV_REQUEST_EXTRA_DATA(work_req);
  iotjs_spi_t* spi = (iotjs_spi_t*)worker_data->data;

  switch (worker_data->op) {
    case kSpiOpClose: {
      worker_data->result = iotjs_spi_close(spi);
      break;
    }
    case kSpiOpOpen: {
      worker_data->result = iotjs_spi_open(spi);
      break;
    }
    case kSpiOpTransferArray:
    case kSpiOpTransferBuffer: {
      worker_data->result = iotjs_spi_transfer(spi);
      break;
    }
    default:
      IOTJS_ASSERT(!"Invalid Operation");
  }
}

JS_FUNCTION(spi_constructor) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, object);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  // Create SPI object
  jerry_value_t jspi = JS_GET_THIS();
  iotjs_spi_t* spi = spi_create(jspi);

  // Set configuration
  jerry_value_t jconfig;
  JS_GET_REQUIRED_ARG_VALUE(0, jconfig, IOTJS_MAGIC_STRING_CONFIG, object);

  jerry_value_t res = iotjs_spi_set_platform_config(spi, jconfig);
  if (jerry_value_is_error(res)) {
    return res;
  }
  IOTJS_ASSERT(jerry_value_is_undefined(res));

  res = spi_set_configuration(spi, jconfig);
  if (jerry_value_is_error(res)) {
    return res;
  }
  IOTJS_ASSERT(jerry_value_is_undefined(res));

  jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(1, function);

  // If the callback doesn't exist, it is completed synchronously.
  // Otherwise, it will be executed asynchronously.
  if (!jerry_value_is_null(jcallback)) {
    iotjs_periph_call_async(spi, jcallback, kSpiOpOpen, spi_worker);
  } else if (!iotjs_spi_open(spi)) {
    return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kSpiOpOpen));
  }

  return jerry_create_undefined();
}

static uint8_t spi_transfer_helper(jerry_value_t jtx_buf, iotjs_spi_t* spi) {
  uint8_t result = kSpiOpTransferBuffer;
  if (jerry_value_is_array(jtx_buf)) {
    spi->buf_len = spi_get_array_data(&spi->tx_buf_data, jtx_buf);
    result = kSpiOpTransferArray;
  } else if (jerry_value_is_object(jtx_buf)) {
    iotjs_bufferwrap_t* tx_buf = iotjs_bufferwrap_from_jbuffer(jtx_buf);

    spi->tx_buf_data = tx_buf->buffer;
    spi->buf_len = iotjs_bufferwrap_length(tx_buf);
  }

  IOTJS_ASSERT(spi->buf_len > 0);
  spi->rx_buf_data = iotjs_buffer_allocate(spi->buf_len);
  IOTJS_ASSERT(spi->tx_buf_data != NULL && spi->rx_buf_data != NULL);

  return result;
}

// FIXME: do not need transferArray if array buffer is implemented.
JS_FUNCTION(spi_transfer) {
  JS_DECLARE_THIS_PTR(spi, spi);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  uint8_t op = spi_transfer_helper(jargv[0], spi);
  iotjs_periph_call_async(spi, JS_GET_ARG_IF_EXIST(1, function), op,
                          spi_worker);

  return jerry_create_undefined();
}

JS_FUNCTION(spi_transfer_sync) {
  JS_DECLARE_THIS_PTR(spi, spi);

  uint8_t op = spi_transfer_helper(jargv[0], spi);

  jerry_value_t result = jerry_create_undefined();
  if (!iotjs_spi_transfer(spi)) {
    result = JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(op));
  } else {
    result = iotjs_jval_create_byte_array(spi->buf_len, spi->rx_buf_data);
  }

  if (op == kSpiOpTransferArray) {
    IOTJS_RELEASE(spi->tx_buf_data);
  }

  IOTJS_RELEASE(spi->rx_buf_data);

  return result;
}

JS_FUNCTION(spi_close) {
  JS_DECLARE_THIS_PTR(spi, spi);
  DJS_CHECK_ARG_IF_EXIST(1, function);

  iotjs_periph_call_async(spi, JS_GET_ARG_IF_EXIST(0, function), kSpiOpClose,
                          spi_worker);

  return jerry_create_undefined();
}

JS_FUNCTION(spi_close_sync) {
  JS_DECLARE_THIS_PTR(spi, spi);

  if (!iotjs_spi_close(spi)) {
    return JS_CREATE_ERROR(COMMON, iotjs_periph_error_str(kSpiOpClose));
  }

  return jerry_create_undefined();
}

jerry_value_t iotjs_init_spi(void) {
  jerry_value_t jspi_cons = jerry_create_external_function(spi_constructor);

  jerry_value_t prototype = jerry_create_object();
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_CLOSE, spi_close);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_CLOSESYNC,
                        spi_close_sync);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_TRANSFER, spi_transfer);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_TRANSFERSYNC,
                        spi_transfer_sync);

  iotjs_jval_set_property_jval(jspi_cons, IOTJS_MAGIC_STRING_PROTOTYPE,
                               prototype);
  jerry_release_value(prototype);

  // SPI mode properties
  jerry_value_t jmode = jerry_create_object();
  iotjs_jval_set_property_number(jmode, IOTJS_MAGIC_STRING_0, kSpiMode_0);
  iotjs_jval_set_property_number(jmode, IOTJS_MAGIC_STRING_1, kSpiMode_1);
  iotjs_jval_set_property_number(jmode, IOTJS_MAGIC_STRING_2, kSpiMode_2);
  iotjs_jval_set_property_number(jmode, IOTJS_MAGIC_STRING_3, kSpiMode_3);
  iotjs_jval_set_property_jval(jspi_cons, IOTJS_MAGIC_STRING_MODE_U, jmode);
  jerry_release_value(jmode);

  // SPI mode properties
  jerry_value_t jcs = jerry_create_object();
  iotjs_jval_set_property_number(jcs, IOTJS_MAGIC_STRING_NONE_U, kSpiCsNone);
  iotjs_jval_set_property_number(jcs, IOTJS_MAGIC_STRING_HIGH_U, kSpiCsHigh);
  iotjs_jval_set_property_jval(jspi_cons, IOTJS_MAGIC_STRING_CHIPSELECT_U, jcs);
  jerry_release_value(jcs);

  // SPI order properties
  jerry_value_t jbit_order = jerry_create_object();
  iotjs_jval_set_property_number(jbit_order, IOTJS_MAGIC_STRING_MSB,
                                 kSpiOrderMsb);
  iotjs_jval_set_property_number(jbit_order, IOTJS_MAGIC_STRING_LSB,
                                 kSpiOrderLsb);
  iotjs_jval_set_property_jval(jspi_cons, IOTJS_MAGIC_STRING_BITORDER_U,
                               jbit_order);
  jerry_release_value(jbit_order);

  return jspi_cons;
}
