
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

var codes = ["100", "150", "199", "200", "204", "304", "404", "510"];
var responses = [];
var completedResponses = 0;

var server = http.createServer(function (request, response) {
  var str = '';

  request.on('data', function (chunk) {
      str += chunk;
  });

  request.on('end', function() {
    if (codes.indexOf(str) > -1) {
      response.writeHead(parseInt(str));
    }

    response.write(str);
    response.end(function() {
      if(str == 'close server') {
        server.close();
      }
    });
  });
});

server.listen(3008, 5);

var options = {
  method: 'POST',
  port: 3008,
  headers: {'Content-Length': 3}
};


for (var i = 0; i < codes.length; i++) {
  var request = http.request(options, function(response) {
    responses.push(response.statusCode);
    completedResponses++;
    if (completedResponses == codes.length) {
      // Done with downloads.
      for (var j = 0; j < codes.length; j++) {
        assert(responses.indexOf(parseInt(codes[j])) > -1);
      }
    }
  }).end(codes[i]);
}

var closeMessage = 'close server';
var closeOptions = {
  method : 'POST',
  port : 3008,
  headers : {'Content-Length': closeMessage.length}
};
var closeHandler = function(response) {
  var str = '';

  assert.equal(200, response.statusCode);

  response.on('end', function() {
    assert.equal(closeMessage, str);
  });

  response.on('data', function(chunk) {
    str += chunk;
  });
};

closeRequest = http.request(closeOptions, closeHandler);
closeRequest.write(closeMessage);
closeRequest.end();
