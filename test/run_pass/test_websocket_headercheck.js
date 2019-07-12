/* Copyright 2019-present Samsung Electronics Co., Ltd. and other contributors
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
var ws = require('websocket');
var http = require('http');

var websocket = new ws.Websocket();

var test_connected = false;
var test_statuscode = -1;

var server = new ws.Server({ port: 8001 }, function(srv) {
  console.log("Connected!");
  test_connected = true;
  server.close()
});

var client = http.request({
  method: 'GET',
  port: 8001,
  headers: {
    // Test if multiple values for the Connection header is accepted
    'Connection': 'keep-alive, Upgrade',
    'Upgrade': 'websocket',
    'Sec-WebSocket-Key': 'r3UXMybFKTPGuT2CK5cYGw==',
    'Sec-WebSocket-Version': 13,
  }
}, function(response) {
  // 101 Switching Protocols
  test_statuscode = response.statusCode;
  server.close();
});

client.end();


process.on('exit', function () {
  assert(test_connected, 'WebScoket server did not received connection event');
  assert.equal(test_statusCode, 101,
               'GET with multiple Connection value should return 101 status');
});
