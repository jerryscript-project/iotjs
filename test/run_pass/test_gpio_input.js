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
var gpio = require('gpio');
var pin = require('tools/systemio_common').pin;
var checkError = require('tools/systemio_common').checkError;

var ledGpio = null, switchGpio = null;

var SWITCH_ON = false,
    LED_ON = true,
    LED_OFF = false;

var loopCnt = 0;

ledGpio = gpio.open({
  pin: pin.led,
  direction: gpio.DIRECTION.OUT,
}, function(err) {
  checkError(err);
  ledGpio.writeSync(LED_OFF);
});

switchGpio = gpio.openSync({
  pin: pin.switch,
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
