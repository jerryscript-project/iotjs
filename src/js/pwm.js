/* Copyright 2016 Samsung Electronics Co., Ltd.
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

var util = require('util');
var pwm = process.binding(process.binding.pwm);

// PWM(pin[, options][, callback])
function PWM(pin, options, callback) {
    if (!(this instanceof PWM)) {
        return new PWM(pin, options, callback);
    }

    if (!util.isNumber(pin) && !util.isString(pin)) {
        throw new TypeError('Bad arguments - pin');
    }

    this._pin = pin;

    // Check arguments.
    if (util.isFunction(options)) {
        callback = options;
        options = {};
    } else if (!util.isObject(options)) {
        options = {};
    }

    return pwm.export(this._pin, options, function(err) {
        if (util.isFunction(callback)) {
            callback(err);
        }
    });
}

// pwm.setDutyCycle(dutyCycle[, callback])
PWM.prototype.setDutyCycle = function(dutyCycle, callback) {
    // Check arguments.
    if (!util.isNumber(dutyCycle)) {
        throw new TypeError('Bad arguments - dutyCycle');
    }

    return pwm.setDutyCycle(this._pin, dutyCycle, function(err) {
        if (util.isFunction(callback)) {
            callback(err);
        }
    });
};

// pwm.setPeriod(period[, callback])
PWM.prototype.setPeriod = function(period, callback) {
    // Check arguments.
    if (!util.isNumber(period)) {
        throw new TypeError('Bad arguments - period');
    }

    return pwm.setDutyCycle(this._pin, period, function(err) {
        if (util.isFunction(callback)) {
            callback(err);
        }
    });
};

// pwm.enable(enable[, callback])
PWM.prototype.enable = function(enable, callback) {
    // Check arguments.
    if (!util.isNumber(enable) && !util.isBoolean(enable)) {
        throw new TypeError('Bad arguments - enable');
    }

    return pwm.enable(this._pin, enable, function(err) {
        if (util.isFunction(callback)) {
            callback(err);
        }
    });
};

// pwm.unexport([callback])
PWM.prototype.unexport = function(callback) {
    return pwm.unexport(this._pin, function(err) {
        if (util.isFunction(callback)) {
            callback(err);
        }
    });
};

module.exports = PWM;
