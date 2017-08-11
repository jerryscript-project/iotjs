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

var I2C = require('i2c');
var i2c = new I2C();

var CMD_BRIGHTNESS = 0xE0;
var CMD_OSCILLATOR = 0x21;

var iotChar = [0x00, 0x00, 0x00, 0x00,
  0xCE, 0x73, 0x44, 0x22,
  0x44, 0x22, 0xCE, 0x23,
  0x00, 0x00, 0x00, 0x00];

var writeLed = function(wire, data) {
  // 0x00 is a initial signal for writing
  var buffer = [0x00].concat(data);
  wire.write(buffer);
}

var configuration = {};
configuration.address = 0x70;

if (process.platform === 'linux') {
  configuration.device = '/dev/i2c-1';
} else if (process.platform === 'nuttx') {
  configuration.device = 1;
} else {
  throw new Error('Unsupported platform');
}

var wire = i2c.open(configuration, function(err) {
  if (err) {
    throw err;
  }

  wire.write([CMD_OSCILLATOR]); // turn on oscillator
  wire.write([CMD_BRIGHTNESS | 1]); // adjust brightness

  writeLed(wire, iotChar);
});
