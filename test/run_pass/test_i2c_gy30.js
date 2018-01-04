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


/* This should be tested with GY-30 Sensor.
  *
  * The following is pin information.
  * Linux(Raspberry PI2): BCM2(SDA), BCM3(SCL)
  * NuttX(STM32F4-Discovery): PB7(SDA), PB8(SCL)
  * TizenRT(Artik053): CON703(10)(SDA), CON703(8)(SCL) - XI2C1
  *
*/

var assert = require('assert');
var pin = require('tools/systemio_common').pin;
var checkError = require('tools/systemio_common').checkError;
var i2c = require('i2c');

var configuration = {
  address: 0x23,
  device: pin.i2c1, // for Linux
  bus: pin.i2c1, // for TizenRT and NuttX
};

syncTest();

function syncTest() {
  console.log('I2C sync function test');

  var wire = i2c.openSync(configuration);
  var loopCnt = 5;

  var loop = setInterval(function() {
    wire.writeSync([0x10]);
    var res = wire.readSync(2);
    console.log('read result', (res[1] + (256 * res[0])));

    if (--loopCnt <= 0) {
      clearInterval(loop);
      wire.closeSync();
      asyncTest();
    }
  }, 800);
}

function asyncTest() {
  console.log('I2C async function test');

  i2c.open(configuration, function(err, wire) {
    checkError(err);
    var loopCnt = 5;

    var loop = setInterval(function() {
      wire.write([0x10], function(err) {
        checkError(err);

        wire.read(2, function(err, res) {
          checkError(err);
          assert.equal(res.length, 2, 'I2C read failed.(length is not equal)');

          console.log('read result: ', (res[1] + (256 * res[0])));

          if (--loopCnt <= 0) {
            clearInterval(loop);
            wire.close();
          }
        });
      });
    }, 800);
  });
}
