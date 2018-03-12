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

#if !defined(__TIZENRT__)
#error "Module __FILE__ is for TizenRT only"
#endif

#include <tinyara/config.h>

#if !defined(CONFIG_SPI)
#error "\n\nTizenRT CONFIG_SPI configuration flag required for SPI module\n\n"
#elif !defined(CONFIG_SPI_EXCHANGE)
#error "\n\nTizenRT CONFIG_SPI_EXCHANGE flag required for SPI module\n\n"
#endif

#include <iotbus_error.h>
#include <iotbus_spi.h>

#include "modules/iotjs_module_spi.h"


struct iotjs_spi_platform_data_s {
  unsigned int bus;
  iotbus_spi_context_h spi_context;
};

void iotjs_spi_create_platform_data(iotjs_spi_t* spi) {
  spi->platform_data = IOTJS_ALLOC(iotjs_spi_platform_data_t);

  spi->platform_data->spi_context = NULL;
}


void iotjs_spi_destroy_platform_data(iotjs_spi_platform_data_t* platform_data) {
  IOTJS_ASSERT(platform_data);
  IOTJS_RELEASE(platform_data);
}


jerry_value_t iotjs_spi_set_platform_config(iotjs_spi_t* spi,
                                            const jerry_value_t jconfig) {
  iotjs_spi_platform_data_t* platform_data = spi->platform_data;

  JS_GET_REQUIRED_CONF_VALUE(jconfig, platform_data->bus,
                             IOTJS_MAGIC_STRING_BUS, number);

  return jerry_create_undefined();
}

bool iotjs_spi_open(iotjs_spi_t* spi) {
  iotjs_spi_platform_data_t* platform_data = spi->platform_data;

  struct iotbus_spi_config_s cfg = {.bits_per_word = spi->bits_per_word,
                                    .chip_select = spi->chip_select,
                                    .frequency = spi->max_speed };

  switch (spi->mode) {
    case kSpiMode_0:
      cfg.mode = IOTBUS_SPI_MODE0;
      break;
    case kSpiMode_1:
      cfg.mode = IOTBUS_SPI_MODE1;
      break;
    case kSpiMode_2:
      cfg.mode = IOTBUS_SPI_MODE2;
      break;
    case kSpiMode_3:
      cfg.mode = IOTBUS_SPI_MODE3;
      break;
    default:
      cfg.mode = IOTBUS_SPI_MODE0;
  }

  platform_data->spi_context = iotbus_spi_open(platform_data->bus, &cfg);
  if (platform_data->spi_context == NULL) {
    return false;
  }

  DDLOG(
      "SPI Options \n mode: %d\n chipSelect: %d\n bitOrder: %d\n "
      "maxSpeed: %d\n bitPerWord: %d\n loopback: %d",
      spi->mode, spi->chip_select, spi->bit_order, spi->max_speed,
      spi->bits_per_word, spi->loopback);

  return true;
}


bool iotjs_spi_transfer(iotjs_spi_t* spi) {
  iotjs_spi_platform_data_t* platform_data = spi->platform_data;

  int err =
      iotbus_spi_transfer_buf(platform_data->spi_context,
                              (unsigned char*)spi->tx_buf_data,
                              (unsigned char*)spi->rx_buf_data, spi->buf_len);
  if (err != 0) {
    DDLOG("%s - transfer failed: %d", __func__, err);
    return false;
  }

  return true;
}


bool iotjs_spi_close(iotjs_spi_t* spi) {
  iotjs_spi_platform_data_t* platform_data = spi->platform_data;

  if (platform_data->spi_context != NULL) {
    int err = iotbus_spi_close(platform_data->spi_context);
    if (err != 0) {
      DDLOG("%s - close failed: %d", __func__, err);
      return false;
    }
    platform_data->spi_context = NULL;
  }

  return true;
}
