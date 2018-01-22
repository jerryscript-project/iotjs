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


var adc = require('adc');
var assert = require('assert');
var configuration = {};

if (process.platform === 'linux') {
  configuration.device =
    '/sys/devices/12d10000.adc/iio:device0/in_voltage0_raw';
} else if (process.platform === 'nuttx') {
  configuration.pin = require('stm32f4dis').pin.ADC1_3;
} else if (process.platform === 'tizenrt') {
  configuration.pin = 0;
} else {
  assert.assert(false, "Unsupported platform: " + process.platform);
}

// start async test
asyncTest();

function asyncTest() {
  var adc0 = adc.open(configuration, function(err) {
    console.log('ADC initialized');

    if (err) {
      assert.assert(false, "Failed to open device.");
    }

    var loopCnt = 5;

    console.log('test1 start(read async test)');
    var test1Loop = setInterval(function() {
      if (--loopCnt < 0) {
        console.log('test1 complete');
        clearInterval(test1Loop);
        adc0.close(function (err) {
          assert.equal(err, null);

          // start sync test
          syncTest();
        });
      } else {
        adc0.read(function(err, value) {
          if (err) {
            assert.assert(false, "Failed to read device.");
          }

          console.log(value);
        });
      }
    }, 1000);
  });
}

function syncTest() {
  var adc0 = adc.openSync(configuration);
  console.log('ADC initialized');

  var loopCnt = 5,
      value = -1;

  console.log('test2 start(read sync test)');
  var test2Loop = setInterval(function() {
    if (--loopCnt < 0) {
      console.log('test2 complete');
      clearInterval(test2Loop);
      adc0.closeSync();
    } else {
      value = adc0.readSync();
      if (value < 0) {
        assert.assert(false, "Failed to read device.");
      } else {
        console.log(value);
      }
    }
  }, 1000);
}

// error test
assert.throws(function() {
   var adc = adc.open(configuration);
}, TypeError, 'Calling adc.open without a callback function.');
