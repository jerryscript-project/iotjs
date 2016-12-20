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
var spi = require('spi');

//  mcp3008 test
var channel = 0;
var spi0 = new spi(0, function() {
  var mode = (8 + channel) << 4;
  var tx = [1, mode, 0];
  var rx = [0, 0, 0];

  spi0.transfer(tx, rx, function(err, buf) {
    assert.equal(err, null);
    assert.equal(buf.length, 3);

    var value = ((buf[1] & 0x03) << 8) + buf[2];
    console.log(value);

    spi0.unexport();
  });
});

// Buffer test
var options = {
  maxSpeed: 1000000,
  mode: 1
};

var spi1 = new spi(0, function() {
  var data = 'Hello IoTjs';
  var tx = new Buffer(data);
  var rx = new Buffer(11);

  spi1.setOption(options);
  spi1.transfer(tx, rx, function(err, buf) {
    assert.equal(err, null);
    assert.equal(buf.length, 11);

    var value = '';
    for (var i = 0; i < 11; i++) {
      value += String.fromCharCode(buf[i]);
    }
    console.log(value);
    assert.equal(value, data);

    spi1.unexport();
  });
});
