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

#include <uv.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

#include "iotjs_def.h"
#include "iotjs_systemio-linux.h"
#include "modules/iotjs_module_buffer.h"
#include "modules/iotjs_module_spi.h"

#define ADC_DEVICE_PATH_FORMAT "/dev/spidev%d.%d"
#define ADC_DEVICE_PATH_BUFFER_SIZE 16

struct iotjs_spi_platform_data_s {
  iotjs_string_t device;
  int device_fd;
};

void iotjs_spi_create_platform_data(iotjs_spi_t* spi) {
  spi->platform_data = IOTJS_ALLOC(iotjs_spi_platform_data_t);
  spi->platform_data->device_fd = -1;
}

void iotjs_spi_destroy_platform_data(iotjs_spi_platform_data_t* pdata) {
  iotjs_string_destroy(&pdata->device);
  IOTJS_RELEASE(pdata);
}

jerry_value_t iotjs_spi_set_platform_config(iotjs_spi_t* spi,
                                            const jerry_value_t jconfig) {
  iotjs_spi_platform_data_t* platform_data = spi->platform_data;

  JS_GET_REQUIRED_CONF_VALUE(jconfig, platform_data->device,
                             IOTJS_MAGIC_STRING_DEVICE, string);

  return jerry_create_undefined();
}

static bool spi_set_configuration(iotjs_spi_t* spi) {
  int fd = spi->platform_data->device_fd;
  if (fd < 0) {
    return false;
  }

  uint8_t data;

  switch (spi->mode) {
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
  if (spi->loopback) {
    data |= SPI_LOOP;
  }

  if (spi->chip_select == kSpiCsHigh) {
    data |= SPI_CS_HIGH;
  }

  if (ioctl(fd, SPI_IOC_WR_MODE, &spi->mode) < 0) {
    return false;
  }


  if (spi->bit_order == kSpiOrderLsb) {
    data = 1;
    if (ioctl(fd, SPI_IOC_WR_LSB_FIRST, &data) < 0) {
      return false;
    }
  }

  if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &spi->bits_per_word) < 0) {
    return false;
  }

  if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi->max_speed) < 0) {
    return false;
  }

  DDDLOG(
      "SPI Options \n mode: %d\n chipSelect: %d\n bitOrder: %d\n "
      "maxSpeed: %d\n bitPerWord: %d\n loopback: %d",
      spi->mode, spi->chip_select, spi->bit_order, spi->max_speed,
      spi->bits_per_word, spi->loopback);

  return true;
}

bool iotjs_spi_transfer(iotjs_spi_t* spi) {
  iotjs_spi_platform_data_t* platform_data = spi->platform_data;

  struct spi_ioc_transfer data = {.tx_buf = (unsigned long)spi->tx_buf_data,
                                  .rx_buf = (unsigned long)spi->rx_buf_data,
                                  .len = spi->buf_len,
                                  .speed_hz = spi->max_speed,
                                  .bits_per_word = spi->bits_per_word,
                                  .delay_usecs = 0 };

  // Transfer data
  int err = ioctl(platform_data->device_fd, SPI_IOC_MESSAGE(1), &data);
  if (err < 1) {
    DLOG("%s - transfer failed: %d", __func__, err);
    return false;
  }

  return true;
}

bool iotjs_spi_close(iotjs_spi_t* spi) {
  iotjs_spi_platform_data_t* platform_data = spi->platform_data;

  if (platform_data->device_fd >= 0) {
    const iotjs_environment_t* env = iotjs_environment_get();
    uv_loop_t* loop = iotjs_environment_loop(env);
    uv_fs_t fs_close_req;

    int err = uv_fs_close(loop, &fs_close_req, platform_data->device_fd, NULL);
    uv_fs_req_cleanup(&fs_close_req);
    if (err < 0) {
      DLOG("%s - close failed: %d", __func__, err);
      return false;
    }
    platform_data->device_fd = -1;
  }

  return true;
}


bool iotjs_spi_open(iotjs_spi_t* spi) {
  iotjs_spi_platform_data_t* platform_data = spi->platform_data;

  const char* device_path = iotjs_string_data(&platform_data->device);
  if (!iotjs_systemio_check_path(device_path)) {
    return false;
  }

  // Open file
  const iotjs_environment_t* env = iotjs_environment_get();
  uv_loop_t* loop = iotjs_environment_loop(env);

  uv_fs_t open_req;
  int result = uv_fs_open(loop, &open_req, device_path, O_RDONLY, 0666, NULL);
  uv_fs_req_cleanup(&open_req);
  if (result < 0) {
    return false;
  }
  platform_data->device_fd = open_req.result;

  // Set options
  if (!spi_set_configuration(spi)) {
    return false;
  }
  return true;
}
