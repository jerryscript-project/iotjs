/* Copyright 2016-present Samsung Electronics Co., Ltd. and other contributors
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
var dgram = require('dgram');

var port = 41235;
var msg = '';
var sockcount = 5;
var sendcount = 0;
var server = dgram.createSocket('udp4');

server.on('error', function(err) {
  assert.fail();
  server.close();
});

server.on('message', function(data, rinfo) {
  console.log('server got data : ' + data);
  msg += data;

  server.send(data, rinfo.port, 'localhost', function (err, len) {
    sendcount++;
    if (sendcount >= sockcount) {
      server.close();
    }
  });
});

server.bind(port);

for (var i = 0; i < sockcount; i++) {
  (function sendAndRecieve(i) {
    var client = dgram.createSocket('udp4');

    client.send(i.toString(), port, 'localhost');

    client.on('error', function(err) {
      assert.fail();
      client.close();
    });

    client.on('message', function(data, rinfo) {
      console.log('client got data : ' + data);
      assert.equal(port, rinfo.port);
      assert.equal(data, i.toString());
      client.close();
    });
  })(i);
}

process.on('exit', function(code) {
  assert.equal(msg.length, sockcount);
  for (var i = 0; i < sockcount; i++) {
      if (msg.indexOf(i.toString()) == -1) {
        assert.fail();
      }
  }
  assert.equal(code, 0);
});
