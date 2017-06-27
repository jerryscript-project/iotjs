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

var pin = {};

if (process.platform === 'linux') {
  pin.led1 = 20;
} else if (process.platform === 'nuttx') {
  var stm32_pin = require('stm32f4dis').pin;
  pin.led1 = stm32_pin.PA10;
} else {
  throw new Error('Unsupported platform');
}

exports.pin = pin;
