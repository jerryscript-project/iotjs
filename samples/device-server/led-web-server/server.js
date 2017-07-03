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
var gpio = require('gpio');

var port = 8080;
var result = '';

var ledPin = 16;

var server = http.createServer(function(req, res) {
  console.log('on request - url: ' + req.url);
  if (req.url == '/') {
    onIndex(req, res);
  } else if (req.url == '/light') {
    onLight(req, res);
  } else if (req.url == '/toggle') {
    onToggle(req, res);
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

function onLight(req, res) {
  gpio.readPin(ledPin, function(err, value) {
    if (err) {
      res.writeHead(500);
      res.end();
    } else {
      res.writeHead(200);
      res.end(value ? "on" : "off");
    }
  });
}

function onToggle(req, res) {
  gpio.readPin(ledPin, function(err, value) {
    if (err) {
      res.writeHead(500);
      res.end();
    } else {
      gpio.writePin(ledPin, !value, function(err) {
        if (err) {
          res.writeHead(500);
          res.end();
        } else {
          res.writeHead(200);
          res.end(value ? "on" : "off");
        }
      });
    }
  });
}

gpio.initialize();

gpio.on('initialize', function() {
  console.log('GPIO initilized');
  gpio.setPin(ledPin, "out", function() {
    console.log('GPIO led ready');
    server.listen(port);
  });
});

gpio.on('error', function(err) {
  console.log(err);
  process.exit(1);
});
