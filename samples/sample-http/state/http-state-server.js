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

var http = require('http');

var port = 8080;
var state = '';

var server = http.createServer(function(req, res) {
  var response = function() {
    res.writeHead(200, { 'Content-Type' : 'text/plain',
                         'Content-Length' : state.length });
    res.end(state);
  };

  if (req.method == 'GET') {
    response();
  } else if (req.method == 'POST') {
    var new_state = ''
    req.on('data', function(data) {
      new_state += data;
    });
    req.on('end', function() {
      state = new_state;
      response();
    });
  }
});

server.listen(port, 5);

