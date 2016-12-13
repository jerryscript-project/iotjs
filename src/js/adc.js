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


// ADC(pin[, callback])
function ADC(pin, callback) {
  if (!util.isNumber(pin)) {
    throw new TypeError('Bad arguments - pin');
  }

  this._pin = pin;

  adc.export(this._pin, function(err) {
    if (util.isFunction(callback)) {
      callback(err);
    }
  });
}

// adc.read([callback])
ADC.prototype.read = function(callback) {
  return adc.read(this._pin, function(err, value) {
    if (util.isFunction(callback)) {
      callback(err, value);
    }
  });
};

// adc.readSync()
ADC.prototype.readSync = function() {
  return adc.readSync(this._pin);
};

// adc.unexport([callback])
ADC.prototype.unexport = function(callback) {
  return adc.unexport(this._pin, function(err) {
    if (util.isFunction(callback)) {
      callback(err);
    }
  });
};

module.exports = ADC;
