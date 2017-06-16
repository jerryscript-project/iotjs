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


#ifndef IOTJS_MODULE_SPI_H
#define IOTJS_MODULE_SPI_H

#include "iotjs_def.h"
#include "iotjs_module_buffer.h"
#include "iotjs_objectwrap.h"
#include "iotjs_reqwrap.h"

#if defined(__TIZENRT__)
#include <iotbus_spi.h>
#include <tinyara/config.h>
#endif


#if defined(__NUTTX__)
#include <nuttx/spi/spi.h>
#endif

typedef enum {
  kSpiOpOpen,
  kSpiOpTransferArray,
  kSpiOpTransferBuffer,
  kSpiOpClose,
} SpiOp;

typedef enum {
  kSpiMode_0,
  kSpiMode_1,
  kSpiMode_2,
  kSpiMode_3,
} SpiMode;

typedef enum {
  kSpiCsNone,
  kSpiCsHigh,
} SpiChipSelect;

typedef enum { kSpiOrderMsb, kSpiOrderLsb } SpiOrder;


typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;
#if defined(__linux__)
  iotjs_string_t device;
  int32_t device_fd;
#elif defined(__NUTTX__)
  int bus;
  uint32_t cs_chip;
  struct spi_dev_s* spi_dev;
#elif defined(__TIZENRT__)
  unsigned int bus;
  iotbus_spi_context_h hSpi;
#endif
  SpiMode mode;
  SpiChipSelect chip_select;
  SpiOrder bit_order;
  uint8_t bits_per_word;
  uint16_t delay;
  uint32_t max_speed;
  bool loopback;

  // SPI buffer
  char* tx_buf_data;
  char* rx_buf_data;
  uint8_t buf_len;

} IOTJS_VALIDATED_STRUCT(iotjs_spi_t);


typedef struct {
  bool result;
  SpiOp op;
} iotjs_spi_reqdata_t;


typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_work_t req;
  iotjs_spi_reqdata_t req_data;
  iotjs_spi_t* spi_instance;
} IOTJS_VALIDATED_STRUCT(iotjs_spi_reqwrap_t);


#define THIS iotjs_spi_reqwrap_t* spi_reqwrap

iotjs_spi_reqwrap_t* iotjs_spi_reqwrap_from_request(uv_work_t* req);
iotjs_spi_reqdata_t* iotjs_spi_reqwrap_data(THIS);

iotjs_spi_t* iotjs_spi_instance_from_reqwrap(THIS);

#undef THIS


#define SPI_WORKER_INIT                                                     \
  iotjs_spi_reqwrap_t* req_wrap = iotjs_spi_reqwrap_from_request(work_req); \
  iotjs_spi_reqdata_t* req_data = iotjs_spi_reqwrap_data(req_wrap);         \
  iotjs_spi_t* spi = iotjs_spi_instance_from_reqwrap(req_wrap);


bool iotjs_spi_transfer(iotjs_spi_t* spi);
bool iotjs_spi_close(iotjs_spi_t* spi);

void iotjs_spi_open_worker(uv_work_t* work_req);


#endif /* IOTJS_MODULE_SPI_H */
