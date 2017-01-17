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

/* This test is based on Raspberry Pi with GY-30 Sensor. */

var assert = require('assert');
var i2c = require('i2c');

var address = 0x23;
var wire = new i2c(address, {device: '/dev/i2c-1'});

wire.write([0x10], function(err) {
  assert.equal(err, null);
  console.log('write done');
});

wire.writeByte(0x10, function(err) {
  assert.equal(err, null);
  console.log('writeByte done');
});

wire.writeBytes(0x10, [0x10], function(err) {
  assert.equal(err, null);
  console.log('writeBytes done');
})

wire.read(2, function(err, res) {
  assert.equal(err, null);
  assert.equal(res.length, 2, 'I2C read failed.(length is not equal)');
  console.log('read result: '+res[0]+', '+res[1]);
});

wire.readByte(function(err, res) {
  assert.equal(err, null);
  console.log('readByte result: '+res);
});

wire.readBytes(0x20, 2, function(err, res) {
  assert.equal(err, null);
  assert.equal(res.length, 2, 'I2C readBytes failed.(length is not equal)');
  console.log('readBytes(cmd:0x20, length:2) result: '+res[0]+', '+res[1]);
});

wire.scan(function(err, res) {
  assert.equal(err, null);
  console.log('scan result: ' + res.length + ' device\(s\) scanned.');
  for (var i = 0; i < res.length; i++) {
    console.log(' .result[' + i + ']: ' + res[i]);
  }
  wire.stream(0x20, 2, 1000);
});

var count = 0;
wire.on('data', function(data) {
  console.log('data listener');
  console.log('data.timestamp: '+data.timestamp);
  console.log('data.address: '+data.address);
  console.log('data.cmd: '+data.cmd);
  console.log('data.length: '+data.length);
  console.log('data.data: '+data.data);
  console.log('');
  if(count++ == 2) process.exit();
});
