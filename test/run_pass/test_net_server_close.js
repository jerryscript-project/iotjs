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

var check = 0;

var server2 = net.createServer();
server2.listen( {port: 27773, host: "127.0.0.1"} );
server2.on('close', function() { check++; });
server2.close(function() { });

var server3 = net.createServer();
server3.on('close', function() { check++; });
server3.close(function() { });

setTimeout(function () {
  assert.equal(check, 2);
}, 10);
