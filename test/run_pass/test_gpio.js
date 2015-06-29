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

var gpio_map = gpio.readconfig("../resources/test_gpio_cfg.js");

var gpioSequence = '';

gpio.initialize(gpio_map);
gpio.setpin('OUT1', "out", function(err) {
  assert(err>=0, "failed to setpin");
  gpioSequence += 'A';
});
gpio.setpin('INP1', "in", function(err) {
  assert(err>=0, "failed to setpin");
  gpioSequence += 'B';
});

gpio.write("OUT1", 0, function(err) {
  assert(err>=0, "failed to write");
  gpioSequence += 'C';
});
gpio.read("INP1", function(err, value) {
  assert(err>=0, "failed to read");
  gpioSequence += 'D';
});

gpio.resetpin('OUT1', function(err) {
  assert(err>=0, "failed to resetpin");
  gpioSequence += 'E';
});
gpio.resetpin('INP1', function(err) {
  assert(err>=0, "failed to resetpin");
  gpioSequence += 'F';
});
gpio.release();

process.on('exit', function(code) {
  assert.equal(gpioSequence, 'ABCDEF');
});
