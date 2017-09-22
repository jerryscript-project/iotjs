/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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
var UART = require('uart');
var uart = new UART();
var DATA = 'hello world';
var DEVICE = '';

switch (process.platform) {
    case 'tizenrt':
        DEVICE = '/dev/ttyS1';
        break;
    case 'nuttx':
        break;
    case 'linux':
        if (process.iotjs.board === 'RP2') {
            DEVICE = '/dev/serial0';
        } else {
            assert.fail('Test on platform not supported');
        }
        break;
}

console.log('uart: opening');
var res = uart.open({
    device: DEVICE,
    baudRate: 115200,
    dataBits: 8
}, function (err) {
    var onData = function (data) {
        console.log('uart: received ' + data);
        setTimeout(function () { // @fixme: onData caller fails test
                                 // when exception thrown
            assert(data == DATA, 'uart: response was ok');
        }, 0);
        console.log('uart: closing');
        res.closeSync();
    };

    if (err) {
        assert(false, 'uart: error ' + err);
    } else {
        console.log('uart: opened');
        res.on('data', onData);
        console.log('uart: writing ' + DATA);
        res.write(DATA + '\n', function (err) {
            if (err) {
                assert(false, 'uart: error ' + err);
            }
        })
    }
});
