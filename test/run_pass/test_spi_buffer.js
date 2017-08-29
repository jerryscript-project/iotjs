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

// Buffer test
var spi1 = spi.open(configuration, function() {
  var data = 'Hello IoTjs';
  var tx = new Buffer(data);
  var rx = new Buffer(11);

  this.transferSync(tx, rx);
  var value = '';
  for (var i = 0; i < 11; i++) {
    value += String.fromCharCode(rx[i]);
  }
  console.log(value);
  assert.equal(value, data);

  setTimeout(function() {
    spi1.transfer(tx, rx, function(err) {
      assert.equal(err, null);
      assert.equal(rx.length, 11);

      var value = '';
      for (var i = 0; i < 11; i++) {
        value += String.fromCharCode(rx[i]);
      }
      console.log(value);
      assert.equal(value, data);

      spi1.close();
    });
  }, 500);
});
