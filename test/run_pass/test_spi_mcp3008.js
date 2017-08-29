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
var Spi = require('spi');

var spi = new Spi();

var configuration = {};

if (process.platform === 'linux') {
  configuration.device = '/dev/spidev0.0';
} else if (process.platform === 'nuttx' || process.platform === 'tizenrt') {
  configuration.bus = 1;
} else {
  assert.fail();
}

//  mcp3008 test
var channel = 0;
var spi0 = spi.open(configuration, function() {
  var mode = (8 + channel) << 4;
  var tx = [1, mode, 0];
  var rx = [0, 0, 0];

  spi0.transferSync(tx, rx);
  console.log(((rx[1] & 0x03) << 8) + rx[2]);

  var loopCnt = 10;
  var loop = setInterval(function() {
    spi0.transfer(tx, rx, function(err) {
      assert.equal(err, null);
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
