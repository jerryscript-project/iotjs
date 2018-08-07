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

var websocket = require('websocket');
var assert = require('assert');
var http = require('http');

var client = new websocket.Websocket();
var client2 = new websocket.Websocket();
var client3 = new websocket.Websocket();
var message_sent = 'Hello IoT.js WebSocket';
var ping_sent = 'IoT.js ping message';
var close_string = 'Connection successfully closed';
var close_code = 1000;

function listener(ws) {
  ws.on('message', function(msg) {
    assert.equal(message_sent, msg.toString(),
                 'Message received by the Server');
    // echo
    ws.send(msg.toString(), {mask: true, binary: false});
  });
  ws.on('ping', function(msg) {
    assert.equal(ping_sent, msg.toString(),
                 'Ping received by the Server');
  });
}


// Test two clients connection
var options = {
  port: 8081,
};

var wss = new websocket.Server(options, listener);
var address = wss.address();

wss.on('close', function(msg) {
  assert.equal(close_string, msg.reason,
               'Close reason received by the Server1');
  assert.equal(close_code, msg.code,
               'Close code received by the Server1');
  assert.equal(address.address, '0.0.0.0', 'Address of Server1');
  assert.equal(address.family, 'IPv4', 'Ip family of Server1');
  assert.equal(address.port, '8081', 'Port of Server1');
});

client.connect('ws://localhost', 8081, '/', function() {
  this.on('message', function(msg) {
    assert.equal(message_sent, msg.toString(),
                 'Message received by the Client1');
  });

  this.ping(ping_sent, true, function(msg) {
    assert.equal(ping_sent, msg.toString(),
                 'Ping received by the Client1');
  });

  this.on('close', function(msg) {
    assert.equal(close_string, msg.reason,
                 'Close reason received by the Client1');
    assert.equal(close_code, msg.code,
                 'Close code received by the Client1');
  });

  // Client2 connect
  client2.connect('ws://localhost', 8081, '/');
  client2.on('open', function() {
    // Broadcast then terminate all clients and close the server
    wss.broadcast(message_sent);
    wss.close();
  });

  this.send(message_sent, {mask: true, binary: false});
});


// Test http server upgrade to websocket
var httpServer = http.createServer().listen(8082);

options = {
  server: httpServer,
};

var wss2 = new websocket.Server(options, listener);

wss2.on('close', function(msg) {
  assert.equal(close_string, msg.reason,
               'Close reason received by the Server2');
  assert.equal(close_code, msg.code,
               'Close code received by the Server2');
});

client3.connect('ws://localhost', 8082, '/', function() {
  this.on('message', function(msg) {
    assert.equal(message_sent, msg.toString(),
                 'Message received by the Client3');
    wss2.close();
  });

  this.ping(ping_sent, true, function(msg) {
    assert.equal(ping_sent, msg.toString(),
                 'Ping received by the Client3');
  });

  this.on('close', function(msg) {
    assert.equal(close_string, msg.reason,
                 'Close reason received by the Client3');
    assert.equal(close_code, msg.code,
                 'Close code received by the Client3');
  });

  this.send(message_sent, {mask: true, binary: false});
});
