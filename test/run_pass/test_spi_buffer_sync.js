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

var assert = require('assert');
var spi = require('spi');
var pin = require('tools/systemio_common').pin;

var configuration = {
    device: pin.spi1, // for Linux
    bus: pin.spi1, // for Tizen, TizenRT and Nuttx
};

// Buffer test
var spi1 = spi.openSync(configuration);
var data = 'Hello IoTjs';
var tx = new Buffer(data);

var rx = spi1.transferSync(tx);
var len = data.length;
assert.equal(rx.length, len);
var value = '';
for (var i = 0; i < len; i++) {
  value += String.fromCharCode(rx[i]);
}
console.log(value);
assert.equal(value, data);

spi1.closeSync();
