/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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

var ws = require('websocket');
var assert = require('assert');

var websocket = new ws.Websocket();

var connected = false;
var message_sent = 'Hello IoT.js WebSocket';
var message_received = null;
var ping_sent = 'IoT.js ping message';
var ping_received = null;
var close_string = 'Connection successfully closed';

websocket.connect('ws://echo.websocket.org', 80, '/', function() {
  connected = true;

  this.on('message', function(msg){
    message_received = msg.toString();
  });

  this.ping(ping_sent, true, function(msg) {
    ping_received = msg.toString();
  });

  this.on('close', function(msg) {
    assert.equal(msg.code, '1000');
    assert.equal(msg.reason, close_string);
  });

  this.send(message_sent, {mask: true, binary: false})
  this.close(close_string, 1000);
});


process.on('exit', function() {
  assert.equal(connected, true);
  assert.equal(message_received, message_sent);
  assert.equal(ping_sent, ping_received);
});
