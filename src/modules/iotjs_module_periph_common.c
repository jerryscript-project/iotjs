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

#include "iotjs_module_periph_common.h"
#include "iotjs_module_adc.h"
#include "iotjs_module_gpio.h"
#include "iotjs_module_i2c.h"
#include "iotjs_module_pwm.h"
#include "iotjs_module_spi.h"
#include "iotjs_module_uart.h"

const char* iotjs_periph_error_str(uint8_t op) {
  switch (op) {
#if ENABLE_MODULE_ADC
    case kAdcOpClose:
      return "Close error, cannot close ADC";
    case kAdcOpOpen:
      return "Open error, cannot open ADC";
    case kAdcOpRead:
      return "Read error, cannot read ADC";
#endif /* ENABLE_MODULE_ADC */
#if ENABLE_MODULE_GPIO
    case kGpioOpOpen:
      return "Open error, cannot open GPIO";
    case kGpioOpWrite:
      return "Write error, cannot write GPIO";
    case kGpioOpRead:
      return "Read error, cannot read GPIO";
    case kGpioOpClose:
      return "Close error, cannot close GPIO";
#endif /* ENABLE_MODULE_GPIO */
#if ENABLE_MODULE_I2C
    case kI2cOpOpen:
      return "Open error, cannot open I2C";
    case kI2cOpWrite:
      return "Write error, cannot write I2C";
    case kI2cOpRead:
      return "Read error, cannot read I2C";
    case kI2cOpClose:
      return "Close error, cannot close I2C";
#endif /* ENABLE_MODULE_I2C */
#if ENABLE_MODULE_PWM
    case kPwmOpClose:
      return "Cannot close PWM device";
    case kPwmOpOpen:
      return "Failed to open PWM device";
    case kPwmOpSetDutyCycle:
      return "Failed to set duty-cycle";
    case kPwmOpSetEnable:
      return "Failed to set enable";
    case kPwmOpSetFrequency:
      return "Failed to set frequency";
    case kPwmOpSetPeriod:
      return "Failed to set period";
#endif /* ENABLE_MODULE_PWM */
#if ENABLE_MODULE_SPI
    case kSpiOpClose:
      return "Close error, cannot close SPI";
    case kSpiOpOpen:
      return "Open error, cannot open SPI";
    case kSpiOpTransferArray:
    case kSpiOpTransferBuffer:
      return "Transfer error, cannot transfer from SPI device";
#endif /* ENABLE_MODULE_SPI */
#if ENABLE_MODULE_UART
    case kUartOpClose:
      return "Close error, failed to close UART device";
    case kUartOpOpen:
      return "Open error, failed to open UART device";
    case kUartOpWrite:
      return "Write error, cannot write to UART device";
#endif /* ENABLE_MODULE_UART */
    default:
      return "Unknown error";
  }
}

