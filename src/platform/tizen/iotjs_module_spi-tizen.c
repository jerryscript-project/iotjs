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

#include "modules/iotjs_module_spi.h"
#include <peripheral_io.h>

struct _iotjs_spi_module_platform_t {
  peripheral_spi_h peripheral_spi;
  int bus;
};

static bool iotjs_spi_set_configuration(iotjs_spi_t* spi) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);

  peripheral_spi_mode_e mode;

  switch (_this->mode) {
    case kSpiMode_0:
      mode = PERIPHERAL_SPI_MODE_0;
      break;
    case kSpiMode_1:
      mode = PERIPHERAL_SPI_MODE_1;
      break;
    case kSpiMode_2:
      mode = PERIPHERAL_SPI_MODE_2;
      break;
    case kSpiMode_3:
      mode = PERIPHERAL_SPI_MODE_3;
      break;
    default:
      mode = PERIPHERAL_SPI_MODE_0;
  }

  if (peripheral_spi_set_mode(_this->platform->peripheral_spi, mode) < 0)
    return false;

  if (peripheral_spi_set_lsb_first(_this->platform->peripheral_spi,
                                   _this->bit_order == kSpiOrderLsb) < 0)
    return false;

  if (peripheral_spi_set_bits_per_word(_this->platform->peripheral_spi,
                                       _this->bits_per_word) < 0)
    return false;

  if (peripheral_spi_set_frequency(_this->platform->peripheral_spi,
                                   _this->max_speed) < 0)
    return false;

  DDDLOG(
      "SPI Options \n mode: %d\n bitOrder: %d\n maxSpeed: %d\n bitPerWord: "
      "%d\n loopback: %d",
      _this->mode, _this->bit_order, _this->max_speed, _this->bits_per_word);

  return true;
}


bool iotjs_spi_transfer(iotjs_spi_t* spi) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);

  int err = peripheral_spi_read_write(_this->platform->peripheral_spi,
                                      (unsigned char*)_this->tx_buf_data,
                                      (unsigned char*)_this->rx_buf_data,
                                      _this->buf_len);
  if (err < 1) {
    DLOG("%s - transfer failed: %d", __func__, err);
    return false;
  }

  return true;
}


bool iotjs_spi_close(iotjs_spi_t* spi) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);

  if (peripheral_spi_close(_this->platform->peripheral_spi) < 0)
    return false;

  return true;
}


void iotjs_spi_open_worker(uv_work_t* work_req) {
  SPI_WORKER_INIT;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);

  if (peripheral_spi_open(_this->platform->bus, _this->chip_select,
                          &_this->platform->peripheral_spi) < 0) {
    DLOG("%s - SPI open failed %d", __func__, _this->platform->bus);
    req_data->result = false;
    return;
  }
  if (!iotjs_spi_set_configuration(spi)) {
    req_data->result = false;
    return;
  }
  req_data->result = true;
}

void iotjs_spi_platform_create(iotjs_spi_t_impl_t* _this) {
  _this->platform = IOTJS_ALLOC(struct _iotjs_spi_module_platform_t);
}

void iotjs_spi_platform_destroy(iotjs_spi_t_impl_t* _this) {
  IOTJS_RELEASE(_this->platform);
}

void iotjs_spi_platform_set_cofiguration(iotjs_spi_t_impl_t* _this,
                                         const iotjs_jval_t* joptions) {
  iotjs_jval_t jbus = iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_BUS);
  _this->platform->bus = iotjs_jval_as_number(&jbus);
  iotjs_jval_destroy(&jbus);
}
