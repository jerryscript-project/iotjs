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
var adc = process.binding(process.binding.adc);


function Adc() {
  if (!(this instanceof Adc)) {
    return new Adc();
  }
}

Adc.prototype.open = function(configuration, callback) {
  return adcPinOpen(configuration, callback);
};


function adcPinOpen(configuration, callback) {
  var _binding = null;

  function AdcPin(configuration, callback) {
    var self = this;

    if (util.isObject(configuration)) {
      if (process.platform === 'linux') {
        if (!util.isString(configuration.device)) {
          throw new TypeError(
            'Bad configuration - device is mandatory and should be String');
        }
      } else if (process.platform === 'nuttx') {
        if (!util.isNumber(configuration.pin)) {
          throw new TypeError(
            'Bad configuration - pin is mandatory and should be Number');
        }
      }
    } else {
      throw new TypeError('Bad arguments - configuration should be Object')
    }

    _binding = new adc.Adc(configuration, function(err) {
      util.isFunction(callback) && callback.call(self, err);
    });

    process.on('exit', (function(self) {
      return function() {
        if (!util.isNull(_binding)) {
          self.closeSync();
        }
      };
    })(this));
  }

  AdcPin.prototype.read = function(callback) {
    var self = this;

    if (util.isNull(_binding)) {
      throw new Error('ADC pin is not opened');
    }

    _binding.read(function(err, value) {
      util.isFunction(callback) && callback.call(self, err, value);
    });
  };

  AdcPin.prototype.readSync = function() {
    if (util.isNull(_binding)) {
      throw new Error('ADC pin is not opened');
    }

    return _binding.read();
  };

  AdcPin.prototype.close = function(callback) {
    var self = this;

    if (util.isNull(_binding)) {
      throw new Error('ADC pin is not opened');
    }

    _binding.close(function(err) {
      util.isFunction(callback) && callback.call(self, err);
    });

    _binding = null;
  };

  AdcPin.prototype.closeSync = function() {
    if (util.isNull(_binding)) {
      throw new Error('ADC pin is not opened');
    }

    _binding.close();

    _binding = null;
  };

  return new AdcPin(configuration, callback);
}


module.exports = Adc;
