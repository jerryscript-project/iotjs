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
var Gpio = require('gpio');
var gpio = new Gpio();

var openCnt = 0;

var GPIO_PIN_OUT = 50,
    GPIO_PIN_IN = 48;

var gpioOut = null,
    gpioIn = null;


switch (process.platform) {
    case 'tizenrt':
        GPIO_PIN_OUT = 50;
        GPIO_PIN_IN = 48;
        break;
    case 'nuttx':
        break;
    case 'linux':
    default:
        if (process.iotjs.board === 'RP2') {
            GPIO_PIN_OUT = 23;
            GPIO_PIN_IN = 24;
        } else {
            assert.fail('Test on platform not supported');
        }
        break;
}

function test() {
    assert(gpioIn.readSync() == 0, 'input pin state low on start');
    gpioOut.writeSync(1);
    assert(gpioOut.readSync() == 1, 'out pin state high set for echo service');
    setTimeout(function () {
        assert(gpioIn.readSync() == 1, 'input pin state high by echo service');
        gpioIn.closeSync();
        gpioOut.closeSync();
    }, 500);
}

function openCallback(err) {
    if (err) {
        assert.fail(err);
    } else {
        openCnt++;
        if (openCnt === 2) {
            test();
        }
    }
}

gpioOut = gpio.open({
    pin: GPIO_PIN_OUT,
    direction: gpio.DIRECTION.OUT,
    mode: gpio.MODE.NONE
}, openCallback);

gpioIn = gpio.open({
    pin: GPIO_PIN_IN,
    direction: gpio.DIRECTION.IN,
    mode: gpio.MODE.NONE
}, openCallback);


