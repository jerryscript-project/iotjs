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
var Pwm = require('pwm');

var pwm = new Pwm();

var configuration = {
  period: 0.001  // 1kHz
};

if (process.platform === 'linux') {
  configuration.pin = 0;
} else if (process.platform === 'nuttx') {
  configuration.pin = require('stm32f4dis').pin.PWM1.CH1_1;
} else if (process.platform === 'tizenrt') {
  configuration.pin = 0;
} else {
  assert.fail();
}

var periodOptions = {
  dutyCycle: 0.5,
  // The platform PWM is tested on (artik10/tizen 3.0) has an upper limit
  // of 75.2 Hz of PWM0 frequency.
  //values: [0.2, 0.4, 0.6, 0.8, 1]
  values: [ 0.5 ]
};

var dutyOptions = {
  period: 0.5,
  values: [ 0, 0.1, 0.5, 0.9, 1 ]
};

var testCb = function (err) {
  if (err) {
    assert.fail();
  }
};

var pwm0;
testPeriods();

function testPeriods() {
  pwm0 = pwm.open(configuration, function (err) {
    console.log('PWM initialized');

    if (err) {
      console.log('Have an error: ' + err.message);
      assert.fail();
    }

    pwm0.setEnable(1, function(err) {
      testCb(err);

      var options = periodOptions;
      console.log('PWM: period test start ');
      var idx = 0;
      var period = options.values[idx++];
      console.log("Period(%d)", period);
      pwm0.setFrequencySync(1.0 / period);
      pwm0.setDutyCycleSync(options.dutyCycle);

      var loop = setInterval(function () {
        if (idx == options.values.length) {
          clearInterval(loop);
          console.log('PWM period test complete');
          pwm0.setPeriodSync(options.values[0]);
          pwm0.setEnableSync(0);
          pwm0.closeSync();
          testDutyCycles();
        } else {
          period = options.values[idx++];
          console.log("Period(%d)", period);
          pwm0.setPeriod(period, testCb);
        }
      }, 1000);
    });
  });
}

function testDutyCycles() {
  var options = dutyOptions;

  console.log('PWM: duty cycle test start');
  pwm0 = pwm.open(configuration, function (err) {
    console.log('PWM initialized');

    if (err) {
      console.log('Have an error: ' + err.message);
      assert.fail();
    }

    pwm0.setPeriod(options.period, function(err) {
      testCb(err);

      pwm0.setEnableSync(1, testCb);
      pwm0.setFrequency(1.0 / options.period, function(err) {
        testCb(err);
        var idx = 0;
        var loop = setInterval(function () {
          console.log('Duty cycle %d', options.values[idx]);
          pwm0.setDutyCycle(options.values[idx], testCb);

          if (++idx == options.values.length) {
            clearInterval(loop);
            pwm0.setEnableSync(0);
            pwm0.close(testCb.bind(err));
            console.log('PWM duty cycle test complete');
          }
        }, 1000);
      });
    });
  });
}
