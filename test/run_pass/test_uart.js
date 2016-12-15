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
var uart = require('uart');

var device = '/dev/ttyUSB0';
var options = {
  baudRate: 115200,
  dataBits: 8,
  autoOpen: true,
}

var read = 0;
var write = 0;

var serial = new uart(device, options, function(err){
  assert.equal(err, null);
  console.log('open done');

  serial.on('data', function(data) {
    console.log('read result: ' + data.toString());
    read = 1;

    if (read && write) {
      serial.close();
    }
  });


  serial.write("Hello there?\n", function(err) {
    assert.equal(err, null);
    console.log('write done');
    write = 1;

    if (read && write) {
      serial.close();
    }
  });
});
