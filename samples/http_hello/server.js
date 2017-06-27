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

http.createServer(function (req, res) {
  if (req.method == 'GET') {
    status(res, 'Hello, IoT.JS!');

  } else if (req.method == 'POST') {
    receive(req, function (data) {
      var obj = JSON.parse(data);
      obj.answer = 'Hello, There!'
      status(res, obj);
    });
  }
}).listen(port);

function receive(incoming, callback) {
  var data = '';

  incoming.on('data', function (chunk) {
    data += chunk;
  });

  incoming.on('end', function () {
    callback ? callback(data) : '';
  });
}

function status(res, data) {
  var isJson = (typeof data === 'object');

  if (isJson) {
    data = JSON.stringify(data);
  }

  var headers = {
    'Access-Control-Allow-Origin': '*',
    'Access-Control-Allow-Headers':
    'Origin, X-Requested-With, Content-Type, Accept',
    'Content-Type': isJson ? 'application/json' : 'text/plain',
  };

  res.writeHead(200, headers);
  res.end(data);
};
