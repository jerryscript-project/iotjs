/* Copyright 2019-present Samsung Electronics Co., Ltd. and other contributors
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

// ------ Test API existance
assert.equal(typeof pwm.open, 'function',
             'pwm does not provide \'open\' function');
assert.equal(typeof pwm.openSync, 'function',
             'pwm does not provide \'openSync\' function');

function check_pwmpin(pwmpin) {
  assert.equal(typeof pwmpin.setPeriod, 'function',
               '\'pwmpin\' does not provide \'setPeriod\' function');
  assert.equal(typeof pwmpin.setPeriodSync, 'function',
               '\'pwmpin\' does not provide \'setPeriodSync\' function');
  assert.equal(typeof pwmpin.setFrequency, 'function',
               '\'pwmpin\' does not provide \'setFrequency\' function');
  assert.equal(typeof pwmpin.setFrequencySync, 'function',
               '\'pwmpin\' does not provide \'setFrequencySync\' function');
  assert.equal(typeof pwmpin.setDutyCycle, 'function',
               '\'pwmpin\' does not provide \'setDutyCycle\' function');
  assert.equal(typeof pwmpin.setDutyCycleSync, 'function',
               '\'pwmpin\' does not provide \'setDutyCycleSync\' function');
  assert.equal(typeof pwmpin.setEnable, 'function',
               '\'pwmpin\' does not provide \'setEnable\' function');
  assert.equal(typeof pwmpin.setEnableSync, 'function',
               '\'pwmpin\' does not provide \'setEnableSync\' function');
  assert.equal(typeof pwmpin.close, 'function',
               '\'pwmpin\' does not provide \'close\' function');
  assert.equal(typeof pwmpin.closeSync, 'function',
               '\'pwmpin\' does not provide \'closeSync\' function');
}

// ------ Test synchronous PWM Pin opening
assert.throws(
  function() {
    pwm.openSync({period: 0.1,
                  dutyCycle: 0.5});
  },
  TypeError
);

assert.throws(
  function() {
    pwm.openSync({pin: 0,
                  period: 0.1,
                  dutyCycle: 1.1});
  },
  RangeError
);

assert.throws(
  function() {
    pwm.openSync({pin: 0,
                  period: -0.1,
                  dutyCycle: 0.5});
  },
  RangeError
);

assert.throws(
  function() {
    pwm.openSync({});
  },
  TypeError
);

var config = {
  pin: 0,
  period: 0.1,
  dutyCycle: 0.5
}

var pwmpin = pwm.openSync(config);
check_pwmpin(pwmpin);

assert.doesNotThrow(
  function() {
    pwmpin.setPeriodSync(1);
  }
);

assert.throws(
  function() {
    pwmpin.setPeriodSync();
  },
  Error
);

assert.throws(
  function() {
    pwmpin.setPeriodSync(null);
  },
  Error
);

assert.doesNotThrow(
  function() {
    pwmpin.setFrequencySync(1);
  }
);

assert.throws(
  function() {
    pwmpin.setFrequencySync();
  },
  Error
);

assert.throws(
  function() {
    pwmpin.setFrequencySync(null);
  },
  Error
);

assert.doesNotThrow(
  function() {
    pwmpin.setDutyCycleSync(1);
  }
);

assert.throws(
  function() {
    pwmpin.setDutyCycleSync();
  },
  Error
);

assert.throws(
  function() {
    pwmpin.setDutyCycleSync(null);
  },
  Error
);

assert.doesNotThrow(
  function() {
    pwmpin.setEnableSync(false);
  }
);

assert.throws(
  function() {
    pwmpin.setEnableSync();
  },
  Error
);

assert.throws(
  function() {
    pwmpin.setEnableSync(null);
  },
  Error
);

pwmpin.closeSync();

// ------ Test asynchronous PWM Pin opening
pwm.open(config, function(open_err, async_pwmpin) {
  assert.equal(open_err, null);
  open_cb1 = true;

  assert.throws(
    function() {
      pwmpin.setPeriod();
    },
    Error
  );

  assert.throws(
    function() {
      pwmpin.setPeriod(null);
    },
    Error
  );

  assert.throws(
    function() {
      pwmpin.setFrequency();
    },
    Error
  );

  assert.throws(
    function() {
      pwmpin.setFrequency(null);
    },
    Error
  );

  assert.throws(
    function() {
      pwmpin.setDutyCycle();
    },
    Error
  );

  assert.throws(
    function() {
      pwmpin.setDutyCycle(null);
    },
    Error
  );

  assert.throws(
    function() {
      pwmpin.setEnableSync();
    },
    Error
  );

  assert.throws(
    function() {
      pwmpin.setEnableSync(null);
    },
    Error
  );

  async_pwmpin.setPeriod(1, function(period_err) {
    assert.equal(period_err, null);
    period_cb1 = true;

    async_pwmpin.setFrequency(1, function(frequency_err, res) {
      assert.equal(frequency_err, null);
      frequency_cb1 = true;

      async_pwmpin.setDutyCycle(1, function(dutycycle_err, res) {
        assert.equal(dutycycle_err, null);
        dutycycle_cb1 = true;

        async_pwmpin.setEnable(true, function(enable_err, res) {
          assert.equal(enable_err, null);
          enable_cb1 = true;

          async_pwmpin.close(function(close_err) {
            assert.equal(close_err, null);
            close_cb1 = true;
          });
        });
      });
    });
  });
});

process.on('exit', function(code) {
  if (code === 0) {
    assert.assert(open_cb1, 'callback of \'pwm.open\' was not called');
    assert.assert(close_cb1, 'callback of \'pwm.close\' was not called');

    assert.assert(period_cb1,
                  'callback of \'pwmpin.setPeriod\' was not called');
    assert.assert(frequency_cb1,
                  'callback of \'pwmpin.setFrequency\' was not called');
    assert.assert(dutycycle_cb1,
                  'callback of \'pwmpin.setDutyCycle\' was not called');
    assert.assert(enable_cb1,
                  'callback of \'pwmpin.setEnable\' was not called');
  }
});
