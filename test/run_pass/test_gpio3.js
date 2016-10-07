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
  @SKIP
    skip this test for there is no way to automatically confirm
    correctness of gpio behavior on host machine.
*/

var assert = require('assert');
var gpio = require("gpio");

// mode : pullup (on-low, off-high), pulldn, float, pushpull, opendrain
var INPUT_PIN_NUMBER = 20,
    INPUT_PIN_MODE = "pulldn",
    OUTPUT_PORT_NUMBER = 0,
    OUTPUT_PORT_MODE = "pulldn";

var SWITCH_ON = (INPUT_PIN_NUMBER === "pulldn" ? true : false),
    SWITCH_OFF = !SWITCH_ON,
    LED_ON = (OUTPUT_PORT_MODE === "pulldn" ? 0xFF : 0x00),
    LED_OFF = ~LED_ON;

var SWITCH_SET_MASK = 0x00,
    LED_SET_MASK = 0x01,
    GPIO_SET_MASK = 0x03;

var settingMask = 0x00;

gpio.initialize();

gpio.on('initialize', function() {
  console.log('GPIO initialized');

  gpio.setPin(INPUT_PIN_NUMBER, "in", INPUT_PIN_MODE);
  gpio.setPort(OUTPUT_PORT_NUMBER, "out", OUTPUT_PORT_MODE);
});

gpio.on('setPin', function(pin, dir, mode) {
  console.log('setpin complete - pin: %d, direction: %s, mode: %s',
              pin, dir, mode);

  if (pin === INPUT_PIN_NUMBER) {
    settingMask |= (1 << SWITCH_SET_MASK);
  }
});

gpio.on('setPort', function(port, dir, mode) {
  console.log('setport complete - port: %d, direction: %s, mode: %s',
              port, dir, mode);

  if (port === OUTPUT_PORT_NUMBER) {
    settingMask |= (1 << LED_SET_MASK);
  }
  gpio.writePort(OUTPUT_PORT_NUMBER, LED_OFF);
});

gpio.on('release', function() {
  console.log('released');
});

gpio.on('error', function(err) {
  console.log(err);
});

var loop = setInterval(function() {
  if ((settingMask & 0xFF) !== GPIO_SET_MASK) {
    return;
  }

  gpio.readPin(INPUT_PIN_NUMBER, function(err, value) {
    if (err) {
      clearInterval(loop);
    }

    if (value === SWITCH_ON) {
      gpio.writePort(OUTPUT_PORT_NUMBER, LED_ON)
    } else {
      gpio.writePort(OUTPUT_PORT_NUMBER, LED_OFF);
    }
  });
}, 500);
