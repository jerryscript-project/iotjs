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

var fs = require('fs');
var http = require('http');


var port = 8080;
var result = '';

var server = http.createServer(function(req, res) {
  if (req.url == '/') {
    onIndex(req, res);
  } else if (req.url == '/calculate') {
    onCalculate(req, res);
  } else {
    res.writeHead(500);
    res.end();
  }
});


function onIndex(req, res) {
  fs.readFile('index.html', function(err, data) {
    if (err) {
      res.writeHead(500);
      res.end();
    } else {
      res.writeHead(200);
      res.end(data);
    }
  });
}

function onCalculate(req, res) {
  var formula = '';

  req.on('data', function(data) {
    formula += data;
  });

  req.on('end', function() {
    res.writeHead(200);
    try {
      result = eval(formula);
    } catch (e) {
    }
    res.end(result);
  });
}

server.listen(port);
