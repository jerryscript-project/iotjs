
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

var codes = ["150", "199", "200", "204", "304", "404", "510"];
var queue = codes.slice(0);
var responses = [];
var options = {
  method: 'POST',
  port: 3008,
  headers: {'Content-Length': 3}
};

var server = http.createServer(function (request, response) {
  var str = '';

  request.on('data', function (chunk) {
      str += chunk;
  });

  request.on('end', function() {
    if (codes.indexOf(str) > -1) {
      response.writeHead(parseInt(str));
    }

    response.end();
  });
}).listen(3008, 5);

requestOnQueue(queue.shift());

function requestOnQueue(code) {
  var request = http.request(options, function(res) {
      responses.push(res.statusCode);

      if (responses.length == codes.length) {
        // Done with downloads.
        for (var j = 0; j < codes.length; j++) {
          assert(responses.indexOf(parseInt(codes[j])) > -1);
        }

        server.close();
      } else {
        if(queue.length) {
          process.nextTick(function() {
            requestOnQueue(queue.shift());
          });
        }
      }
    }).end(code);
}
