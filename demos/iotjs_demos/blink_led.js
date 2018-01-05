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

// A simple program to make a LED blink on and off using IoTJS

var Gpio = require('gpio');
var pin = {};

var gpio = new Gpio();

console.log("Starting LED blink program...");

var gpio_led = gpio.open({
  pin: 5,
  direction: gpio.DIRECTION.OUT
}, function(err) {
  if (!err) {
    gpio_led.writeSync(true);

    var interval = setInterval(function() {
      gpio_led.read(function(err, value) {
        if (!err) {
          console.log("read value:%d", value);
          gpio_led.write(!value);
        } else {
          console.log("Something went wrong with gpio_led.read");
          console.error(err);
          clearInterval(interval);
        }
      });
    }, 1000);
  } else {
	  console.log("Something went wrong! :-( ...");
	  console.error(err);
  }
});


