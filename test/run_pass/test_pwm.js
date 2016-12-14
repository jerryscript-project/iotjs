/* Copyright 2016-present Samsung Electronics Co., Ltd. and other contributors
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
var pwm = require('pwm');
var option = {
  period: 1000000,
  dutyCycle: 25
};

var callback = function (err) {
  if (err) {
    assert.fail();
  }
};

var pwm0 = new pwm(0, option, function (err) {
  console.log('PWM initialized');

  if (err) {
    assert.fail();
  }

  pwm0.setEnable(1, callback);

  test1();
});

function test1() {
  var maxCnt = 3,
    loopCnt = maxCnt,
    dutyCycle;

  console.log('test1 start');
  var test1Loop = setInterval(function () {
    loopCnt--;

    dutyCycle = (option.dutyCycle * (maxCnt - loopCnt));
    pwm0.setDutyCycle(dutyCycle, callback);

    console.log('Duty-Cycle : %d', dutyCycle);

    if (loopCnt <= 0) {
      clearInterval(test1Loop);
      console.log('test1 complete');
      test2();
    }
  }, 1000);
}

function test2() {
  var maxCnt = 3,
    loopCnt = maxCnt,
    period;

  option.period = 500000;
  option.dutyCycle = 25;
  pwm0.setDutyCycle(option.dutyCycle, callback);

  console.log('test2 start');
  var test2Loop = setInterval(function () {
    loopCnt--;

    period = (option.period * (maxCnt - loopCnt));
    pwm0.setPeriod(period, callback);

    console.log('Period : %d', period);

    if (loopCnt <= 0) {
      clearInterval(test2Loop);
      console.log('test2 complete');
    }
  }, 1000);
}
