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

//  mcp3008 test
var channel = 0;
var spi0 = spi.open({
  device: '/dev/spidev0.0'
}, function() {
  var mode = (8 + channel) << 4;
  var tx = [1, mode, 0];
  var rx = [0, 0, 0];

  spi0.transferSync(tx, rx);
  console.log(((rx[1] & 0x03) << 8) + rx[2]);

  setTimeout(function() {
    spi0.transfer(tx, rx, function(err) {
      assert.equal(err, null);
      assert.equal(rx.length, 3);

      var value = ((rx[1] & 0x03) << 8) + rx[2];
      console.log(value);

      spi0.close();
    });
  }, 500);
});

// Buffer test
var spi1 = spi.open({device: '/dev/spidev0.0'}, function() {
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
