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
    throw new TypeError('Bad arguments - pin');
  }

  self._options = {};
  self._pin = pin;

  // Check arguments.
  if (util.isFunction(options)) {
    callback = options;
    self._options.dutyCycle = -1;
    self._options.period = -1;
  } else if (util.isObject(options)) {
    if (util.isNumber(options.dutyCycle)) {
      self._options.dutyCycle = options.dutyCycle;
    } else {
      self._options.dutyCycle = -1;
    }

    if (util.isNumber(options.period)) {
      self._options.period = options.period;
    } else {
      self._options.dutyCycle = -1;
    }

    if (util.isNumber(options.frequency)) {
      self._options.period = options.frequency;
    } else {
      self._options.dutyCycle = -1;
    }
  }

  pwm.export(self._pin, self._options, function (err) {
    var errObj = CreatePwmError('PWM()', err);

    if (util.isFunction(callback)) {
      callback(errObj);
    }
  });
}

// pwm.setPeriod(period[, callback])
PWM.prototype.setPeriod = function (period, callback) {
  var self = this;
  // Check arguments.
  if (!util.isNumber(period)) {
    throw new TypeError('Bad arguments - period');
  }

  self._options.period = period;

  return pwm.setPeriod(self._pin, self._options, function (err) {
    var errObj = CreatePwmError('setPeriod()', err);

    if (util.isFunction(callback)) {
      callback(errObj);
    }
  });
};

// pwm.setFrequency(frequency[, callback])
PWM.prototype.setFrequency = function (frequency, callback) {
  var self = this;
  // Check arguments.
  if (!util.isNumber(frequency)) {
    throw new TypeError('Bad arguments - frequency');
  }

  self._options.period = frequency;

  return pwm.setFrequency(self._pin, self._options, function (err) {
    var errObj = CreatePwmError('setFrequency()', err);

    if (util.isFunction(callback)) {
      callback(errObj);
    }
  });
};

// pwm.setDutyCycle(dutyCycle[, callback])
PWM.prototype.setDutyCycle = function (dutyCycle, callback) {
  var self = this;
  // Check arguments.
  if (!util.isNumber(dutyCycle)) {
    throw new TypeError('Bad arguments - dutyCycle');
  }

  if (util.isNullOrUndefined(self._options.period)) {
    throw new Error('setDutyCycle() - first set period, ' +
      'before calling this function');
  }

  self._options.dutyCycle = dutyCycle;

  return pwm.setDutyCycle(self._pin, self._options, function (err) {
    var errObj = CreatePwmError('setDutyCycle()', err);

    if (util.isFunction(callback)) {
      callback(errObj);
    }
  });
};

// pwm.setEnable(enable[, callback])
PWM.prototype.setEnable = function (enable, callback) {
  var self = this;
  // Check arguments.
  if (!util.isNumber(enable) && !util.isBoolean(enable)) {
    throw new TypeError('Bad arguments - enable');
  }

  return pwm.setEnable(self._pin, !!enable, function (err) {
    var errObj = CreatePwmError('setEnable()', err);

    if (util.isFunction(callback)) {
      callback(errObj);
    }
  });
};

// pwm.unexport([callback])
PWM.prototype.unexport = function (callback) {
  return pwm.unexport(this._pin, function (err) {
    var errObj = CreatePwmError('unexport()', err);

    if (util.isFunction(callback)) {
      callback(errObj);
    }
  });
};

module.exports = PWM;
