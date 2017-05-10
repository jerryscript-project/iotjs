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

var recievedResponse = false;

var server = http.createServer(function(request, response) {
  server.close();
});

server.listen(3085, 'localhost');

// Connection refusal: The request will try to connect to a server,
// however the connection can not be created because the port is invalid.
// Therefore the client will destroy the socket without an error emit.
http.request({
  host: 'localhost',
  port: 3089,
  path: '/',
  method: 'GET'
}, function(response) {
  response.on('end', function() {
    recievedResponse = true;
  });
}).end();

process.on('exit', function() {
  assert.equal(recievedResponse, false);
});
