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

var assert = require('assert');

var pin = {};

if (process.platform === 'linux') {
  pin.led = 20;
  pin.switch = 13;
  pin.pwm1 = 0;
  pin.i2c1 = '/dev/i2c-1';
  pin.spi1 = '/dev/spidev0.0';
  pin.uart1 = '/dev/ttyS0';
} else if (process.platform === 'tizen') {
  if (process.iotjs.board === 'rpi3') {
    pin.led = 20;
    pin.switch = 13;
    pin.spi1 = 0;
    pin.uart1 = 0;
  } else if (process.iotjs.board === 'artik530') {
    pin.led = 128;
    pin.switch = 27;
    pin.pwm1 = 2;
    pin.spi1 = 2;
    pin.uart1 = 4;
  }
  pin.i2c1 = 1;
} else if (process.platform === 'nuttx') {
  var board = process.iotjs.board;
  var stm32_pin = require(board).pin;
  pin.led = stm32_pin.PA10;
  pin.switch = stm32_pin.PA15;
  pin.pwm1 = stm32_pin.PWM1.CH1_1;
  pin.i2c1 = 1;
  pin.spi1 = 1;
  pin.uart1 = '/dev/ttyS1';
} else if (process.platform === 'tizenrt') {
  pin.led = 41;
  pin.switch = 39;
  pin.pwm1 = 0;
  pin.i2c1 = 1;
  pin.spi1 = 1;
  pin.uart1 = '/dev/ttyS1';
} else {
  throw new Error('Unsupported platform');
}

function checkError(err) {
  if (err) {
    console.log('Have an error: ' + err.message);
    assert.fail();
  }
}

exports.pin = pin;
exports.checkError = checkError;
