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

#ifndef IOTJS_MODULE_SPI_LINUX_GENERAL_INL_H
#define IOTJS_MODULE_SPI_LINUX_GENERAL_INL_H


#include <uv.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

#include "iotjs_def.h"
#include "iotjs_systemio-linux.h"
#include "modules/iotjs_module_buffer.h"
#include "modules/iotjs_module_spi.h"

#define ADC_DEVICE_PATH_FORMAT "/dev/spidev%d.%d"
#define ADC_DEVICE_PATH_BUFFER_SIZE 16


static bool iotjs_spi_set_configuration(iotjs_spi_t* spi) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);

  int fd = _this->device_fd;
  if (fd < 0) {
    return false;
  }

  uint8_t data;

  switch (_this->mode) {
    case kSpiMode_0:
      data = SPI_MODE_0;
      break;
    case kSpiMode_1:
      data = SPI_MODE_1;
      break;
    case kSpiMode_2:
      data = SPI_MODE_2;
      break;
    case kSpiMode_3:
      data = SPI_MODE_3;
      break;
    default:
      data = SPI_MODE_0;
  }
  if (_this->loopback) {
    data |= SPI_LOOP;
  }

  if (_this->chip_select == kSpiCsHigh) {
    data |= SPI_CS_HIGH;
  }

  if (ioctl(fd, SPI_IOC_WR_MODE, &_this->mode) < 0) {
    return false;
  }


  if (_this->bit_order == kSpiOrderLsb) {
    data = 1;
    if (ioctl(fd, SPI_IOC_WR_LSB_FIRST, &data) < 0) {
      return false;
    }
  }

  if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &_this->bits_per_word) < 0) {
    return false;
  }

  if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &_this->max_speed) < 0) {
    return false;
  }

  DDDLOG(
      "SPI Options \n mode: %d\n chipSelect: %d\n bitOrder: %d\n "
      "maxSpeed: %d\n bitPerWord: %d\n loopback: %d",
      _this->mode, _this->chip_select, _this->bit_order, _this->max_speed,
      _this->bits_per_word, _this->loopback);

  return true;
}


bool iotjs_spi_transfer(iotjs_spi_t* spi) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);

  struct spi_ioc_transfer data = {.tx_buf = (unsigned long)_this->tx_buf_data,
                                  .rx_buf = (unsigned long)_this->rx_buf_data,
                                  .len = _this->buf_len,
                                  .speed_hz = _this->max_speed,
                                  .bits_per_word = _this->bits_per_word,
                                  .delay_usecs = 0 };

  // Transfer data
  int err = ioctl(_this->device_fd, SPI_IOC_MESSAGE(1), &data);
  if (err < 1) {
    DLOG("%s - transfer failed: %d", __func__, err);
    return false;
  }

  return true;
}


bool iotjs_spi_close(iotjs_spi_t* spi) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);

  if (_this->device_fd >= 0) {
    const iotjs_environment_t* env = iotjs_environment_get();
    uv_loop_t* loop = iotjs_environment_loop(env);
    uv_fs_t fs_close_req;

    int err = uv_fs_close(loop, &fs_close_req, _this->device_fd, NULL);
    uv_fs_req_cleanup(&fs_close_req);
    if (err < 0) {
      DLOG("%s - close failed: %d", __func__, err);
      return false;
    }
    _this->device_fd = -1;
  }

  return true;
}


void iotjs_spi_open_worker(uv_work_t* work_req) {
  SPI_WORKER_INIT;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);

  const char* device_path = iotjs_string_data(&_this->device);
  if (iotjs_systemio_check_path(device_path)) {
    // Open file
    const iotjs_environment_t* env = iotjs_environment_get();
    uv_loop_t* loop = iotjs_environment_loop(env);

    uv_fs_t open_req;
    int result = uv_fs_open(loop, &open_req, device_path, O_RDONLY, 0666, NULL);
    uv_fs_req_cleanup(&open_req);
    if (result < 0) {
      req_data->result = false;
    }
    _this->device_fd = open_req.result;

    // Set options
    if (!iotjs_spi_set_configuration(spi)) {
      req_data->result = false;
      return;
    }
    req_data->result = true;
  } else {
    req_data->result = false;
  }
}


#endif /* IOTJS_MODULE_SPI_LINUX_GENERAL_INL_H */
