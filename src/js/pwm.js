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


function Pwm() {
  if (!(this instanceof Pwm)) {
    return new Pwm();
  }
}

Pwm.prototype.open = function(configuration, callback) {
  return pwmPinOpen(configuration, callback);
};


function pwmPinOpen(configuration, callback) {
  var _binding = null;

  function PwmPin(configuration, callback) {
    var self = this;
    self._configuration = {};

    if (util.isObject(configuration)) {
      if (process.platform === 'linux') {
        if (util.isNumber(configuration.chip)) {
          self._configuration.chip = configuration.chip
        } else {
          self._configuration.chip = 0;
        }
      }

      if (!util.isNumber(configuration.pin)) {
        throw new TypeError(
          'Bad configuration - pin is mandatory and should be Number');
      } else {
        self._configuration.pin = configuration.pin;
      }
    } else {
      throw new TypeError('Bad arguments - configuration should be Object')
    }

    // validate configuration
    var dutyCycle = configuration.dutyCycle;
    var period = configuration.period;
    if (!util.isNumber(period) && util.isNumber(configuration.frequency)) {
      period = 1.0 / configuration.frequency;
    }

    if (util.isNumber(dutyCycle) && dutyCycle >= 0.0 && dutyCycle <= 1.0 &&
      util.isNumber(period) && util.isFinite(period) && period > 0) {
      self._configuration.dutyCycle = dutyCycle;
      self._configuration.period = period;
    }

    _binding = new pwm(self._configuration, function(err) {
      util.isFunction(callback) && callback.call(self, err);
    });

    process.on('exit', (function(self) {
      return function() {
        if (_binding !== null) {
          self.closeSync();
        }
      };
    })(this));
  }

  PwmPin.prototype._validatePeriod = function(period) {
    if (!util.isNumber(period)) {
      throw new TypeError('Period is not a number(' + typeof(period) + ')');
    } else if (period < 0) {
      throw new RangeError('Period(' + period + ') is negative');
    }
    return true;
  };

  PwmPin.prototype._validateFrequency = function(frequency) {
    if (!util.isNumber(frequency)) {
      throw new TypeError('Frequency is not a number(' +
        typeof(frequency) + ')');
    } else if (frequency <= 0) {
      throw RangeError('Nonpositivie frequency of ' + frequency);
    }
    return true;
  };

  PwmPin.prototype._validateDutyCycle = function(dutyCycle) {
    if (!util.isNumber(dutyCycle)) {
      throw TypeError('DutyCycle is not a number(' + typeof(dutyCycle) + ')');
    } else if (dutyCycle < 0.0 || dutyCycle > 1.0) {
      throw RangeError('DutyCycle of ' + dutyCycle + ' out of bounds [0..1]');
    }
    return true;
  };

  PwmPin.prototype.setPeriod = function(period, callback) {
    var self = this;

    if (_binding === null) {
      throw new Error('Pwm pin is not opened');
    }

    if (this._validatePeriod(period)) {
      _binding.setPeriod(period, function(err) {
        util.isFunction(callback) && callback.call(self, err);
      });
    }
  };

  PwmPin.prototype.setPeriodSync = function(period) {
    if (_binding === null) {
      throw new Error('Pwm pin is not opened');
    }

    if (this._validatePeriod(period)) {
      _binding.setPeriod(period);
    }
  };

  PwmPin.prototype.setFrequency = function(frequency, callback) {
    var self = this;

    if (_binding === null) {
      throw new Error('Pwm pin is not opened');
    }

    if (this._validateFrequency(frequency)) {
      _binding.setPeriod(1.0 / frequency, function(err) {
        util.isFunction(callback) && callback.call(self, err);
      });
    }
  };

  PwmPin.prototype.setFrequencySync = function(frequency) {
    if (_binding === null) {
      throw new Error('Pwm pin is not opened');
    }

    if (this._validateFrequency(frequency)) {
      _binding.setPeriod(1.0 / frequency);
    }
  };

  PwmPin.prototype.setDutyCycle = function(dutyCycle, callback) {
    var self = this;

    if (_binding === null) {
      throw new Error('Pwm pin is not opened');
    }

    // Check arguments.
    if (this._validateDutyCycle(dutyCycle)) {
      _binding.setDutyCycle(dutyCycle, function(err) {
        util.isFunction(callback) && callback.call(self, err);
      });
    }
  };

  PwmPin.prototype.setDutyCycleSync = function(dutyCycle) {
    if (_binding === null) {
      throw new Error('Pwm pin is not opened');
    }

    // Check arguments.
    if (this._validateDutyCycle(dutyCycle)) {
      _binding.setDutyCycle(dutyCycle);
    }
  };

  PwmPin.prototype.setEnable = function(enable, callback) {
    var self = this;

    if (_binding === null) {
      throw new Error('Pwm pin is not opened');
    }

    // Check arguments.
    if (!util.isNumber(enable) && !util.isBoolean(enable)) {
      throw new TypeError('enable is of type ' + typeof(enable));
    }

    _binding.setEnable(!!enable, function(err) {
      util.isFunction(callback) && callback.call(self, err);
    });
  };

  PwmPin.prototype.setEnableSync = function(enable) {
    if (_binding === null) {
      throw new Error('Pwm pin is not opened');
    }

    // Check arguments.
    if (!util.isNumber(enable) && !util.isBoolean(enable)) {
      throw new TypeError('enable is of type ' + typeof(enable));
    }

    _binding.setEnable(!!enable);
  };

  PwmPin.prototype.close = function(callback) {
    var self = this;

    if (_binding === null) {
      throw new Error('Pwm pin is not opened');
    }

    _binding.close(function(err) {
      util.isFunction(callback) && callback.call(self, err);
      _binding = null;
    });
  };

  PwmPin.prototype.closeSync = function() {
    if (_binding === null) {
      throw new Error('Pwm pin is not opened');
    }

    _binding.close();
    _binding = null;
  };

  return new PwmPin(configuration, callback);
}


module.exports = Pwm;
