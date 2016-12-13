/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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
var checkReqFinish = false;

// server side code
// Server just ends after sending with some headers.

var server = http.createServer(function (req, res) {

  req.on('data', function (chunk) {
    body += chunk;
  });

  var endHandler = function () {

    res.setHeader('h1','h1');
    res.setHeader('h2','h2');
    res.setHeader('h3','h3');
    res.removeHeader('h2');
    if (res.getHeader('h3') == 'h3') {
      res.setHeader('h3','h3prime'); // h3 value should be overwrited
    }
    // final res.headers = { 'h1' : 'h1', 'h3': 'h3prime' }

    res.end(function(){
        server.close();
    });
  };

  req.on('end', endHandler);

});
server.listen(3045, 3);


// client req code

var options = {
  method : 'GET',
  port : 3045
};


var postResponseHandler = function (res) {
  var res_body = '';

  assert.equal(200, res.statusCode);
  assert.equal(res.headers['h1'], 'h1');
  assert.equal(res.headers['h2'], undefined);
  assert.equal(res.headers['h3'], 'h3prime');

  var endHandler = function(){
    checkReqFinish = true;
  };
  res.on('end', endHandler);

  res.on('data', function(chunk){
    res_body += chunk.toString();
  });
};

var req = http.request(options, postResponseHandler);
req.end();

process.on('exit', function() {
  assert.equal(checkReqFinish, true);
});
