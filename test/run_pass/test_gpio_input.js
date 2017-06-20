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
var Gpio = require('gpio');
var gpio = new Gpio();

var ledGpio = null, switchGpio = null;
var ledPin, switchPin, ledMode;

var SWITCH_ON = false,
    LED_ON = true,
    LED_OFF = false;

var loopCnt = 0;

if (process.platform === 'linux') {
  ledPin = 20;
  switchPin = 13;
  ledMode = gpio.MODE.NONE;
} else if (process.platform === 'nuttx') {
  var pin = require('stm32f4dis').pin;
  ledPin = pin.PA10;
  switchPin = pin.PA15;
  ledMode = gpio.MODE.PUSHPULL;
} else if(process.platform === 'tizenrt') {
  ledPin = 41;
  switchPin = 39;
  ledMode = gpio.MODE.NONE;
} else {
  assert.fail();
}

ledGpio = gpio.open({
  pin: ledPin,
  direction: gpio.DIRECTION.OUT,
  mode: ledMode
}, function() {
  this.writeSync(LED_OFF);
});

switchGpio = gpio.open({
  pin: switchPin,
  direction: gpio.DIRECTION.IN
});

var loop = setInterval(function() {
  if (!ledGpio || !switchGpio) {
    return;
  }

  if ((++loopCnt) == 10) {
    clearInterval(loop);
    ledGpio.closeSync();
    switchGpio.closeSync();
    ledGpio = switchGpio = null;
    console.log('finish test');
    return;
  }

  switchGpio.read(function(err, value) {
    if (err) {
      clearInterval(loop);
      assert.fail();
    }

    if (value === SWITCH_ON) {
      console.log('led on');
      ledGpio.writeSync(LED_ON);
    } else {
      ledGpio.writeSync(LED_OFF);
    }
  });
}, 500);
