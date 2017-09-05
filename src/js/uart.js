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

var EventEmitter = require('events').EventEmitter;
var util = require('util');
var uart = process.binding(process.binding.uart);

// VALIDATION ARRAYS
var BAUDRATE = [0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400
                , 4800, 9600, 19200, 38400, 57600, 115200, 230400];
var DATABITS = [5, 6, 7, 8];

var defaultConfiguration = {
  baudRate: 9600,
  dataBits: 8
};


function Uart() {
  if (!(this instanceof Uart)) {
    return new Uart();
  }
}

Uart.prototype.open = function(configuration, callback) {
  return uartPortOpen(configuration, callback);
};


function uartPortOpen(configuration, callback) {
  var _binding = null;

  function UartPort(configuration, callback) { //constructor
    var self = this;

    if (util.isObject(configuration)) {
      if (!util.isString(configuration.device)) {
        throw new TypeError(
          'Bad configuration - device is mandatory and should be String');
      }
    } else {
      throw new TypeError('Bad arguments - configuration should be Object');
    }

    // validate baud rate
    if (configuration.baudRate !== undefined) {
      if (BAUDRATE.indexOf(configuration.baudRate) === -1) {
        throw new TypeError("Invalid 'baudRate': " + configuration.baudRate);
      }
    } else {
      configuration.baudRate = defaultConfiguration.baudRate;
    }

    // validate data bits
    if (configuration.dataBits !== undefined) {
      if (DATABITS.indexOf(configuration.dataBits) === -1) {
        throw new TypeError("Invalid 'databits': " + configuration.dataBits);
      }
    } else {
      configuration.dataBits = defaultConfiguration.dataBits;
    }

    EventEmitter.call(this);

    _binding = new uart(configuration, this, function(err) {
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

  util.inherits(UartPort, EventEmitter);

  UartPort.prototype.write = function(buffer, callback) {
    var self = this;

    if (_binding === null) {
      throw new Error('UART port is not opened');
    }

    _binding.write(buffer, function(err) {
      util.isFunction(callback) && callback.call(self, err);
    });
  };

  UartPort.prototype.writeSync = function(buffer) {
    var self = this;

    if (_binding === null) {
      throw new Error('UART port is not opened');
    }

    _binding.write(buffer);
  };

  UartPort.prototype.close = function(callback) {
    var self = this;

    if (_binding === null) {
      throw new Error('UART port is not opened');
    }

    _binding.close(function(err) {
      util.isFunction(callback) && callback.call(self, err);
      _binding = null;
    });
  };

  UartPort.prototype.closeSync = function() {
    if (_binding === null) {
      throw new Error('UART port is not opened');
    }

    _binding.close();
    _binding = null;
  };

  return new UartPort(configuration, callback);
}


module.exports = Uart;
