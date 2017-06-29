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
var https = require('https');
var net = require('net');

// Messages for further requests.
var message = 'Hello IoT.js';

// Options for further requests.
var options = {
  method: 'POST',
  host: "httpbin.org",
  path: '/post',
  headers: {'Content-Length': message.length,
    'Content-Type': 'application/json'}
};

// Simple request with valid utf-8 message.
var isRequest1Finished = false;
var request1 = https.request(options, function(response) {
  var str = '';

  response.on('data', function(chunk) {
    str += chunk.toString();
  });

  response.on('end', function() {
    var response = JSON.parse(str);
    assert.equal(message, response['data']);
    isRequest1Finished = true;
  });
});
request1.end(message);


// Simple request with multiple end callback.
var isRequest2Finished = false;
var request2 = https.request(options, function(response) {
  var str = '';

  response.on('data', function(chunk) {
    str += chunk.toString();
  });

  response.on('end', function() {
    var response = JSON.parse(str);
    assert.equal(message, response['data']);
  });
});

request2.end(message, function() {
  isRequest2Finished = true;
});

// Call the request2 end again to test the finish state.
request2.end(message, function() {
  // This clabback should never be called.
  assert.equal(isRequest2Finished, false);
});


// Simple request with buffer chunk as message parameter.
var isRequest3Finished = false;
var request3 = https.request(options, function(response) {
  var str = '';

  response.on('data', function(chunk) {
    str += chunk;
  });

  response.on('end', function() {
    var response = JSON.parse(str);
    assert.equal(message, response['data']);
    isRequest3Finished = true;
  });
});
request3.end(new Buffer(message));


// Test the IncomingMessage read function.
var isRequest4Finished = false;
var readRequest = https.request({
  method: 'GET',
  host: "httpbin.org",
  path: '/get'
});

readRequest.on('response', function(incomingMessage) {
  incomingMessage.on('readable', function() {
    var inc = incomingMessage.read();
    assert.equal(inc instanceof Buffer, true);
    assert(inc.toString('utf8').length > 0);
    isRequest4Finished = true;
  });
});
readRequest.end();


process.on('exit', function() {
  assert.equal(isRequest1Finished, true);
  assert.equal(isRequest2Finished, true);
  assert.equal(isRequest3Finished, true);
  assert.equal(isRequest4Finished, true);
});
