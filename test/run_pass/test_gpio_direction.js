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

var gpio = require('gpio');
var pin = require('tools/systemio_common').pin;

var gpioPin = gpio.openSync({
  pin: pin.led,
  direction: gpio.DIRECTION.OUT,
});

console.log('GPIO input test. Press the button.');
gpioPin.setDirectionSync(gpio.DIRECTION.IN);
var loop = setInterval(function() {
  var value = gpioPin.readSync();
  console.log('GpioPin value:', value);
}, 500);

setTimeout(function() {
  clearInterval(loop);

  console.log('GPIO output test. Led is on for 5000ms.');
  gpioPin.setDirectionSync(gpio.DIRECTION.OUT);
  gpioPin.writeSync(1);
  setTimeout(function() {
    gpioPin.writeSync(0);
  }, 5000);
}, 5000);

