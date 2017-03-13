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

var util = require('util');
var pwm = process.binding(process.binding.pwm);


function PwmError(code, operation, message) {
  this.name = 'PwmError';
  this.code = code;
  this.operation = operation;
  this.message = operation + ': ' + message;
}

util.inherits(PwmError, Error);


function CreatePwmError(operation, errno) {
  if (errno == pwm.kPwmErrOk) {
    return null;
  }

  switch (errno) {
  case pwm.kPwmErrExport:
    return new PwmError(errno, operation, 'Failed to export PWM');
  case pwm.kPwmErrUnexport:
    return new PwmError(errno, operation, 'Failed to unexport PWM');
  case pwm.kPwmErrEnable:
    return new PwmError(errno, operation, 'Failed to enable PWM');
  case pwm.kPwmErrWrite:
    return new PwmError(errno, operation, 'Failed to write value');
  case pwm.kPwmErrSys:
    return new PwmError(errno, operation, 'System error');
  }

  return new PwmError(errno, operation, 'Unknown error');
}


// PWM(pin[, options][, callback])
function PWM(pin, options, callback) {
  var self = this;

  if (!(this instanceof PWM)) {
    return new PWM(pin, options, callback);
  }

  if (!util.isNumber(pin)) {
    throw new TypeError('pin is not a number(' + typeof(pin) + ')');
  }

  self._options = {};
  self._pin = pin;

  // Check arguments.
  if (util.isFunction(options)) {
    callback = options;
  } else if (util.isObject(options)) {
    var dutyCycle = options.dutyCycle;
    var period = options.period;
    if (!util.isNumber(period) && util.isNumber(options.frequency)) {
      period = 1.0 / options.frequency;
    }

    if (util.isNumber(dutyCycle) && dutyCycle >= 0.0 && dutyCycle <= 1.0 &&
        util.isNumber(period) && util.isFinite(period) && period > 0) {
      self._options.dutyCycle = dutyCycle;
      self._options.period = period;
    }
  }

  pwm.export(self._pin, self._options, function(err) {
    var errObj = CreatePwmError('PWM()', err);

    if (util.isFunction(callback)) {
      callback(errObj);
    }
  });
}

PWM.prototype._validatePeriod = function(period) {
  if (!util.isNumber(period)) {
    throw new TypeError('Period is not a number(' + typeof(period) + ')');
  } else if (period < 0) {
    throw new RangeError('Period(' + period + ') is negative');
  }
  return true;
};

PWM.prototype._validateDutyCycle = function(dutyCycle) {
  if (!util.isNumber(dutyCycle)) {
    throw TypeError('DutyCycle is not a number(' + typeof(dutyCycle) + ')');
  } else if (dutyCycle < 0.0 || dutyCycle > 1.0) {
    throw RangeError('DutyCycle of ' + dutyCycle + ' out of bounds [0..1]');
  }
  return true;
};

PWM.prototype._validateOptions = function() {
  return util.isNumber(this._options.period) &&
    util.isNumber(this._options.dutyCycle);
};

PWM.prototype._doSetPeriod = function(callback) {
  if (this._validateOptions()) {
    return pwm.setPeriod(this._pin, this._options, function(err) {
      var errObj = CreatePwmError('setPeriod()', err);

      if (util.isFunction(callback)) {
        callback(errObj);
      }
    });
  } else return false;
};

// pwm.setPeriod(period[, callback])
PWM.prototype.setPeriod = function(period, callback) {
  // Check arguments.
  if (this._validatePeriod(period)) {
    this._options.period = period;
    return this._doSetPeriod(callback);
  }
};

// pwm.setFrequency(frequency[, callback])
PWM.prototype.setFrequency = function(frequency, callback) {
  if (util.isNumber(frequency)) {
    if (frequency > 0) {
      return this.setPeriod(1.0 / frequency, callback);
    } else {
      throw RangeError('Nonpositivie frequency of ' + frequency);
    }
  } else {
    throw new TypeError('Frequency is not a number(' +
                        typeof(frequency) + ')');
  }
};

PWM.prototype._doSetDutyCycle = function(callback) {
  if (this._validateOptions()) {
    return pwm.setDutyCycle(this._pin, this._options, function(err) {
      var errObj = CreatePwmError('setDutyCycle()', err);

      if (util.isFunction(callback)) {
        callback(errObj);
      }
    });
  } else return false;
};
// pwm.setDutyCycle(dutyCycle[, callback])
PWM.prototype.setDutyCycle = function(dutyCycle, callback) {
  // Check arguments.
  if (this._validateDutyCycle(dutyCycle)) {
    this._options.dutyCycle = dutyCycle;
    return this._doSetDutyCycle(callback);
  }
};

// pwm.setEnable(enable[, callback])
PWM.prototype.setEnable = function(enable, callback) {
  // Check arguments.
  if (!util.isNumber(enable) && !util.isBoolean(enable)) {
    throw new TypeError('enable is of type ' + typeof(enable));
  }

  return pwm.setEnable(this._pin, !!enable, function(err) {
    var errObj = CreatePwmError('setEnable()', err);

    if (util.isFunction(callback)) {
      callback(errObj);
    }
  });
};

// pwm.unexport([callback])
PWM.prototype.unexport = function(callback) {
  return pwm.unexport(this._pin, function(err) {
    var errObj = CreatePwmError('unexport()', err);

    if (util.isFunction(callback)) {
      callback(errObj);
    }
  });
};

module.exports = PWM;
