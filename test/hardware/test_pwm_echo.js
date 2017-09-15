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
var GPIO = require('gpio');
var PWM = require('pwm');
var pwm = new PWM();
var gpio = new GPIO();

var pwm0 = null;

var PWM_PIN = 0;
var GPIO_PWM_ON_PIN = 55;
var GPIO_MODE_FULL_PIN = 54;
var GPIO_PWM_OK_PIN = 53;
var gpioPins = [];
var gpioOpenCounter = 0;

function resetGpio() {
    console.log('pwm: reseting gpio');
    gpioPins.forEach(function (pin, index) {
        if (pin && index !== GPIO_PWM_OK_PIN) {
            pin.writeSync(0);
        }
    });
}

function endTest() {
    console.log('pwm: closing');
    pwm0.closeSync();

    resetGpio();

    console.log('pwm: closing gpio');
    gpioPins.forEach(function (pin) {
        if (pin) {
            pin.closeSync();
        }
    });
}

function runTest() {
    // turn on ADC on echo mode for driving the tests
    // as ADC through a low-pass filter is used
    // for testing the PWM signal
    console.log('pwm: starting test');
    resetGpio();
    gpioPins[GPIO_PWM_ON_PIN].write(1, function (err) {
        if (err) {
            console.error('pwm error: '
                          + 'failed to start adc mode on echo service');
        } else {
            console.log('pwm: enable with half duty cycle');
            pwm0.setEnableSync(true); // enable the pwm with half duty cycle
            setTimeout(function () { // wait for the device to respond
                console.log('pwm: checking echo response');
                gpioPins[GPIO_MODE_FULL_PIN].read(function (err, full) {
                    if (err) {
                        console.error('pwm error: could not read mode signal',
                                      err);
                    } else {
                        console.log('pwm: device responded with', full);
                        assert(full, 'pwm: echo service requested full duty '
                                     + 'cycle');
                        console.log('pwm: changing duty cycle to full');
                        pwm0.setDutyCycleSync(0.99); // 1.0 is fultered out :(
                        setTimeout(function () {
                            console.log('pwm: reading echo service response');
                            gpioPins[GPIO_PWM_OK_PIN].read(function (err, ok) {
                                if (err) {
                                    console.error('pwm error: could not '
                                                  + 'read response from echo '
                                                  + 'service',
                                                  err);
                                } else {
                                    console.log('pwm: device responded with',
                                                ok);
                                    assert(ok,
                                          'pwm: echo service confirmed PWM '
                                          + 'working');
                                }
                            });
                        }, 1000);
                    }
                });
            }, 1100);
        }
    });

}

function gpioOpenCallback(err) {
    if (err) {
        console.error('pwm error: failed to open gpio pin', err);
    } else {
        gpioOpenCounter++;
        console.log('pwm: pin opened');
        if (gpioOpenCounter >= 3) {
            runTest();
        }
    }
}

process.on('exit', endTest);

console.log('pwm: opening device');
pwm0 = pwm.open({
    period: 0.001,
    pin: PWM_PIN,
    dutyCycle: 0.5
}, function (err) {
    assert(!err, 'pwm: opened sucessfuly');
    if (err) {
        console.error('pwm error: could not be opened', err);
    } else {
        console.log('pwm: opening gpio pins for driving test');
        [
         GPIO_PWM_ON_PIN,
         GPIO_MODE_FULL_PIN,
         GPIO_PWM_OK_PIN
        ].forEach(function (gpioPin) {
            gpioPins[gpioPin] = gpio.open({
                pin: gpioPin,
                direction: gpioPin === GPIO_PWM_ON_PIN
                                     ? gpio.DIRECTION.OUTPUT
                                     : gpio.DIRECTION.INPUT,
                mode: gpio.MODE.NONE
            }, gpioOpenCallback);
        });
    }
});
