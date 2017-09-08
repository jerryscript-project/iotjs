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


var Adc = require('adc');
var assert = require('assert');
var adc = new Adc();
var configuration = {};

if (process.platform === 'linux') {
  configuration.device =
    '/sys/devices/12d10000.adc/iio:device0/in_voltage0_raw';
} else if (process.platform === 'nuttx') {
  configuration.pin = require('stm32f4dis').pin.ADC1_3;
} else if (process.platform === 'tizenrt') {
  configuration.pin = 0;
} else {
  assert.fail();
}

asyncTest();

// read async test
function asyncTest() {
  var adc0 = adc.open(configuration, function(err) {
    console.log('ADC initialized');

    if (err) {
      assert.fail();
    }

    var loopCnt = 5;

    console.log('test1 start(read async test)');
    var test1Loop = setInterval(function() {
      if (--loopCnt < 0) {
        console.log('test1 complete');
        clearInterval(test1Loop);
        adc0.closeSync();
        syncTestst();
      } else {
        adc0.read(function(err, value) {
          if (err) {
            console.log('read failed');
            assert.fail();
          }

          console.log(value);
        });
      }
    }, 1000);
  });
}

// read sync test
function syncTestst() {
  var adc0 = adc.open(configuration, function(err) {
    console.log('ADC initialized');

    if (err) {
      assert.fail();
    }

    var loopCnt = 5,
        value = -1;

    console.log('test2 start(read sync test)');
    var test2Loop = setInterval(function() {
      if (--loopCnt < 0) {
        console.log('test2 complete');
        clearInterval(test2Loop);
        adc0.close();
      } else {
        value = adc0.readSync();
        if (value < 0) {
          console.log('read failed');
          assert.fail();
        } else {
          console.log(value);
        }
      }
    }, 1000);
  });
}
