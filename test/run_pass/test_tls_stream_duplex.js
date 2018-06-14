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

var stream = require('stream');
var tls = require('tls');
var assert = require('assert');
var fs = require('fs');

var tls1_message = 'Hello, this is tls1';
var tls1_expected_message = tls1_message;
var tls2_message = 'Hello, this is tls2';
var tls2_expected_message = tls2_message;
var tls1_received_message = '';
var tls2_received_message = '';
var tls1_received_encrypted = '';
var tls2_received_encrypted = '';

var handshake_done = false;
var tls1_ended = false;
var tls2_ended = false;

var duplex1 = new stream.Duplex();
var duplex2 = new stream.Duplex();

duplex1._write = function(chunk, callback, onwrite) {
  duplex2.push(chunk);
  onwrite();
};

duplex2._write = function(chunk, callback, onwrite) {
  duplex1.push(chunk);
  onwrite();
};

duplex1._readyToWrite();
duplex2._readyToWrite();

var server_opts = {
  key: fs.readFileSync(process.cwd() + '/resources/my_key.key').toString(),
  cert: fs.readFileSync(process.cwd() + '/resources/my_crt.crt').toString(),
  isServer: true,
  rejectUnauthorized: false,
};

var client_opts = {
  rejectUnauthorized: false,
};

var tls1 = new tls.TLSSocket(duplex1, server_opts);
var tls2 = new tls.TLSSocket(duplex2, client_opts);

tls2.on('secureConnect', function() {
  handshake_done = true;
});

tls1.on('data', function(data) {
  tls1_received_message += data.toString();
  tls1.end();
});

tls1._socket.on('data', function(data) {
  tls1_received_encrypted += data.toString('hex');
});

tls2.on('data', function(data) {
  tls2_received_message += data.toString();
  tls2.write(tls2_message);
});

tls2._socket.on('data', function(data) {
  tls2_received_encrypted += data.toString('hex');
});

tls1.on('end', function() {
  tls1_ended = true;
  tls2.end();
});

tls2.on('end', function() {
  tls2_ended = true;
});

tls1.write(tls1_message);

process.on('exit', function() {
  assert.equal(tls1_received_message === tls2_expected_message, true);
  assert.equal(tls2_received_message === tls1_expected_message, true);
  assert.equal(tls1_received_encrypted === tls2_message, false);
  assert.equal(tls2_received_encrypted === tls1_message, false);
  assert.equal(handshake_done === true, true);
  assert.equal(tls1_ended === true, true);
  assert.equal(tls2_ended === true, true);
});
