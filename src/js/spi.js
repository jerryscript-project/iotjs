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

var defaultConfiguration = {
  mode : spi.MODE[0],
  chipSelect : spi.CHIPSELECT.NONE,
  maxSpeed : 500000,
  bitsPerWord : 8,
  bitOrder : spi.BITORDER.MSB,
  loopback : false
};


function Spi() {
  if (!(this instanceof Spi)) {
    return new Spi();
  }
}

Spi.prototype.open = function(configuration, callback) {
  return new SpiBus(configuration, callback);
};

Spi.prototype.MODE = spi.MODE;
Spi.prototype.CHIPSELECT = spi.CHIPSELECT;
Spi.prototype.BITORDER = spi.BITORDER;


// SpiBus(configuration[, callback])
function SpiBus(configuration, callback) {
  var self = this;

  // validate device
  if (util.isObject(configuration)) {
    if (!util.isString(configuration.device)) {
      throw new TypeError('Bad configuration - device is mandatory and String');
    }
  } else {
    throw new TypeError('Bad arguments - configuration should be Object');
  }

  // validate mode
  var mode = configuration.mode;
  if (!util.isUndefined(mode)) {
    if (mode !== spi.MODE[0] && mode !== spi.MODE[1] &&
        mode !== spi.MODE[2] && mode !== spi.MODE[3]) {
      throw new TypeError(
        'Bad arguments - mode should be MODE[0], [1], [2] or [3]');
    }
  } else {
    configuration.mode = defaultConfiguration.mode;
  }

  // validate chip-select
  var chipSelect = configuration.chipSelect;
  if (!util.isUndefined(chipSelect)) {
    if (chipSelect != spi.CHIPSELECT.NONE &&
        chipSelect != spi.CHIPSELECT.HIGH) {
      throw new TypeError(
        'Bad arguments - chipSelect should be CHIPSELECT.NONE or HIGH');
    }
  } else {
    configuration.chipSelect = defaultConfiguration.chipSelect;
  }

  // validate max speed
  if (!util.isUndefined(configuration.maxSpeed)) {
    if (!util.isNumber(configuration.maxSpeed)) {
      throw new TypeError('Bad arguments - maxSpeed should be Number');
    }
  } else {
    configuration.maxSpeed = defaultConfiguration.maxSpeed
  }

  // validate bits per word
  var bitsPerWord = configuration.bitsPerWord;
  if (!util.isUndefined(bitsPerWord)) {
    if (bitsPerWord != 8 && bitsPerWord != 9) {
      throw new TypeError('Bad arguments - bitsPerWord should be 8 or 9');
    }
  } else {
    configuration.bitsPerWord = defaultConfiguration.bitsPerWord;
  }

  // validate bit order
  var bitOrder = configuration.bitOrder;
  if (!util.isUndefined(bitOrder)) {
    if (bitOrder != spi.BITORDER.MSB && bitOrder != spi.BITORDER.LSB) {
      throw new TypeError(
        'Bad arguments - bitOrder should be BITORDER.MSB or LSB');
    }
  } else {
    configuration.bitOrder = defaultConfiguration.bitOrder;
  }

  // validate loopback
  var loopback = configuration.loopback;
  if (!util.isUndefined(loopback)) {
    if (!util.isBoolean(loopback)) {
      throw new TypeError('Bad arguments - loopback should be Boolean');
    }
  } else {
    configuration.loopback = defaultConfiguration.loopback;
  }

  this._binding = new spi.Spi(configuration, function(err) {
   util.isFunction(callback) && callback.call(self, err);
  });

  process.on('exit', (function(self) {
   return function() {
     if (!util.isNull(self._binding)) {
       self.closeSync();
     }
   };
  })(this));
}

// spibus.transfer(txBuffer, rxBuffer[, callback])
SpiBus.prototype.transfer = function(txBuffer, rxBuffer, callback) {
  var self = this;

  if (util.isNull(this._binding)) {
    throw new Error('SPI bus is not opened');
  }

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

    util.isFunction(callback) && callback.call(self, err);
  };

  if (util.isArray(txBuffer) && util.isArray(rxBuffer)) {
    this._binding.transferArray(txBuffer, rxBuffer, afterCallback);
  } else if (util.isBuffer(txBuffer) && util.isBuffer(rxBuffer)) {
    this._binding.transferBuffer(txBuffer, rxBuffer, afterCallback);
  } else {
    throw new TypeError('Bad arguments - buffer should be Array or Buffer');
  }
};

// spibus.transferSync(txBuffer, rxBuffer)
SpiBus.prototype.transferSync = function(txBuffer, rxBuffer) {
  if (util.isNull(this._binding)) {
    throw new Error('SPI bus is not opened');
  }

  if (util.isUndefined(txBuffer.length) || util.isUndefined(rxBuffer.length)
    || txBuffer.length <= 0 || rxBuffer.length <= 0
    || txBuffer.length != rxBuffer.length) {
    throw new Error('Bad arguments - buffer length');
  }

  var data = null;
  if (util.isArray(txBuffer) && util.isArray(rxBuffer)) {
    data = this._binding.transferArray(txBuffer, rxBuffer);
  } else if (util.isBuffer(txBuffer) && util.isBuffer(rxBuffer)) {
    data = this._binding.transferBuffer(txBuffer, rxBuffer);
  } else {
    throw new TypeError('Bad arguments - buffer should be Array or Buffer');
  }

  if (data !== null && (util.isArray(data) || util.isBuffer(data)) &&
    data.length === rxBuffer.length) {
    for (var i = 0; i < rxBuffer.length; i++) {
      rxBuffer[i] = data[i];
    }
  } else {
    throw new Error('Spi Transfer Error');
  }
};

// spi.close([callback])
SpiBus.prototype.close = function(callback) {
  var self = this;

  if (util.isNull(this._binding)) {
    throw new Error('SPI bus is not opened');
  }

  return this._binding.close(function(err) {
    util.isFunction(callback) && callback.call(self, err);
  });
};

// spibus.closeSync()
SpiBus.prototype.closeSync = function() {
  if (util.isNull(this._binding)) {
    throw new Error('SPI bus is not opened');
  }

  return this._binding.close();
};

module.exports = Spi;
