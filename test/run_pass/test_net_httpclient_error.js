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
  var str = '';

  request.on('data', function (chunk) {
    str += chunk;
  });

  request.on('end', function() {
    response.end();
  });
});

server.listen(20004, 5);

// Connection refusal: The request will try to connect to a server,
// however the connection can not be created because the port is invalid.
var req = http.request({
  host: 'localhost',
  port: 20002,
  path: '/',
  method: 'GET'
}, function(response) {
  var str = '';

  response.on('data', function(chunk) {
    str += chunk;
  });

  response.on('end', function() {
    recievedResponse = true;
    server.close();
  });
});

req.on('error', function() {
  recievedResponse = false;
  server.close();
});

req.end();


process.on('exit', function() {
  assert.equal(recievedResponse, false);
});
