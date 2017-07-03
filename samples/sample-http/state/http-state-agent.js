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
var address = process.argv[2];
var method = process.argv[3];
var state = '';

var req_options = {
  host: address,
  port: port,
};

if (method.toUpperCase() == 'GET') {
  req_options.method = 'GET';
} else if (method.toUpperCase() == 'POST') {
  state = process.argv[4];
  req_options.method = 'POST';
  req_options.headers = { 'Content-Length': state.length };
} else {
  console.log('Invalid method: ' + method);
  process.exit(1);
}

var req = http.request(req_options, function(res) {
  var server_state = '';
  res.on('data', function(data) {
    server_state += data;
  });
  res.on('end', function() {
    console.log('server state: ' + server_state);
  });
});


req.end(state);

