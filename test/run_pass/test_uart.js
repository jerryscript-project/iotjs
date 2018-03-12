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

var uart = require('uart');
var pin = require('tools/systemio_common').pin;
var checkError = require('tools/systemio_common').checkError;

var configuration = {
  device: pin.uart1, // for Linux, TizenRT and Nuttx
  port: pin.uart1,   // for Tizen
  baudRate: 115200,
  dataBits: 8,
};

writeTest();

function writeTest() {
  var serial = uart.openSync(configuration);
  console.log('open done');

  serial.writeSync('Hello IoT.js.\n\r');
  serial.closeSync();
  console.log('close done');
  writeReadTest();
}

function writeReadTest() {
  var read = 0;
  var write = 0;

  var serial = uart.open(configuration, function(err) {
    checkError(err);
    console.log('open done');

    serial.on('data', function(data) {
      console.log('read result: ' + data.toString());
      read = 1;

      if (read && write) {
        serial.close();
        console.log('close done');
      }
    });

    serial.write('Hello there?\n\r', function(err) {
      checkError(err);
      console.log('write done');
      write = 1;

      if (read && write) {
        serial.close(function(err) {
          checkError(err);
        });
        console.log('close done');
      }
    });
  });
}
