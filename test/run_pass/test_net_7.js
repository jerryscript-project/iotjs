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


var net = require('net');
var assert = require('assert');
var timers = require('timers');

var port = 22707;

var count = 40;
var check = [];

function serverListen() {
  var server = net.createServer({
    allowHalfOpen: true
  });

  var cnt = 0;

  server.listen(port);

  server.on('connection', function(socket) {
    var msg = '';
    socket.on('data', function(data) {
      msg += data;
    });
    socket.on('end', function() {
      socket.end(msg);
      cnt++;

      if (cnt == count) {
        server.close();
      }
    });
  });
}

serverListen();

for (var i = 0; i < count; ++i) {
  (function(i) {
    var socket = new net.Socket();
    var msg = "";

    socket.connect(port, "localhost");
    socket.on('connect', function() {
      socket.end(i.toString());
    });
    socket.on('data', function(data) {
      check[data] = true;
    });
  })(i);
}

process.on('exit', function(code) {
  assert.equal(code, 0);
  for (var i = 0; i < count; ++i) {
    if (!check[i]) {
      assert.fail();
    }
  }
});
