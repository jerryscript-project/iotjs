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
var https = require('https');


var isRequest1Finished = false;
// 1. GET req
options = {
  method: 'GET',
  host: "httpbin.org",
  path: '/user-agent',
  headers: {'user-agent': 'iotjs'}
};

var getResponseHandler = function (res) {
  var res_body = '';

  assert.equal(200, res.statusCode);

  var endHandler = function(){
    var response = JSON.parse(res_body);
    assert.equal('iotjs', response['user-agent']);
    isRequest1Finished = true;
  };
  res.on('end', endHandler);

  res.on('data', function(chunk){
    res_body += chunk.toString();
  });
};

https.get(options, getResponseHandler);

// 2. close server req
var testMsg = 'Hello IoT.js';
var finalOptions = {
  method: 'POST',
  host: "httpbin.org",
  path: '/post',
  headers: {'Content-Length': testMsg.length,
    'Content-Type': 'application/json'}
};
var isRequest2Finished = false;

var finalResponseHandler = function (res) {
  var res_body = '';

  assert.equal(200, res.statusCode);

  var endHandler = function(){
    var response = JSON.parse(res_body);
    assert.equal(testMsg, response['data']);
    isRequest2Finished = true;
  };
  res.on('end', endHandler);

  res.on('data', function(chunk){
    res_body += chunk.toString();
  });
};

var finalReq = https.request(finalOptions, finalResponseHandler);
finalReq.write(testMsg);
finalReq.end();


process.on('exit', function() {
  assert.equal(isRequest1Finished, true);
  assert.equal(isRequest2Finished, true);
});
