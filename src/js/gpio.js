/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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
var gpio = process.binding(process.binding.gpio);
var util = require('util');


var defaultConfiguration = {
  direction: gpio.DIRECTION.OUT,
  mode: gpio.MODE.NONE,
  edge: gpio.EDGE.NONE
};


function Gpio() {
  if (!(this instanceof Gpio)) {
    return new Gpio();
  }
}

Gpio.prototype.open = function(configuration, callback) {
  return gpioPinOpen(configuration, callback);
};

Gpio.prototype.DIRECTION = gpio.DIRECTION;

Gpio.prototype.MODE = gpio.MODE;

Gpio.prototype.EDGE = gpio.EDGE;

function gpioPinOpen(configuration, callback) {
  var _binding = null;

  function GpioPin(configuration, callback) {
    var self = this;

    // validate pin
    if (util.isObject(configuration)) {
      if (!util.isNumber(configuration.pin)) {
        throw new TypeError('Bad configuration - pin is mandatory and number');
      }
    } else {
      throw new TypeError('Bad arguments - configuration should be Object')
    }

    // validate direction
    if (configuration.direction !== undefined) {
      if (configuration.direction !== gpio.DIRECTION.IN &&
          configuration.direction !== gpio.DIRECTION.OUT) {
        throw new TypeError(
          'Bad configuration - direction should be DIRECTION.IN or OUT');
      }
    } else {
      configuration.direction = defaultConfiguration.direction;
    }

    // validate mode
    var mode = configuration.mode;
    if (process.platform === 'nuttx' && mode !== undefined) {
      if (configuration.direction === gpio.DIRECTION.IN) {
        if (mode !== gpio.MODE.NONE && mode !== gpio.MODE.PULLUP &&
            mode !== gpio.MODE.PULLDOWN) {
          throw new TypeError(
            'Bad configuration - mode should be MODE.NONE, PULLUP or PULLDOWN');
        }
      } else if (configuration.direction === gpio.DIRECTION.OUT) {
        if (mode !== gpio.MODE.NONE && mode !== gpio.MODE.FLOAT &&
            mode !== gpio.MODE.PUSHPULL && mode !== gpio.MODE.OPENDRAIN) {
          throw new TypeError(
            'Bad configuration - ' +
            'mode should be MODE.NONE, FLOAT, PUSHPULL or OPENDRAIN');
        }
      }
    } else {
      configuration.mode = defaultConfiguration.mode;
    }

    // validate edge
    var edge = configuration.edge;
    if (configuration.edge !== undefined) {
      if (edge !== gpio.EDGE.NONE && edge !== gpio.EDGE.RISING &&
          edge !== gpio.EDGE.FALLING && edge !== gpio.EDGE.BOTH) {
        throw new TypeError(
          'Bad configuration - ' +
          'edge should be EDGE.NONE, RISING, FALLING or BOTH');
      }
    } else {
      configuration.edge = defaultConfiguration.edge;
    }

    EventEmitter.call(this);

    _binding = new gpio.Gpio(configuration, function(err) {
      util.isFunction(callback) && callback.call(self, err);
    });

    _binding.onChange = function() {
      self.emit('change');
    };

    process.on('exit', (function(self) {
      return function() {
        if (_binding !== null) {
          self.closeSync();
        }
      };
    })(this));
  }

  util.inherits(GpioPin, EventEmitter);

  GpioPin.prototype.write = function(value, callback) {
    var self = this;

    if (_binding === null) {
      throw new Error('GPIO pin is not opened');
    }

    if (!util.isNumber(value) && !util.isBoolean(value)) {
      throw new TypeError('Bad arguments - value should be Boolean');
    }

    _binding.write(!!value, function(err) {
      util.isFunction(callback) && callback.call(self, err);
    });
  };

  GpioPin.prototype.writeSync = function(value) {
    if (_binding === null) {
      throw new Error('GPIO pin is not opened');
    }

    if (!util.isNumber(value) && !util.isBoolean(value)) {
      throw new TypeError('Bad arguments - value should be Boolean');
    }

    _binding.write(!!value);
  };

  GpioPin.prototype.read = function(callback) {
    var self = this;

    if (_binding === null) {
      throw new Error('GPIO pin is not opened');
    }

    _binding.read(function(err, value) {
      util.isFunction(callback) && callback.call(self, err, value);
    });
  };

  GpioPin.prototype.readSync = function() {
    if (_binding === null) {
      throw new Error('GPIO pin is not opened');
    }

    return _binding.read();
  };

  GpioPin.prototype.close = function(callback) {
    var self = this;

    if (_binding === null) {
      throw new Error('GPIO pin is not opened');
    }

    _binding.close(function(err) {
      util.isFunction(callback) && callback.call(self, err);
      _binding = null;
    });
  };

  GpioPin.prototype.closeSync = function() {
    if (_binding === null) {
      throw new Error('GPIO pin is not opened');
    }

    _binding.close();
    _binding = null;
  };

  return new GpioPin(configuration, callback);
}


module.exports = Gpio;
