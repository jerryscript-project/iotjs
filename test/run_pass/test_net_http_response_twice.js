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
var http = require('http');
var net = require('net');

var body = 'Hello IoT.js\r\n';
var fullResponse =
  'HTTP/1.1 500 Internal Server Error\r\n' +
  'Content-Length: ' + body.length + '\r\n' +
  'Content-Type: text/plain\r\n' +
  'Date: Fri + 5 May 2017 10:22:45 GMT\r\n' +
  'Host: 127.0.0.1\r\n' +
  'Access-Control-Allow-Credentials: true\r\n' +
  'Server: badly broken/0.1 (OS NAME)\r\n' +
  '\r\n' +
  body;

var server = net.createServer(function(socket) {
  var postBody = '';

  socket.on('data', function(chunk) {
    postBody += chunk;

    if (postBody.indexOf('\r\n') > -1) {
      socket.write(fullResponse);
      // Wrote the response twice.
      socket.end(fullResponse);
    }
  });

  socket.on('error', function(err) {});
});

server.listen(3085, function() {
  http.get({ port: 3085 }, function(response) {
    var buffer = '';

    response.on('data', function(chunk) {
      buffer += chunk;
    });

    response.on('end', function() {
      assert.equal(body, buffer);
      server.close();
    });
  });
});
