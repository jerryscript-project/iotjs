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
var MODULE_PROCESS = 3;
var MODULE_TIMER = 4;

this.global = this;
var global = this.global;

  global.console = process.binding(MODULE_CONSOLE);
  var Timer = process.binding(MODULE_TIMER);

  var timerobj = new Timer();
  timerobj.start(500, 0, function() {
    console.log("JS Timer fired");
  });
  console.log("JS Timer timeout in 500 msec...");

  var repeat_cnt = 1;
  var timerobj2 = new Timer();
  timerobj2.start(400,100, function() {
    console.log("Js Timer repeat #" + repeat_cnt);
    if (repeat_cnt >= 5) {
      timerobj2.stop();
    }
    repeat_cnt++;
  });
  console.log("JS Timer repeat in 400 msec, 100 msec interval, 5 times...");
