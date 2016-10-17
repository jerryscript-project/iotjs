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


var assert = require('assert');
var gpio = require("gpio");

// mode : pullup (on-low, off-high), pulldn, float, pushpull, opendrain
var LED_PORT_NUMBER = 0,
    LED_PORT_MODE = "pulldn";

var LED_VALUE = 0xAA;


gpio.initialize();

gpio.on('initialize', function() {
  console.log('GPIO initialized');

  gpio.setPort(LED_PORT_NUMBER, "out", LED_PORT_MODE);
});

gpio.on('setPort', function(port, dir, mode) {
  console.log('setPort complete - port: %d, direction: %s, mode: %s',
              port, dir, mode);

  if (port === LED_PORT_NUMBER) {
    gpio.writePort(port, LED_VALUE, function(err) {
      console.log('writePort complete - port: %d, value: %s',
                  port, getBinaryString(LED_VALUE));
    });

    setTimeout(function() {
      gpio.readPort(port, function(err, value) {
        gpio.writePort(port, ~value, function(err) {
          console.log('writePort complete - port: %d, value: %s',
                      port, getBinaryString(~value));
        });
      });

    }, 2000);
  }
});

gpio.on('setPin', function(port, dir, mode) {
  console.log('setPin complete - pin: %d, direction: %s, mode: %s',
              port, dir, mode);
});

gpio.on('writePort', function(port, value) {
  console.log('writePort event - port: %d, value: %s',
              port, getBinaryString(value));
});

gpio.on('readPort', function(port, value) {
  console.log('readPort event - port: %d, value: %s', port,
              getBinaryString(value));
});

gpio.on('release', function() {
  console.log('released');
});

gpio.on('error', function(err) {
  console.log(err);
});

function getBinaryString(number) {
  var binaryString = '';

  for(var i = 7; i >= 0; i--) {
    binaryString += (number & (1 << i)) ? '1' : '0';
  }

  return binaryString;
}
