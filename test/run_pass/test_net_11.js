/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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

var server = net.createServer();
var port = 22701;

server.listen(port, 5);

assert.throws(function() {
  var bad_sock = new net.Socket();
  bad_sock.connect("something", "127.0.0.1");
 }, RangeError);

assert.throws(function() {
  var bad_sock = new net.Socket();
  bad_sock.connect({port: port, host:"127.0.0.1", family:"7"}, "127.0.0.1");
}, RangeError);

var bad_sock = new net.Socket();
bad_sock.on('error', function(err){
  assert.equal(err instanceof Error, true);
});

bad_sock.connect( {
  port: port, host: "255.255.255.test", family: 4
}, "127.0.0.1");

assert.throws(function() {
  var bad_sock = new net.Socket(); bad_sock.write(null);
}, TypeError);


var keepalive_sock = new net.Socket();

keepalive_sock.setKeepAlive(true, 1000);
keepalive_sock._sockname = "";
keepalive_sock.address();

keepalive_sock.setTimeout(1000, function() { });
keepalive_sock.setKeepAlive(1, 0);

keepalive_sock.setTimeout(0, function(){ });
keepalive_sock.setTimeout(0);
keepalive_sock.on('error', function(err){
  assert.equal(err instanceof Error, true);
});

keepalive_sock.connect(80, "127.0.0.01");
var server2 = net.createServer();
server2.listen(80);
server2.on('close', function(){ });
server2.close(function() { });

var server3 = net.createServer();
server3.on('close', function(){ });
server3.close(function() { });

var server4 = net.createServer();
server4.address();
assert.throws(function() {server4.listen("80"); }, Error);
server4.listen(22701, 5);
server4.listen(22701, "127.0.0.1");
keepalive_sock.connect(22701, "127.0.0.1");
server4.address(server4);
server4.listen(80, "127.0.0.1");
server4.close();

keepalive_sock.address(bad_sock);
keepalive_sock._sockname = "test";
keepalive_sock.address(bad_sock);

net.Server("test");
net.Server(function(){ });
net.Socket("test");

server.close();
//special case
var serverd = net.createServer();
serverd.listen(22701, 5);
var socketd = new net.Socket();
socketd.connect(22701, "localhost");
socketd.destroy();
serverd.close();
