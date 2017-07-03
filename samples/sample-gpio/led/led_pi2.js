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

var gpio = require("gpio");

gpio.initialize();

gpio.on('initialize', function() {
  console.log('GPIO initialized');
  gpio.setPin(16, "out");
});

gpio.on('setpin', function(pin, dir, mode) {
  console.log('setpin complete - pin: %d, direction: %s, mode: %s',
              pin, dir, mode);
  gpio.writePin(pin, true);
  var interval = setInterval(function() {
    gpio.readPin(pin, function(err, value) {
      if (!err) {
        console.log("read pin:%d value:%d", pin, value);
        gpio.writePin(pin, !value);
      } else {
        clearInterval(interval);
      }
    });
  }, 1000);
});

gpio.on('error', function(err) {
  console.log(err);
});

