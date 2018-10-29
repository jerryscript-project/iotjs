/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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
var net = require('net');
var stream = require('stream');
var Duplex = stream.Duplex;
var Readable = stream.Readable;
var Writable = stream.Writable;

var readable = new Readable();

var msg = [];
var resp = [];

// Test case 1: basic data correctness.
var dest1 = new Writable();

msg.push('Hello');

dest1._write = function(chunk, callback, onwrite) {
  resp.push(chunk.toString());
}

dest1._readyToWrite();

readable.pipe(dest1);
readable.push(msg[0]);
readable.unpipe(dest1);

// Test case 2: serial piping.
var dest2 = new Duplex();
var dest3 = new Duplex();
var dest4 = new Duplex();

msg.push('data');

dest2._write = function(chunk, callback, onwrite) {
  this.push(chunk);
  resp.push(chunk.toString());
}

dest3._write = function(chunk, callback, onwrite) {
  this.push(chunk);
  resp.push(chunk.toString());
}

dest4._write = function(chunk, callback, onwrite) {
  resp.push(chunk.toString());
}

dest2._readyToWrite();
dest3._readyToWrite();
dest4._readyToWrite();

readable.pipe(dest2).pipe(dest3).pipe(dest4);
readable.push(msg[1]);
readable.unpipe(dest2);
dest2.unpipe(dest3);
dest3.unpipe(dest4);

// Test case 3: unpipe test.
var dest5 = new Writable();

var dest5_write_called = false;
dest5._write = function(chunk, callback, onwrite) {
  dest5_write_called = true;
}

dest5._readyToWrite();

readable.pipe(dest5);
readable.unpipe(dest5);
readable.push('foo');

// Test case 4: pushing data to the readable stream
// before piping it.
var readable2 = new Readable();
var dest6 = new Writable();
msg.push('data before pipe');

dest6._write = function(chunk, callback, onwrite) {
  resp.push(chunk.toString());
}

dest6._readyToWrite();

readable2.push(msg[2]);
readable2.pipe(dest6);
readable2.unpipe(dest6);

// Test case 5: piping multiple destinations to a single Readable
var readable3 = new Readable();
msg.push('Multiple pipe test');

var dest7 = new Writable();
var dest8 = new Duplex();

dest7._write = function(chunk, callback, onwrite) {
  resp.push(chunk.toString());
}

dest8._write = function(chunk, callback, onwrite) {
  resp.push(chunk.toString());
}

dest7._readyToWrite();
dest8._readyToWrite();

readable.pipe(dest7);
readable.pipe(dest8);
readable.push(msg[3]);
readable.unpipe(dest7);
readable.unpipe(dest8);

// Test case 6: piping with net.Socket source
msg.push('Hello from the server');

var server = net.createServer({}, function(socket) {
  socket.write(msg[4]);
});
server.listen(8080);

var dest9 = new Duplex();

dest9._write = function(chunk, callback, onwrite) {
  resp.push(chunk.toString());
  this.emit('_write_done');
}

// This is needed to make sure that the order of the results in the resp
// array is correct.
dest9.on('_write_done', testCase7);

dest9._readyToWrite();

var socket = new net.Socket();
socket.pipe(dest9);

socket.on('data', function(data) {
  socket.end();
});

socket.on('end', function() {
  socket.unpipe(dest9);
  server.close();
});

socket.connect({'port': 8080});

// Test case 7: piping with net.Socket destination
function testCase7() {
 msg.push('Hello from the socket');

  var server2 = net.createServer({}, function(socket) {
    socket.on('data', function(data) {
      resp.push(data.toString());
      socket.end();
    });
  });
  server2.listen(8081);

  var socket2 = new net.Socket();

  socket2.on('data', function(data) {
    socket2.write(data);
  });

  socket2.on('end', function() {
    server2.close();
  });

  var readable4 = new Readable();
  readable4.pipe(socket2);
  socket2.connect({'port': 8081});
  readable4.push(msg[5]);
  readable4.unpipe(socket2);
  socket2.end();
}

// checking the results
process.on('exit', function() {
  assert.equal(msg[0], resp[0], 'Basic data correctness test failed');
  assert.equal(msg[1], resp[1], 'Serial pipe test failed');
  assert.equal(msg[1], resp[2], 'Serial pipe test failed');
  assert.equal(msg[1], resp[3], 'Serial pipe test failed');
  assert.equal(dest5_write_called, false, 'Unpipe test failed');
  assert.equal(msg[2], resp[4], 'Incorrect data when pushing to the ' +
                                'readable stream before piping');
  assert.equal(msg[3], resp[5], 'Multiple piping test failed');
  assert.equal(msg[3], resp[6], 'Multiple piping test failed');
  assert.equal(msg[4], resp[7], 'net.Socket source piping test failed');
  assert.equal(msg[5], resp[8], 'net.Socket destination piping test failed');
});
