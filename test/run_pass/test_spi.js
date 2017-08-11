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
} else if (process.platform === 'nuttx') {
  configuration.bus = 1;
} else if (process.platform === 'tizenrt') {
  configuration.bus = 0;
} else {
  assert.fail('OS not supported:' + process.platform);
}


// ------ Test API existance
assert.equal(typeof Spi, 'function',
             'spi module does not export construction function');
assert.assert(spi.MODE,
              'spi module does not provide \'MODE\' property');
assert.assert(spi.CHIPSELECT,
              'spi module does not provide \'CHIPSELECT\' property');
assert.assert(spi.BITORDER,
              'spi module does not provide \'BITORDER\' property');
assert.equal(typeof spi.open, 'function',
             'spi does not provide \'open\' function');


// ------ Test basic API functions
var data = 'Hello IoTjs';
var tx = new Buffer(data);
var rx = new Buffer(data.length);

var spi1 = spi.open(configuration, function(err) {
  assert.assert(err === null, 'spi.open failed: ' + err);

  assert.equal(typeof spi1.transfer, 'function',
              'spibus does not provide \'transfer\' function');
  assert.equal(typeof spi1.transferSync, 'function',
              'spibus does not provide \'transferSync\' function');
  assert.equal(typeof spi1.close, 'function',
              'spibus does not provide \'close\' function');
  assert.equal(typeof spi1.closeSync, 'function',
              'spibus does not provide \'closeSync\' function');

  spi1.transfer(tx, rx, function(err) {
    assert.assert(err === null, 'spibus.transfer failed: ' + err);

    spi1.close(function(err) {
      assert.assert(err === null, 'spibus.close failed: ' + err);
      testSync();
    });
  });
});

function testSync() {
  var spi2 = spi.open(configuration, function(err) {
    assert.assert(err === null, 'spi.open for sync test failed: ' + err);
    spi2.transferSync(tx, rx);
    spi2.closeSync();
  });
}
