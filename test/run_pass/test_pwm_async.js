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

var assert = require('assert');
var Pwm = require('pwm');
var pin = require('tools/systemio_common').pin;
var checkError = require('tools/systemio_common').checkError;

var pwm = new Pwm();

var dutyCycles = [0.25, 0.5, 0.75],
    frequencies = [1, 10, 30],
    periodInit = false,
    dutyCycleInit = false;

var configuration = {
  period: 0.001,  // 1kHz
  dutyCycle: dutyCycles[0],
  pin: pin.pwm1
};

function initPwm(pwm) {
  pwm.setPeriod(0.001, function(err) {
    checkError(err);
    periodInit = true;
  });
  pwm.setDutyCycle(0.5, function(err) {
    checkError(err);
    dutyCycleInit = true;
  });
}

var pwm0 = null;
pwm0 = pwm.open(configuration, function (err) {
  console.log('PWM initialized');
  checkError(err);

  pwm0.setEnable(1, checkError);
  dutyCycleTest();
});

function dutyCycleTest() {
  var loopCnt = 0;

  var loop = setInterval(function() {
    if (pwm0 === null) {
      return;
    }

    if (loopCnt >= dutyCycles.length) {
      clearInterval(loop);
      initPwm(pwm0);
      console.log('PWM duty-cycle test complete');
      frequencyTest();
      return;
    }
    console.log("dutycycle(%d)", dutyCycles[loopCnt]);
    pwm0.setDutyCycle(dutyCycles[loopCnt++], checkError);
  }, 1000);
}

function frequencyTest() {
  var loopCnt = 0;

  var loop = setInterval(function() {
    if (!dutyCycleInit || !periodInit) {
      return;
    }

    if (loopCnt >= frequencies.length) {
      clearInterval(loop);
      pwm0.setEnable(0, function(err) {
        checkError(err);
        pwm0.close(function(err) {
          checkError(err);
          console.log('PWM frequency test complete');
        });
      });
      return;
    }

    console.log("frequency(%d)", frequencies[loopCnt]);
    pwm0.setFrequency(frequencies[loopCnt++], checkError);
  }, 2000);
}
