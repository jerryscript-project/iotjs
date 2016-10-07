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
var LED_PIN_NUMBER = 0,
    LED_PIN_MODE = "pulldn";

var LED_ON = true,
    LED_OFF = false;

gpio.initialize();

gpio.on('initialize', function() {
  console.log('GPIO initialized');

  gpio.setPin(LED_PIN_NUMBER, "out", LED_PIN_MODE);
});

gpio.on('setPin', function(pin, dir, mode) {
  console.log('setpin complete - pin: %d, direction: %s, mode: %s',
              pin, dir, mode);

  if (pin === LED_PIN_NUMBER) {
    gpio.writePin(pin, LED_ON, function(err) {
      console.log('writePin complete - pin: %d, value: %d', pin, LED_ON);
    });

    setTimeout(function() {
      gpio.writePin(pin, LED_OFF, function(err) {
        console.log('writePin complete - pin: %d, value: %d', pin, LED_OFF);
      });
    }, 2000);
  }
});

gpio.on('writePin', function(pin, value) {
  console.log('writePin event - pin: %d, value: %d', pin, value);

  gpio.readPin(pin, function(err, value) {
    console.log('readPin complete - pin: %d, value: %d', pin, value);
  });
});

gpio.on('readPin', function(pin, value) {
  console.log('readPin event - pin: %d, value: %d', pin, value);
});

gpio.on('release', function() {
  console.log('released');
});

gpio.on('error', function(err) {
  console.log(err);
});
