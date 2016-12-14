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
var stm32fPin = require('pin').STM32F4DIS;

var adc0 = new adc(stm32fPin.ADC1_3, function(err) {
  console.log('ADC initialized');

  if (err) {
    assert.fail();
  }

  test1();
});

// read async test
function test1() {
  var loopCnt = 5;

  console.log('test1 start(read async test)');
  var test1Loop = setInterval(function() {
    if (--loopCnt < 0) {
      console.log('test1 complete');
      clearInterval(test1Loop);
      test2();
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
}

// read sync test
function test2() {
  var loopCnt = 5,
    value = -1;

  console.log('test2 start(read sync test)');
  var test2Loop = setInterval(function() {
    if (--loopCnt < 0) {
      console.log('test2 complete');
      clearInterval(test2Loop);
      adc0.unexport();
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
}
