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
var message = process.argv[3];

var req_options = {
  host: address,
  port: port,
  method: 'POST',
  headers: { 'Content-Length': message.length }
};

var req = http.request(req_options, function(res) {
  var body = '';
  res.on('data', function(data) {
    body += data;
  });
  res.on('end', function() {
    console.log(body);
  });
});


req.end(message);


