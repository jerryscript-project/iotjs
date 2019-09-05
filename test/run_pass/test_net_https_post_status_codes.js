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
// 1. POST req
var data = JSON.stringify({});

var options = {
  method: 'POST',
  hostname: 'httpbin.org',
  path: '/post',
  rejectUnauthorized: false,
  headers: {
    'content-type': 'application/json',
    'content-length': data.length,
  }
};

var getResponseHandler = function(res) {
  var res_body = '';
  assert.equal(200, res.statusCode);

  var endHandler = function() {
    var response = JSON.parse(res_body);
    assert.assert(response['data'], 'Recieved incorrect response from server');
    isRequest1Finished = true;
  };
  res.on('end', endHandler);

  res.on('data', function(chunk) {
    res_body += chunk.toString();
  });
};

var req = https.request(options, getResponseHandler);
req.write(data);
req.end();

process.on('exit', function() {
  assert.equal(isRequest1Finished, true);
});