static void after_worker(uv_work_t* work_req, int status) {
  iotjs_periph_reqwrap_t* reqwrap =
      (iotjs_periph_reqwrap_t*)iotjs_reqwrap_from_request((uv_req_t*)work_req);

  size_t jargc = 0;
  jerry_value_t jargs[2] = { 0 };

  if (status) {
    jargs[jargc++] = iotjs_jval_create_error_without_error_flag("System error");
  } else {
    if (!reqwrap->result) {
      jargs[jargc++] = iotjs_jval_create_error_without_error_flag(
          iotjs_periph_error_str(reqwrap->op));
    } else {
      jargs[jargc++] = jerry_create_null();
      switch (reqwrap->op) {
        case kAdcOpClose:
        case kAdcOpOpen:
        case kGpioOpClose:
        case kGpioOpOpen:
        case kGpioOpWrite:
        case kI2cOpClose:
        case kI2cOpOpen:
        case kI2cOpWrite:
        case kSpiOpClose:
        case kSpiOpOpen:
        case kPwmOpClose:
        case kPwmOpOpen:
        case kPwmOpSetDutyCycle:
        case kPwmOpSetEnable:
        case kPwmOpSetFrequency:
        case kPwmOpSetPeriod:
        case kUartOpClose:
        case kUartOpOpen:
        case kUartOpWrite: {
          break;
        }
        case kAdcOpRead: {
#if ENABLE_MODULE_ADC
          iotjs_adc_t* adc = (iotjs_adc_t*)reqwrap->data;
          jargs[jargc++] = jerry_create_number(adc->value);
#endif /* ENABLE_MODULE_ADC */
          break;
        }
        case kGpioOpRead: {
#if ENABLE_MODULE_GPIO
          iotjs_gpio_t* gpio = (iotjs_gpio_t*)reqwrap->data;
          jargs[jargc++] = jerry_create_boolean(gpio->value);
#endif /* ENABLE_MODULE_GPIO */
          break;
        }
        case kI2cOpRead: {
#if ENABLE_MODULE_I2C
          iotjs_i2c_t* i2c = (iotjs_i2c_t*)reqwrap->data;
          jargs[jargc++] =
              iotjs_jval_create_byte_array(i2c->buf_len, i2c->buf_data);
          IOTJS_RELEASE(i2c->buf_data);
#endif /* ENABLE_MODULE_I2C */
          break;
        }
        case kSpiOpTransferArray:
        case kSpiOpTransferBuffer: {
#if ENABLE_MODULE_SPI
          iotjs_spi_t* spi = (iotjs_spi_t*)reqwrap->data;
          // Append read data
          jargs[jargc++] =
              iotjs_jval_create_byte_array(spi->buf_len, spi->rx_buf_data);
          IOTJS_RELEASE(spi->rx_buf_data);
#endif /* ENABLE_MODULE_SPI */
          break;
        }
        default: {
          IOTJS_ASSERT(!"Unreachable");
          break;
        }
      }
    }
#if ENABLE_MODULE_SPI
    if (reqwrap->op == kSpiOpTransferArray) {
      iotjs_spi_t* spi = (iotjs_spi_t*)reqwrap->data;
      IOTJS_RELEASE(spi->tx_buf_data);
    }
#endif /* ENABLE_MODULE_SPI */
#if ENABLE_MODULE_UART
    if (reqwrap->op == kUartOpWrite) {
      iotjs_uart_t* uart = (iotjs_uart_t*)reqwrap->data;
      iotjs_string_destroy(&uart->buf_data);
    }
#endif /* ENABLE_MODULE_UART */
  }

  jerry_value_t jcallback = iotjs_reqwrap_jcallback(&reqwrap->reqwrap);
  if (jerry_value_is_function(jcallback)) {
    iotjs_invoke_callback(jcallback, jerry_create_undefined(), jargs, jargc);
  }

  for (size_t i = 0; i < jargc; i++) {
    jerry_release_value(jargs[i]);
  }

  iotjs_reqwrap_destroy(&reqwrap->reqwrap);
  IOTJS_RELEASE(reqwrap);
}

static iotjs_periph_reqwrap_t* reqwrap_create(const jerry_value_t jcallback,
                                              void* data, uint8_t op) {
  iotjs_periph_reqwrap_t* reqwrap = IOTJS_ALLOC(iotjs_periph_reqwrap_t);
  iotjs_reqwrap_initialize((iotjs_reqwrap_t*)reqwrap, jcallback,
                           (uv_req_t*)&reqwrap->req);
  reqwrap->op = op;
  reqwrap->data = data;
  return (iotjs_periph_reqwrap_t*)reqwrap;
}

void iotjs_periph_call_async(void* data, jerry_value_t jcallback, uint8_t op,
                             uv_work_cb worker) {
  uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());
  iotjs_periph_reqwrap_t* req_wrap = reqwrap_create(jcallback, data, op);
  uv_queue_work(loop, &req_wrap->req, worker, after_worker);
}
