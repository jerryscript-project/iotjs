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
    LED_PIN_MODE = "pushpull";

var LED_ON = true,
    LED_OFF = false;

var count = 0;

gpio.initialize();

gpio.on('initialize', function() {
  console.log('GPIO initialized');

  gpio.open(LED_PIN_NUMBER, "out", LED_PIN_MODE);
});

gpio.on('open', function(pin, dir, mode) {
  console.log('open complete - pin: %d, direction: %s, mode: %s',
              pin, dir, mode);

  if (pin === LED_PIN_NUMBER) {
    gpio.write(pin, LED_ON, function(err) {
      console.log('write complete - pin: %d, value: %d', pin, LED_ON);
    });

    setTimeout(function() {
      gpio.write(pin, LED_OFF, function(err) {
        console.log('write complete - pin: %d, value: %d', pin, LED_OFF);
      });
    }, 2000);
  }
});

gpio.on('write', function(pin, writeValue) {
  console.log('write event - pin: %d, value: %d', pin, writeValue);

  gpio.read(pin, function(err, readValue) {
    assert.equal(writeValue, readValue);
    console.log('read complete - pin: %d, value: %d', pin, readValue);
  });
});

gpio.on('read', function(pin, value) {
  console.log('read event - pin: %d, value: %d', pin, value);

  // read function is called two times.
  if ((++count) === 2) {
    // release gpio after 1000ms
    setTimeout(function() {
      gpio.release();
    }, 1000);
  }
});

gpio.on('release', function() {
  console.log('released');
});

gpio.on('error', function(err) {
  console.log(err);
});
