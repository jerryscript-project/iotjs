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

var MODULE_BUFFER = 0;
var MODULE_CONSOLE = 1;
var MODULE_FS = 2;
var MODULE_PROCESS= 3;

this.global = this;
var global = this.global;

this.startIoTjs = function(process) {
  global.console = process.binding(MODULE_CONSOLE);
  var fs = process.binding(MODULE_FS);

  console.log("Hello IoT.js!!");
  fs.open("./greeting.txt", 0, 438, function(fd) {
    console.log("file descriptor: " + fd);
  });
};
