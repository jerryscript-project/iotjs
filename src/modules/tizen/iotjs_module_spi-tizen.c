/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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

#include <peripheral_io.h>

#include "modules/iotjs_module_spi.h"


struct iotjs_spi_platform_data_s {
  int bus;
  peripheral_spi_h spi_h;
};

static peripheral_spi_mode_e mode_to_constant(SpiMode mode) {
  switch (mode) {
    case kSpiMode_0:
      return PERIPHERAL_SPI_MODE_0;
    case kSpiMode_1:
      return PERIPHERAL_SPI_MODE_1;
    case kSpiMode_2:
      return PERIPHERAL_SPI_MODE_2;
    case kSpiMode_3:
      return PERIPHERAL_SPI_MODE_3;
    default:
      IOTJS_ASSERT(!"Invalid SPI mode");
      return PERIPHERAL_SPI_MODE_0;
  }
}

static peripheral_spi_bit_order_e bit_order_to_constant(SpiOrder order) {
  switch (order) {
    case kSpiOrderLsb:
      return PERIPHERAL_SPI_BIT_ORDER_LSB;
    case kSpiOrderMsb:
      return PERIPHERAL_SPI_BIT_ORDER_MSB;
    default:
      IOTJS_ASSERT(!"Invalid SPI bitOrder");
      return PERIPHERAL_SPI_BIT_ORDER_MSB;
  }
}

void iotjs_spi_create_platform_data(iotjs_spi_t* spi) {
  spi->platform_data = IOTJS_ALLOC(iotjs_spi_platform_data_t);

  spi->platform_data->spi_h = NULL;
}

void iotjs_spi_destroy_platform_data(iotjs_spi_platform_data_t* platform_data) {
  IOTJS_ASSERT(platform_data);
  IOTJS_RELEASE(platform_data);
}

jerry_value_t iotjs_spi_set_platform_config(iotjs_spi_t* spi,
                                            const jerry_value_t jconfig) {
  JS_GET_REQUIRED_CONF_VALUE(jconfig, spi->platform_data->bus,
                             IOTJS_MAGIC_STRING_BUS, number);

  return jerry_create_undefined();
}

#define SPI_METHOD_HEADER(arg)                                   \
  iotjs_spi_platform_data_t* platform_data = arg->platform_data; \
  IOTJS_ASSERT(platform_data);                                   \
  if (!platform_data->spi_h) {                                   \
    DLOG("%s: SPI is not opened", __func__);                     \
    return false;                                                \
  }

bool iotjs_spi_open(iotjs_spi_t* spi) {
  iotjs_spi_platform_data_t* platform_data = spi->platform_data;

  int ret = peripheral_spi_open(platform_data->bus, spi->chip_select,
                                &platform_data->spi_h);
  if (ret != PERIPHERAL_ERROR_NONE) {
    DLOG("%s: cannot open(%d)", __func__, ret);
    return false;
  }

  // Set mode
  ret = peripheral_spi_set_mode(platform_data->spi_h,
                                mode_to_constant(spi->mode));
  if (ret != PERIPHERAL_ERROR_NONE) {
    DLOG("%s: cannot set mode(%d)", __func__, ret);
    peripheral_spi_close(platform_data->spi_h);
    return false;
  }

  // Set bit order
  ret = peripheral_spi_set_bit_order(platform_data->spi_h,
                                     bit_order_to_constant(spi->bit_order));
  if (ret != PERIPHERAL_ERROR_NONE) {
    DLOG("%s: cannot set bit order(%d)", __func__, ret);
    peripheral_spi_close(platform_data->spi_h);
    return false;
  }

  // Set bits per word
  ret = peripheral_spi_set_bits_per_word(platform_data->spi_h,
                                         spi->bits_per_word);
  if (ret != PERIPHERAL_ERROR_NONE) {
    DLOG("%s: cannot set bit per word(%d)", __func__, ret);
    peripheral_spi_close(platform_data->spi_h);
    return false;
  }

  // Set maxSpeed
  ret = peripheral_spi_set_frequency(platform_data->spi_h, spi->max_speed);
  if (ret != PERIPHERAL_ERROR_NONE) {
    DLOG("%s: cannot set maxSpeed(%d)", __func__, ret);
    peripheral_spi_close(platform_data->spi_h);
    return false;
  }
  return true;
}

bool iotjs_spi_transfer(iotjs_spi_t* spi) {
  SPI_METHOD_HEADER(spi)

  int ret =
      peripheral_spi_transfer(platform_data->spi_h, (uint8_t*)spi->tx_buf_data,
                              (uint8_t*)spi->rx_buf_data, spi->buf_len);
  if (ret != PERIPHERAL_ERROR_NONE) {
    DLOG("%s: cannot transfer(%d)", __func__, ret);
    return false;
  }

  return true;
}

bool iotjs_spi_close(iotjs_spi_t* spi) {
  SPI_METHOD_HEADER(spi)

  int ret = peripheral_spi_close(platform_data->spi_h);
  if (ret != PERIPHERAL_ERROR_NONE) {
    DLOG("%s: cannot close(%d)", __func__, ret);
    return false;
  }

  platform_data->spi_h = NULL;
  return true;
}
