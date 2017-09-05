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
var dgram = require('dgram');

var port = 41234;
var msg = 'Hello IoT.js';
var msg2 = 'Bye IoT.js';
var addr = '192.168.0.1'; // Change to your ip address
var server = dgram.createSocket('udp4');

var client = dgram.createSocket('udp4');

client.send(msg, port, addr, function(err, len) {
  assert.equal(err, null);
  assert.equal(len, msg.length);
});

client.on('error', function(err) {
  assert.fail();
});

client.on('listening', function(err) {
  client.setTTL(2);
});

client.on('message', function(data, rinfo) {
  console.log('client got data : ' + data);
  console.log('server address : ' + rinfo.address);
  console.log('server port : ' + rinfo.port);
  console.log('server family : ' + rinfo.family);
  assert.equal(port, rinfo.port);
  assert.equal(data, msg);
  /* send with TTL=1 */
  client.setTTL(1);
  client.send(msg2, port, addr, function(err, len) {
    assert.equal(err, null);
    assert.equal(len, msg2.length);
    client.close();
  });
});

process.on('exit', function(code) {
  assert.equal(code, 0);
});

