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

#if !defined(__NUTTX__)
#error "Module __FILE__ is for nuttx only"
#endif

#include <nuttx/board.h>

#include "iotjs_systemio-nuttx.h"
#include "chip.h"
#include "modules/iotjs_module_spi.h"


struct iotjs_spi_platform_data_s {
  int bus;
  uint32_t cs_chip;
  struct spi_dev_s* spi_dev;
};

void iotjs_spi_create_platform_data(iotjs_spi_t* spi) {
  spi->platform_data = IOTJS_ALLOC(iotjs_spi_platform_data_t);
  spi->platform_data->bus = -1;
  spi->platform_data->cs_chip = 0;
  spi->platform_data->spi_dev = NULL;
}

void iotjs_spi_destroy_platform_data(iotjs_spi_platform_data_t* pdata) {
  IOTJS_RELEASE(pdata);
}

jerry_value_t iotjs_spi_set_platform_config(iotjs_spi_t* spi,
                                            const jerry_value_t jconfig) {
  iotjs_spi_platform_data_t* platform_data = spi->platform_data;

  JS_GET_REQUIRED_CONF_VALUE(jconfig, platform_data->bus,
                             IOTJS_MAGIC_STRING_BUS, number);

  return jerry_create_undefined();
}

bool iotjs_spi_transfer(iotjs_spi_t* spi) {
  iotjs_spi_platform_data_t* platform_data = spi->platform_data;

  struct spi_dev_s* spi_dev = platform_data->spi_dev;

  SPI_LOCK(spi_dev, true);

  SPI_SETFREQUENCY(spi_dev, spi->max_speed);

  SPI_SETMODE(spi_dev, spi->mode);
  SPI_SETBITS(spi_dev, spi->bits_per_word);

  // Select the SPI
  iotjs_gpio_write_nuttx(platform_data->cs_chip, false);

  SPI_EXCHANGE(spi_dev, spi->tx_buf_data, spi->rx_buf_data, spi->buf_len);

  // Unselect the SPI device
  iotjs_gpio_write_nuttx(platform_data->cs_chip, true);

  SPI_LOCK(spi_dev, false);

  return true;
}

bool iotjs_spi_close(iotjs_spi_t* spi) {
  iotjs_spi_platform_data_t* platform_data = spi->platform_data;

  iotjs_gpio_unconfig_nuttx(platform_data->cs_chip);

  return true;
}

bool iotjs_spi_open(iotjs_spi_t* spi) {
  iotjs_spi_platform_data_t* platform_data = spi->platform_data;

  switch (platform_data->bus) {
    case 1:
      platform_data->cs_chip = (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz |
                                GPIO_PORTA | GPIO_PIN15 | GPIO_OUTPUT_SET);
      break;
    default:
      return false;
  }

  iotjs_gpio_config_nuttx(platform_data->cs_chip);

  if (!(platform_data->spi_dev =
            iotjs_spi_config_nuttx(platform_data->bus,
                                   platform_data->cs_chip))) {
    DLOG("%s - SPI open failed %d", __func__, platform_data->bus);
    return false;
  }

  return true;
}
