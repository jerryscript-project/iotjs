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

var net = require('net');
var port = 7467;
var address = process.argv[2];
var formula = process.argv[3];

var socket = new net.Socket();

socket.connect(port, address, function() {
  socket.end(formula);
});

var res = '';

socket.on('data', function(data) {
  res += data;
});

socket.on('end', function() {
  console.log(formula + " = " + res);
});

