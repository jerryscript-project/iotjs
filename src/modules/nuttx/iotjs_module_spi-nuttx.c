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

#if defined(__NUTTX__)

#include <nuttx/board.h>

#include "iotjs_systemio-nuttx.h"
#include "chip.h"
#include "modules/iotjs_module_spi.h"


bool iotjs_spi_transfer(iotjs_spi_t* spi) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);

  struct spi_dev_s* spi_dev = _this->spi_dev;

  SPI_LOCK(spi_dev, true);

  SPI_SETFREQUENCY(spi_dev, _this->max_speed);

  SPI_SETMODE(spi_dev, _this->mode);
  SPI_SETBITS(spi_dev, _this->bits_per_word);

  // Select the SPI
  iotjs_gpio_write_nuttx(_this->cs_chip, false);

  SPI_EXCHANGE(spi_dev, _this->tx_buf_data, _this->rx_buf_data, _this->buf_len);

  // Unselect the SPI device
  iotjs_gpio_write_nuttx(_this->cs_chip, true);

  SPI_LOCK(spi_dev, false);

  return true;
}


bool iotjs_spi_close(iotjs_spi_t* spi) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);

  iotjs_gpio_unconfig_nuttx(_this->cs_chip);

  return true;
}


void iotjs_spi_open_worker(uv_work_t* work_req) {
  SPI_WORKER_INIT;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);

  switch (_this->bus) {
    case 1:
      _this->cs_chip = (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz |
                        GPIO_PORTA | GPIO_PIN15 | GPIO_OUTPUT_SET);
      break;
    default:
      req_data->result = false;
      return;
  }

  iotjs_gpio_config_nuttx(_this->cs_chip);

  if (!(_this->spi_dev = iotjs_spi_config_nuttx(_this->bus, _this->cs_chip))) {
    DLOG("%s - SPI open failed %d", __func__, _this->bus);
    req_data->result = false;
    return;
  }

  req_data->result = true;
}


#endif // __NUTTX__
