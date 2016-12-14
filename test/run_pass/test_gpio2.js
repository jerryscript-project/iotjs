/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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
var gpio = require("gpio");

// mode : pullup (on-low, off-high), pulldn, float, pushpull, opendrain
var LED_PIN_NUMBER = 0,
    LED_PIN_MODE = "pushpull",
    SWITCH_PIN_NUMBER = 14,
    SWITCH_PIN_MODE = "pulldn";

var SWITCH_ON = true,
    LED_ON = true,
    LED_OFF = false;

var openSwitch = false;
var loopCnt = 0;

gpio.initialize();

gpio.on('initialize', function() {
  console.log('GPIO initialized');

  gpio.open(LED_PIN_NUMBER, "out", LED_PIN_MODE);
  gpio.open(SWITCH_PIN_NUMBER, "in", SWITCH_PIN_MODE);
});

gpio.on('open', function(pin, dir, mode) {
  console.log('open complete - pin: %d, direction: %s, mode: %s',
              pin, dir, mode);

  if (pin === SWITCH_PIN_NUMBER) {
    openSwitch = true;
  }
  else if (pin === LED_PIN_NUMBER) {
    gpio.write(pin, LED_OFF);
  }
});

gpio.on('release', function() {
  console.log('released');
});

gpio.on('error', function(err) {
  console.log(err);
});

var loop = setInterval(function() {
  if (!openSwitch) {
    return;
  }

  if ((++loopCnt) == 10) {
    // release gpio after 1000ms
    setTimeout(function() {
      gpio.release();
    }, 1000);

    clearInterval(loop);
  }

  gpio.read(SWITCH_PIN_NUMBER, function(err, value) {
    if (err) {
      clearInterval(loop);
    }

    if (value === SWITCH_ON) {
      gpio.write(LED_PIN_NUMBER, LED_ON)
    } else {
      gpio.write(LED_PIN_NUMBER, LED_OFF);
    }
  });
}, 500);
