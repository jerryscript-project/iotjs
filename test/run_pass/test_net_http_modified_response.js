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

var assert = require('assert');
var http = require('http');

var response_message = 'DATA';
var response_got = 'NOT THIS';

/*
 * Test if 'subclassing' the ServerResponse is propagated correctly.
 */
function newServerResponse() {
  http.ServerResponse.apply(this, arguments);
};
newServerResponse.prototype = Object.create(http.ServerResponse.prototype);
newServerResponse.prototype.sendJSON = function (obj) {
  var message_body = JSON.stringify(obj);
  this.setHeader('Content-Length', message_body.length);
  this.setHeader('Content-Type', 'application/json');
  this.writeHead(200);
  this.end(message_body);
};

var serveropts = {
  ServerResponse: newServerResponse,
};

var server = http.createServer(serveropts, function (req, res) {
  assert.equal(req.method, 'POST', 'Incorrect request method detected');
  assert.equal(req.url, '/put_msg', 'Incorrect request url detected');
  assert.assert(req instanceof http.IncomingMessage,
                'Request is not instance of http.IncomingMessage');

  var chunks = [];
  req.on('data', function(chunk) {
    chunks.push(chunk);
  });
  req.on('end', function() {
    var body = Buffer.concat(chunks);
    assert.equal(body.toString(), 'DEMO');

    res.sendJSON({message: response_message});
  });
});

server.listen(3001);

var demo_msg = 'DEMO';
var options = {
  method : 'POST',
  port : 3001,
  path : '/put_msg',
  headers: {
    'Content-Length': demo_msg.length,
  },
};
var req = http.request(options, function (response){
  var content_type =
    response.headers['Content-Type'] || response.headers['content-type']
  assert.equal(content_type, 'application/json',
               'Incorrect content type returned by the server');

  var chunks = []
  response.on('data', function(chunk) {
    chunks.push(chunk);
  });
  response.on('end', function() {
    var body = JSON.parse(Buffer.concat(chunks).toString());
    assert.assert('message' in body, 'No "message" key in response JSON');
    response_got = body.message;

    server.close();
  });
});

req.end(demo_msg);

process.on('exit', function() {
  assert.equal(response_got, response_message,
               'Invalid response returned from the demo server');
});
