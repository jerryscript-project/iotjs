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
var spi = require('spi');
var pin = require('tools/systemio_common').pin;
var checkError = require('tools/systemio_common').checkError;

var configuration = {
    device: pin.spi1, // for Linux
    bus: pin.spi1, // for Tizen, TizenRT and Nuttx
};

//  mcp3008 test
var channel = 0;
var spi0 = spi.open(configuration, function(err) {
  checkError(err);

  var mode = (8 + channel) << 4;
  var tx = [1, mode, 0];

  var rx = spi0.transferSync(tx);
  console.log(((rx[1] & 0x03) << 8) + rx[2]);

  var loopCnt = 10;
  var loop = setInterval(function() {
    spi0.transfer(tx, function(err, rx) {
      checkError(err);
      assert.equal(rx.length, 3);

      var value = ((rx[1] & 0x03) << 8) + rx[2];
      console.log(value);

      if (--loopCnt < 0) {
        spi0.closeSync();
        clearInterval(loop);
        console.log('finish test');
      }

    });
  }, 500);
});
