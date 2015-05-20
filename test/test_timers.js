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

var ti_count = 1;

var timerA = setTimeout(function() {
  console.log("Timer A fired");
}, 1000);

var timerB = setInterval(function() {
  console.log("Timer B fired " + ti_count);
  ti_count++;
  if (ti_count > 5) {
    clearInterval(timerB);
  }
}, 300);

console.log("Timer A fire after 1,000 msec...");
console.log("Timer B fire every 300 msec 5 times...");
