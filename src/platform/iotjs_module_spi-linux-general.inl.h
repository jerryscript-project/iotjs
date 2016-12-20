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
#include "module/iotjs_module_buffer.h"
#include "module/iotjs_module_spi.h"

#define ADC_DEVICE_PATH_FORMAT "/dev/spidev%d.%d"
#define ADC_DEVICE_PATH_BUFFER_SIZE 16


#define SPI_WORKER_INIT_TEMPLATE                                            \
  iotjs_spi_reqwrap_t* req_wrap = iotjs_spi_reqwrap_from_request(work_req); \
  iotjs_spi_reqdata_t* req_data = iotjs_spi_reqwrap_data(req_wrap);


bool iotjs_spi_set_options(iotjs_spi_t* spi, uint32_t selected_option) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, spi);

  int fd = _this->device_fd;
  if (fd < 0) {
    return false;
  }

  uint8_t data;
  if ((selected_option & kSpiOptionMode) ||
      (selected_option & kSpiOptionLoopback) ||
      (selected_option & kSpiOptionChipSelect)) {
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
  }

  if ((selected_option & kSpiOptionBitOrder) &&
      _this->bit_order == kSpiOrderLsb) {
    data = 1;
    if (ioctl(fd, SPI_IOC_WR_LSB_FIRST, &data) < 0) {
      return false;
    }
  }

  if ((selected_option & kSpiOptionBitsPerWord)) {
    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &_this->bits_per_word) < 0) {
      return false;
    }
  }

  if ((selected_option & kSpiOptionMaxSpeed)) {
    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &_this->max_speed) < 0) {
      return false;
    }
  }

  DDLOG(
      "SPI Options \n mode: %d\n chipSelect: %d\n bitOrder: %d\n "
      "maxSpeed: %d\n bitPerWord: %d\n loopback: %d",
      _this->mode, _this->chip_select, _this->bit_order, _this->max_speed,
      _this->bits_per_word, _this->loopback);

  return true;
}


void iotjs_spi_export_worker(uv_work_t* work_req) {
  SPI_WORKER_INIT_TEMPLATE;

  // Set device driver path
  int32_t pin = req_data->pin;

  char path[ADC_DEVICE_PATH_BUFFER_SIZE] = { 0 };
  snprintf(path, ADC_DEVICE_PATH_BUFFER_SIZE - 1, ADC_DEVICE_PATH_FORMAT,
           SPI_GET_DEVICE_NUMBER(req_data->pin),
           SPI_GET_CS_NUMBER(req_data->pin));

  // Open file
  const iotjs_environment_t* env = iotjs_environment_get();
  uv_loop_t* loop = iotjs_environment_loop(env);

  uv_fs_t open_req;
  int result = uv_fs_open(loop, &open_req, path, O_RDONLY, 0666, NULL);
  uv_fs_req_cleanup(&open_req);
  if (result < 0) {
    req_data->result = kSpiErrExport;
  }

  iotjs_spi_set_device_fd(req_data->spi_instance, open_req.result);

  // Set options
  if (!iotjs_spi_set_options(req_data->spi_instance,
                             req_data->selected_option)) {
    req_data->result = kSpiErrExport;
    return;
  }

  req_data->result = kSpiErrOk;
}


void iotjs_spi_transfer_worker(uv_work_t* work_req) {
  SPI_WORKER_INIT_TEMPLATE;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, req_data->spi_instance);

  struct spi_ioc_transfer data = {.tx_buf = (unsigned long)_this->tx_buf_data,
                                  .rx_buf = (unsigned long)_this->rx_buf_data,
                                  .len = _this->buf_len,
                                  .speed_hz = _this->max_speed,
                                  .bits_per_word = _this->bits_per_word,
                                  .delay_usecs = 0 };

  // Transfer data
  int err = ioctl(_this->device_fd, SPI_IOC_MESSAGE(1), &data);
  if (err < 1) {
    DDLOG("SPI transfer worker - transfer failed: %d", err);
    req_data->result = kSpiErrTransfer;
    return;
  }

  req_data->result = kSpiErrOk;
}


void iotjs_spi_unexport_worker(uv_work_t* work_req) {
  SPI_WORKER_INIT_TEMPLATE;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_spi_t, req_data->spi_instance);

  // Close file
  if (_this->device_fd >= 0) {
    const iotjs_environment_t* env = iotjs_environment_get();
    uv_loop_t* loop = iotjs_environment_loop(env);
    uv_fs_t fs_close_req;

    int err = uv_fs_close(loop, &fs_close_req, _this->device_fd, NULL);
    uv_fs_req_cleanup(&fs_close_req);
    if (err < 0) {
      DDLOG("SPI unexport worker - close failed: %d", err);
      req_data->result = kSpiErrUnexport;
      return;
    }
    _this->device_fd = -1;
  }

  req_data->result = kSpiErrOk;
}


#endif /* IOTJS_MODULE_SPI_LINUX_GENERAL_INL_H */
