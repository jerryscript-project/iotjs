/* Copyright 2015 Samsung Electronics Co., Ltd.
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


var onRead = function(err, bytesRead, buffer) {
  if (err) {
    console.log(err.message);
  } else {
    console.log(buffer.toString());
  }
};


var onOpen = function(err, fd) {
  if (err) {
    console.log(err.message);
  } else {
    buffer = new Buffer(64);
    fs.read(fd, buffer, 0, 64, 0, onRead);
  }
};


fs.open("greeting.txt", "r", 438, onOpen);
