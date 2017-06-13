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

var net = require('net');
var assert = require('assert');

var port = 22701;

assert.throws(function() {
  var bad_sock = new net.Socket();
  bad_sock.connect("something", "127.0.0.1");
}, RangeError);

assert.throws(function() {
  var bad_sock = new net.Socket();
  bad_sock.connect({port: port, host:"127.0.0.1", family:"7"}, "127.0.0.1");
}, RangeError);

assert.throws(function() {
  var bad_sock = new net.Socket(); bad_sock.write(null);
}, TypeError);

var bad_sock = new net.Socket();
bad_sock.on('error', function(err){
  assert.equal(err instanceof Error, true);
});

bad_sock.connect( {
  port: port, host: "255.255.255.test", family: 4
}, "127.0.0.1");

bad_sock.connect(80, "127.0.0.01");

//Assertion fail issue test
var server = net.createServer();
server.listen(22701, 5);
var socket = new net.Socket();
var check = 0;
socket.connect(22701, "localhost");
socket.destroy();
socket.on('finish', function() { check++; });
setTimeout(function () {
  assert.equal(check, 1);
}, 1);
server.close();
