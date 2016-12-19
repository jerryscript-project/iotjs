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
var spi = process.binding(process.binding.spi);

var defaultOptions = {
  mode : 0,
  chipSelect : 0,
  maxSpeed : 500000,
  bitsPerWord : 8,
  bitOrder : 0,
  loopback : false
};

function checkOptionArgs(options) {
  var result = {
    result : false,
    message : null
  };

  var mode = options.mode;
  if (!util.isUndefined(mode)) {
    if (mode < 0 || mode > 3) {
      result.message = 'Bad arguments - mode';
      return result;
    }
  }

  var cs = options.chipSelect;
  if (!util.isUndefined(cs)) {
    if (cs != 0 && cs != 1) {
      result.message = 'Bad arguments - mode';
      return result;
    }
  }

  var bitsPerWord = options.bitsPerWord;
  if (!util.isUndefined(bitsPerWord)) {
    if (bitsPerWord != 8 && bitsPerWord != 9) {
      result.message = 'Bad arguments - bitsPerWord';
      return result;
    }
  }

  var bitOrder = options.bitOrder;
  if (!util.isUndefined(bitOrder)) {
    if (bitOrder != 0 && bitOrder != 1) {
      result.message = 'Bad arguments - bitOrder';
      return result;
    }
  }

  var loopback = options.loopback;
  if (!util.isUndefined(loopback) && !util.isBoolean(loopback)) {
    result.message = 'Bad arguments - loopback';
    return result;
  }

  result.result = true;
  return result;
}

// SPI(pin[, options][, callback])
function SPI(spiNumber, options, callback) {
  var self = this;

  if (!(this instanceof SPI)) {
    return new SPI(spiNumber, options, callback);
  }

  // Check arguments.
  if (util.isObject(spiNumber)) {
    self._device = spiNumber.device || 0;
    self._cs = spiNumber.cs || 0;
  } else if (util.isNumber(spiNumber)) {
    self._device = spiNumber;
    self._cs = 0;
  } else {
    throw new TypeError('Bad arguments - spiNumber');
  }

  if (util.isFunction(options)) {
    callback = options;
  }
  if (util.isObject(options)) {
    var checkResult = checkOptionArgs(options);
    if (checkResult.result == null) {
      throw new Error(checkResult.message);
    }
  } else {
    options = {};
  }

  // Copy default options
  self._options = {};
  for (var name in defaultOptions) {
    if (defaultOptions.hasOwnProperty(name)) {
      self._options[name] = util.isUndefined(options[name]) ?
        defaultOptions[name] : options[name];
    }
  }

  var afterCallback = function(err) {
    util.isFunction(callback) && callback(err);
  };

  self._spi = new spi(self._device, self._cs, self._options, afterCallback);
}

// spi.transfer(txBuffer, rxBuffer[, callback])
SPI.prototype.transfer = function(txBuffer, rxBuffer, callback) {
  if (util.isUndefined(txBuffer.length) || util.isUndefined(rxBuffer.length)
      || txBuffer.length <= 0 || rxBuffer.length <= 0
      || txBuffer.length != rxBuffer.length) {
    throw new Error('Bad arguments - buffer length');
  }

  var rxLength = rxBuffer.length;
  var afterCallback = function(err, buffer) {
    for (var i = 0; i < rxLength; i++) {
      rxBuffer[i] = buffer[i];
    }

    util.isFunction(callback) && callback(err, buffer);
  };

  if (util.isArray(txBuffer) && util.isArray(rxBuffer)) {
    this._spi.transferArray(txBuffer, rxBuffer, afterCallback);
  } else if (util.isBuffer(txBuffer) && util.isBuffer(rxBuffer)) {
    this._spi.transferBuffer(txBuffer, rxBuffer, afterCallback);
  } else {
    throw new TypeError('Bad arguments - buffer');
  }
};

// spi.write(txBuffer[, callback])
SPI.prototype.write = function(txBuffer, callback) {
  if (util.isUndefined(txBuffer) || txBuffer.length <= 0) {
    throw new Error('Bad arguments - buffer length');
  }

  var afterCallback = function(err, buffer) {
    util.isFunction(callback) && callback(err, buffer);
  };

  var rxBuffer = null;
  if (util.isArray(txBuffer)) {
    rxBuffer = new Array(txBuffer.length);
    this._spi.transferArray(txBuffer, rxBuffer, afterCallback);
  } else if (util.isBuffer(txBuffer)) {
    rxBuffer = new Buffer(txBuffer.length);
    this._spi.transferBuffer(txBuffer, rxBuffer, afterCallback);
  } else {
    throw new TypeError('Bad arguments - buffer');
  }
};

// spi.read(rxBuffer[, callback])
SPI.prototype.read = function(rxBuffer, callback) {
  if (util.isUndefined(rxBuffer) || rxBuffer.length <= 0) {
    throw new Error('Bad arguments - buffer length');
  }
  var rxLength = rxBuffer.length;
  var afterCallback = function(err, buffer) {
    for (var i = 0; i < rxLength; i++) {
      rxBuffer[i] = buffer[i];
    }

    util.isFunction(callback) && callback(err, buffer);
  };

  var txBuffer = null;
  if (util.isArray(rxBuffer)) {
    txBuffer = new Array(rxBuffer.length);
    this._spi.transferArray(txBuffer, rxBuffer, afterCallback);
  } else if (util.isBuffer(rxBuffer)) {
    txBuffer = new Buffer(rxBuffer.length);
    this._spi.transferBuffer(txBuffer, rxBuffer, afterCallback);
  } else {
    throw new TypeError('Bad arguments - buffer');
  }
};

// spi.setOption(options)
SPI.prototype.setOption = function(options) {
  if (!util.isObject(options)) {
    throw new TypeError('Bad arguments - options');
  }

  var checkResult = checkOptionArgs(options);
  if (!checkResult.result) {
    throw new Error(checkResult.message);
  }

  for (var name in options) {
    if (options.hasOwnProperty(name) && !util.isUndefined(options[name])) {
      this._options[name] = options[name];
    }
  }

  var err = this._spi.setOption(options);
  if (err) {
    throw err;
  }
};

// spi.unexport([callback])
SPI.prototype.unexport = function(callback) {
  return this._spi.unexport(function(err) {
    util.isFunction(callback) && callback(err);
  });
};

module.exports = SPI;
