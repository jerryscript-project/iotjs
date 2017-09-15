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
var ADC = require('adc');
//var adc = new ADC();
var gpio = new GPIO();

var adc0 = null;

var ADC_PIN = 0;
var GPIO_ADC_MODE_1 = 32;
var GPIO_ADC_MODE_2 = 30;
var gpioPins = [];
var gpioOpenCounter = 0;

function resetGpio() {
    console.log('adc: reseting gpio');
    gpioPins.forEach(function (pin, index) {
        if (pin) {
            pin.writeSync(0);
        }
    });
}

function endTest() {
    console.log('adc: closing');
    adc0.closeSync();

    resetGpio();

    console.log('adc: closing gpio');
    gpioPins.forEach(function (pin) {
        if (pin) {
            pin.closeSync();
        }
    });
}

// we have to use median filtering
// due tue tizenrt bug which gives
// first few results wrong
function medianRead(adcPort, observations, callback, interval) {
    var results = [],
        read = function () {
            setTimeout(function () {
                adcPort.read(readCallback);
            }, interval);
        },
        readCallback = function (err, value) {
            var half = 0;
            if (err) {
                callback(err);
            } else {
                results.push(value);
            }
            if (results.length === observations) {
                results.sort(function (a, b) { return a - b; });
                half = (results.length / 2) | 0;
                if (results.length % 2) {
                    callback(null, results[half]);
                } else {
                    callback(null, (results[half - 1] + results[half]) / 2);
                }
            } else {
                read();
            }
        }

    if (interval === undefined) {
        interval = 250; // default read interval in ms
    }

    if (observations > 0) {
        read();
    } else {
        callback('number of observations not specified');
    }
}

function runTest() {
    // turn on ADC on echo mode for driving the tests
    // as ADC through a low-pass filter is used
    // for testing the PWM signal
    console.log('adc: starting test');
    resetGpio();
    medianRead(adc0, 5, function (err, value) {
        console.log('adc: 1st read ' + value);
        if (err) {
            assert(!err, 'adc: 1st read error ' + err);
        } else if (value < 100) {
            gpioPins[GPIO_ADC_MODE_1].write(1, function (err) {
                if (err) {
                    assert(!err, 'adc: gpio error ' + err);
                } else {
                    medianRead(adc0, 10, function (err, value) {
                        console.log('adc: 2nd read ' + value);
                        if (err) {
                            assert(!err, 'adc: 2nd read error ' + err);
                        } else if (value > 500 && value < 900) {
                            gpioPins[GPIO_ADC_MODE_1].write(0, function (err) {
                                if (err) {
                                    assert(!err, 'adc: gpio error ' + err);
                                } else {
                                    gpioPins[GPIO_ADC_MODE_2]
                                        .write(1, function(err) {
                                        if (err) {
                                            assert(!err, 'adc: gpio error '
                                                         + err);
                                        } else {
                                            medianRead(adc0,
                                                       10,
                                                       function (err, value) {
                                                console.log('adc: 3rd read '
                                                            + value);
                                                if (err) {
                                                    assert(!err,
                                                           'adc: 3rd read '
                                                            + error ' + err);
                                                } else if (value > 1200) {
                                                    assert(value,
                                                           'adc: proper value '
                                                           + 'was returned, '
                                                           + 'adc ok');
                                                } else {
                                                    assert(false,
                                                           'adc: value for '
                                                           + mode 2 was wrong '
                                                           + value);
                                                }
                                            });
                                        }
                                    });
                                }
                            });
                        } else {
                            assert(false,
                                   'adc: value for mode 1 was wrong ' + value);
                        }
                    });
                }
            })
        } else {
            assert(false, 'adc: starting value was wrong ' + value);
        }
    });
}

function gpioOpenCallback(err) {
    if (err) {
        assert.fail('adc: error, failed to open gpio pin', err);
    } else {
        gpioOpenCounter++;
        console.log('adc: pin opened');
        if (gpioOpenCounter >= 2) {
            runTest();
        }
    }
}

process.on('exit', endTest);

console.log('adc: opening device');
adc0 = new ADC({
    pin: ADC_PIN
}, function (err) {
    assert(!err, 'adc: opened sucessfuly');
    if (err) {
        assert(!err, 'adc error: could not be opened ' + err);
    } else {
        console.log('adc: opening gpio pins for driving test');
        [GPIO_ADC_MODE_1, GPIO_ADC_MODE_2].forEach(function (gpioPin) {
            gpioPins[gpioPin] = gpio.open({
                pin: gpioPin,
                direction: gpio.DIRECTION.OUTPUT,
                mode: gpio.MODE.NONE
            }, gpioOpenCallback);
        });
    }
});
