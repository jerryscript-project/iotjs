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

var http = require('http');
var assert = require('assert');

var clientMessage = new Buffer([0xE6, 0x7F, 0x3, 0x6, 0x7]);
var serverMessage = new Buffer([0x3, 0x7F, 0x6, 0x7, 0xE6]);
var serverReceived;
var clientReceived;

var server = http.createServer(function(req, res) {
  var received = [];
  req.on('data', function(data) {
    received.push(data);
  });
  req.on('end', function() {
    serverReceived = Buffer.concat(received);
    res.end(serverMessage);
  });
}).listen(8383, 5);

var reqOpts = {
  method: 'POST',
  port: 8383,
  path: '/',
  headers: {'Content-Length': clientMessage.length},
};

var clientReq = http.request(reqOpts, function(res) {
  var response = [];

  res.on('data', function(data) {
    response.push(data);
  });

  res.on('end', function() {
    clientReceived = Buffer.concat(response);
    server.close();
  });
});

clientReq.end(clientMessage);

process.on('exit', function() {
  assert.equal(serverReceived.compare(clientMessage), 0);
  assert.equal(clientReceived.compare(serverMessage), 0);
});
