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
var connectionCount = 0;

if (process.platform === 'linux' || process.platform === 'darwin') {
  var maxConnection = 40;
} else if (process.platform === 'nuttx' || process.platform === 'tizenrt') {
  var maxConnection = 5;
} else {
  assert.fail();
}

var check = [];
var queue = [];
var isClose = false;

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
        isClose = true;
      }
    });
  });
}

serverListen();

function connectServer(i) {
  connectionCount++;

  var socket = new net.Socket();
  var msg = "";

  socket.connect(port, "localhost");
  socket.on('connect', function() {
    socket.end(i.toString(), function() {
      connectionCount--;
    });
  });
  socket.on('data', function(data) {
    check[data] = true;
  });
}

for (var i = 0; i < count; ++i) {
  queue.push(i);
}

var interval = setInterval(function() {
  if (isClose) {
    clearInterval(interval);
  }

  var queueLength = queue.length;
  if (connectionCount !== 0 && queueLength === 0) {
    return;
  }

  var end = maxConnection < queueLength ? maxConnection : queueLength;
  queue.splice(0, end).forEach(function(val) {
    connectServer(val);
  });
}, 500);

process.on('exit', function(code) {
  assert.equal(code, 0);
  for (var i = 0; i < count; ++i) {
    if (!check[i]) {
      assert.fail();
    }
  }
});
