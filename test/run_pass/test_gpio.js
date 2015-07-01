/* Copyright 2015 Samsung Electronics Co., Ltd.
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

/*
  @TIMEOUT=10
*/

var assert = require('assert');
var gpio = require('gpio');

var gpioSequence = '';

gpio.initialize(function(err) {
  assert(err>=0, "failed to initailize");
  gpioSequence += 'I';
});

gpio.pinmode(0x11, function(err) {
  assert(err>=0, "failed to pinmode");
  gpioSequence += 'A';
});

gpio.pinmode(0x22, function(err) {
  assert(err>=0, "failed to pinmode");
  gpioSequence += 'B';
});

gpio.write(0x01, 0xff, function(err) {
  assert(err>=0, "failed to write");
  gpioSequence += 'C';
});

gpio.read(0x02, function(err, value) {
  assert(err>=0, "failed to read");
  gpioSequence += 'D';
});

gpio.release();

process.on('exit', function(code) {
  assert.equal(gpioSequence, 'IABCD');
});
