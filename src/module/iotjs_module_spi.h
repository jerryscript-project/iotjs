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


#define SPI_DEVICE_NUMBER_SHIFT 0
#define SPI_DEVICE_NUMBER_MASK 3
#define SPI_DEVICE_NUMBER(n) ((n) << SPI_DEVICE_NUMBER_SHIFT)
#define SPI_GET_DEVICE_NUMBER(n) \
  (((n) >> SPI_DEVICE_NUMBER_SHIFT) & SPI_DEVICE_NUMBER_MASK)

#define SPI_CS_NUMBER_SHIFT 5
#define SPI_CS_NUMBER_MASK 3
#define SPI_CS_NUMBER(n) ((n) << SPI_CS_NUMBER_SHIFT)
#define SPI_GET_CS_NUMBER(n) (((n) >> SPI_CS_NUMBER_SHIFT) & SPI_CS_NUMBER_MASK)


typedef enum {
  kSpiOptionMode = 0,
  kSpiOptionChipSelect = 2,
  kSpiOptionMaxSpeed = 4,
  kSpiOptionBitsPerWord = 8,
  kSpiOptionBitOrder = 16,
  kSpiOptionLoopback = 32,
} SpiOption;

typedef enum {
  kSpiOpExport,
  kSpiOpOpen,
  kSpiOpTransfer,
  kSpiOpUnexport,
} SpiOp;

typedef enum {
  kSpiErrOk = 0,
  kSpiErrExport = -1,
  kSpiErrTransfer = -2,
  kSpiErrUnexport = -3,
} SpiError;

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

  // SPI setting options;
  int32_t device_fd;
  SpiMode mode;
  SpiChipSelect chip_select;
  SpiOrder order;
  uint8_t bits_per_word;
  uint8_t bit_order;
  uint16_t delay;
  uint32_t max_speed;
  bool loopback;

  // SPI buffer
  char* tx_buf_data;
  char* rx_buf_data;
  uint8_t buf_len;

} IOTJS_VALIDATED_STRUCT(iotjs_spi_t);

typedef struct {
  int32_t pin;
  uint32_t selected_option;
  iotjs_spi_t* spi_instance;

  SpiError result;
  SpiOp op;
} iotjs_spi_reqdata_t;


typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_work_t req;
  iotjs_spi_reqdata_t req_data;
  const iotjs_spi_t* spi_data;
} IOTJS_VALIDATED_STRUCT(iotjs_spi_reqwrap_t);


#define THIS iotjs_spi_reqwrap_t* spi_reqwrap
iotjs_spi_reqwrap_t* iotjs_spi_reqwrap_create(const iotjs_jval_t* jcallback,
                                              SpiOp op);
void iotjs_spi_reqwrap_dispatched(THIS);
uv_work_t* iotjs_spi_reqwrap_req(THIS);
const iotjs_jval_t* iotjs_spi_reqwrap_jcallback(THIS);
iotjs_spi_reqwrap_t* iotjs_spi_reqwrap_from_request(uv_work_t* req);
iotjs_spi_reqdata_t* iotjs_spi_reqwrap_data(THIS);
#undef THIS


iotjs_spi_t* iotjs_spi_create(const iotjs_jval_t* jspi);
iotjs_spi_t* iotjs_spi_get_instance(const iotjs_jval_t* jspi);

void iotjs_spi_set_device_fd(iotjs_spi_t* spi, int32_t fd);

void iotjs_spi_export_worker(uv_work_t* work_req);
void iotjs_spi_transfer_worker(uv_work_t* work_req);
bool iotjs_spi_set_options(iotjs_spi_t* spi, uint32_t selected_option);
void iotjs_spi_unexport_worker(uv_work_t* work_req);


#endif /* IOTJS_MODULE_SPI_H */
