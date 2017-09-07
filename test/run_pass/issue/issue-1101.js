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

var UART = require('uart');
var uart = new UART();
var res = uart.open({
  device: '/dev/ttyS1',
  baudRate: 115200,
  dataBits: 8
}, function(err) {
  if (err) {
    console.error(err);
  } else {
    console.log('opened');
    res.close(function(err) {
      if (err) {
      console.error(err);
    } else {
      console.log('closed');
    }
    });
    var c = 0;
    var p = function() {
      c++;
      console.log('this should still run');
      if (c < 10) {
        setTimeout(p, 250);
      }
    };
    p();
  }
});
