/* Copyright (c) 2013, Kelly Korevec <korevec@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the author nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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

/* This file includes all APIs in 'node-i2c'(https://github.com/kelly/node-i2c).
 * Some functions are translated from coffee script(i2c.coffee) in 'node-i2c'.
 */

var util = require('util');
var i2c = process.binding(process.binding.i2c);

function I2C() {
  if (!(this instanceof I2C)) {
    return new I2C();
  }
}

I2C.prototype.open = function(configurable, callback) {
  return i2cBusOpen(configurable, callback);
};


function i2cBusOpen(configurable, callback) {
  var _binding = null;

  function I2CBus(configurable, callback) {
    var error;
    if (!util.isObject(configurable)) {
      error = new TypeError(
          'configurable: expected Object, received '
          + typeof configurable);
    } else {
      if (process.platform === 'linux') {
        if (!util.isString(configurable.device)) {
          error = new TypeError(
              'configurable.device: string expected, received ' +
              typeof configurable.device);
        }
      } else if (process.platform === 'nuttx' ||
                 process.platform === 'tizen' ||
                 process.platform === 'tizenrt') {
        if (!util.isNumber(configurable.device)) {
          error = new TypeError(
              'configurable.device: number expected, received ' +
              typeof configurable.device);
        }
      } else {
        error = new RangeError('Unsupported platform: ' + process.platform);
      }
    }
    if (error) throw error;

    if (this.validateAddress(configurable.address)) {
      this.device = configurable.device;
      this.address = configurable.address;
    }

    var boundCallback =
        function(err) {
          if (!err) this.setAddress(configurable.address);
          util.isFunction(callback) && callback(err);
        }.bind(this);

    _binding = new i2c(configurable.device, boundCallback);
  }

  I2CBus.prototype.close = function() {
    _binding.close();
  };

  I2CBus.prototype.validateAddress = function(address) {
    // This is incomplete - for device-related transactions valid addresses
    // exclude 0000xxx and 1111xxx patterns, leaving valid range of 8..119
    // For bus-wide transactions all addresses are valid.
    // Note: 8/10 bit addressing is not covered here.
    if (!util.isNumber(address))
      throw new TypeError('Number expected, ' + typeof address + ' received');
    if (address < 0 || address > 127)
      throw new RangeError('value ' + address + ' out of range [0..127]');
    return true;
  };

  I2CBus.prototype.validateByte = function(byte) {
    if (!util.isNumber(byte))
      throw new TypeError('byte expected, ' + typeof byte + ' received');
    if (byte < 0 || byte > 255)
      throw new RangeError('value ' + byte + ' out of range [0..255]');
    return true;
  }

  I2CBus.prototype.validateArrayOfBytes = function(array) {
    if (util.isArray(array)) {
      var maxIdx = array.length;
      // Try-catch maybe to embed context (index, what else?) into the error?
      for (var idx = 0; idx < maxIdx; ++idx) this.validateByte(array[idx]);
    } else throw new TypeError('array expected, ' + typeof array + ' received');
    return true;
  }

  I2CBus.prototype.setAddress = function(address, callback) {
    if (this.validateAddress(address)) {
      this.address = address;
      util.isFunction(callback) && callback();
    }
  };

  I2CBus.prototype.write = function(array, callback) {
    if (this.validateArrayOfBytes(array)) {
      _binding.setAddress(this.address);
      _binding.write(array, function(err) {
        util.isFunction(callback) && callback(err);
      });
    }
  };

  I2CBus.prototype.read = function(length, callback) {
    if (this.validateLength(length)) {
      _binding.setAddress(this.address);
      _binding.read(length, function(err, data) {
        util.isFunction(callback) && callback(err, data);
      });
    }
  };

  I2CBus.prototype.validateLength = function(length) {
    if (!util.isNumber(length)) {
      throw new TypeError('Number expected, ' + typeof length + ' received');
    } else if (length < 0) {
      throw new RangeError('length ' + length + ' < 0');
    }
    return true;
  };

  return new I2CBus(configurable, callback);
}

module.exports = I2C;
