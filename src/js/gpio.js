/* Copyright 2015 Samsung Electronics Co., Ltd.
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

var gpio = process.binding(process.binding.gpio);
var eventEmiter = require('events').EventEmitter;
var util = require('util');
var assert  = require('assert');


var dev_open = false;

var GpioSettingType = {
  kGpioPin: 'Pin',
  kGpioPort: 'Port',
  getLowLetter: function(type) {
    return type === this.kGpioPin ? 'pin' : 'port';
  }
};

function GpioError(code, operation, message) {
  this.name = 'GpioError';
  this.code = code;
  this.operation = operation;
  this.message = operation + ': ' + message;
}

util.inherits(GpioError, Error);


function CreateGpioError(operation, errno) {
  if (errno == 0) {
    return null;
  }

  switch (errno) {
    case gpio.kGpioErrInitialize:
      return new GpioError(errno, operation, 'Failed to initilize GPIO');
    case gpio.kGpioErrNotInitialized:
      return new GpioError(errno, operation, 'GPIO not initialized');
    case gpio.kGpioErrWrongUse:
      return new GpioError(errno, operation, 'Wrong use of GPIO API');
    case gpio.kGpioErrSys:
      return new GpioError(errno, operation, 'System error');
  }
  return new GpioError(errno, operation, 'Unknown error');
}

function GpioControlWorker(_gpio) {
  this._gpioJObj = _gpio;
}

GpioControlWorker.prototype.setGpio = function(type, gpioImp, args) {
  var gpioJObj = this._gpioJObj,
      number = args[0],
      direction = args[1],
      mode = args[2],
      callback = args[3];

  // Check initialized.
  if (!gpioJObj._initialized) {
    throw CreateGpioError('set' + type, gpio.kGpioErrNotInitialized);
  }

  // Check arguments.
  if (!util.isNumber(number)) {
    throw new TypeError('Bad arguments - ' +
                        GpioSettingType.getLowLetter(type) + 'Number');
  }
  if (!util.isString(direction)) {
    throw new TypeError('Bad arguments - direction');
  }
  if (util.isFunction(mode)) {
    callback = mode;
    mode = 'none';
  }
  if (util.isNullOrUndefined(mode) || mode == '') {
    mode = 'none';
  }

  direction = convertDirection(direction);
  mode = convertMode(mode);

  // Set pin/port result handler.
  var afterSet = function(errno) {
    var err = CreateGpioError('set' + type, errno);

    // If callback was given, calls back.
    if (util.isFunction(callback)) {
      callback(err);
    }

    // Emits event according to result.
    if (err) {
      gpioJObj.emit('error', err);
    } else {
      if (type === GpioSettingType.kGpioPin) {
        gpioJObj.emit('setPin', number, direction, mode);
      } else if (type === GpioSettingType.kGpioPort) {
        // Emits setPort event
        gpioJObj.emit('setPort', number, direction, mode);

        // Emits setPin event for pin that are bound to the port.
        var pin = number * 8,
            offset;
        for (offset = 0; offset < 8; offset++) {
          gpioJObj.emit('setPin', pin + offset, direction, mode);
        }
      }
    }
  };

  // Set pin/port configuration.
  gpioImp(number, direction, mode, afterSet);
};


GpioControlWorker.prototype.writeGpio = function(type, gpioImp, args) {
  var gpioJObj = this._gpioJObj,
      number = args[0],
      value = args[1],
      callback = args[2];

  // Check initialized.
  if (!gpioJObj._initialized) {
    throw CreateGpioError('write' + type, gpio.kGpioErrNotInitialized);
  }

  // Check arguments.
  if (!util.isNumber(number)) {
    throw new TypeError('Bad arguments - ' +
                        GpioSettingType.getLowLetter(type) + 'Number');
  }

  if (type === GpioSettingType.kGpioPin) {
    if (!(util.isBoolean(value) || util.isNumber(value))) {
      throw new TypeError('Bad arguments - value');
    }
    // Make value boolean.
    value = !!value;
  } else if (type === GpioSettingType.kGpioPort) {
    if (!util.isNumber(value)) {
      throw new TypeError('Bad arguments - value');
    }
  }

  // After write pin/port handler.
  var afterWrite = function(errno) {
    var err = CreateGpioError('write' + type, errno);

    // If callback was given, calls back.
    if (util.isFunction(callback)) {
      callback(err);
    }

    // Emits error.
    if (err) {
      gpioJObj.emit('error', err);
    } else {
      gpioJObj.emit('write' + type, number, value);
    }
  };

  // write pin/port a value.
  gpioImp(number, value, afterWrite);
};


GpioControlWorker.prototype.readGpio = function(type, gpioImp, args) {
  var gpioJObj = this._gpioJObj,
      number = args[0],
      callback = args[1];

  // Check initialized.
  if (!gpioJObj._initialized) {
    throw CreateGpioError('read' + type, gpio.kGpioErrNotInitialized);
  }

  // Check arguments.
  if (!util.isNumber(number)) {
    throw new TypeError('Bad arguments - ' +
                        GpioSettingType.getLowLetter(type) + 'Number');
  }

  // After read pin/port handler.
  var afterRead = function(errno, value) {
    var err = CreateGpioError('read' + type, errno);

    // calls back.
    if (util.isFunction(callback)) {
      callback(err, value);
    }

    // Emits error.
    if (err) {
      gpioJObj.emit('error', err);
    } else {
      gpioJObj.emit('read' + type, number, value);
    }
  };

  // Read value from a GPIO pin/port.
  gpioImp(number, afterRead);
};


function GPIO() {
  eventEmiter.call(this);

  this._initializing = false;
  this._initialized = false;

  this._gpioControlWorker = new GpioControlWorker(this);
}

util.inherits(GPIO, eventEmiter);

// gpio.initialize(callback)
//  callback: Function(err: GpioError | null)
// event : 'initialize'
GPIO.prototype.initialize = function(callback) {
  var self = this;

  // Initialize result handler.
  var afterInitialize = function(errno) {
    var err = CreateGpioError('initialize', errno);

    if (errno == 0) {
      self._initialized = true;
    } else {
      self._initialized = false;
    }

    // If callback was given, calls back.
    if (util.isFunction(callback)) {
      callback(err);
    }

    // Emits event according to result.
    if (err) {
      self.emit('error', err);
    } else {
      self.emit('initialize');
    }
  };

  // In case of already initialized.
  if (self._initialized) {
    // callback with null (not error)
    // not emit 'initialize' event.
    if (util.isFunction(callback)) {
      process.nextTick(function() {
        callback(null);
      });
    }

    return null;
  }

  // Initialization is undergoing.
  if (self._initializing) {
    throw CreateGpioError('initialize', gpio.kGpioErrWrongUse);
  }

  // Initialize GPIO device.
  this._initializing = true;
  gpio.initialize(afterInitialize);
};


// gpio.release(callback)
//  callback: Function(err: GpioError | null)
// event: 'release'
GPIO.prototype.release = function(callback) {
  var self = this;

  // Check initialized.
  if (!self._initialized) {
    throw CreateGpioError('release', gpio.kGpioErrNotInitialized);
  }

  // Release result handler.
  var afterRelease = function(errno) {
    var err = CreateGpioError('release', errno);

    if (errno == 0) {
      self._initialized = false;
    }

    // If callback was given, calls back.
    if (util.isFunction(callback)) {
      callback(err);
    }

    // Emits event according to result.
    if (err) {
      self.emit('error', err);
    } else {
      self.emit('release');
    }
  };

  // Release GPIO device.
  gpio.release(afterRelease);
};


// gpio.setPin(pinNumber, direction[, mode][, callback])
// event: 'setPin'
GPIO.prototype.setPin = function(pinNumber, direction, mode, callback) {
  this._gpioControlWorker.setGpio(GpioSettingType.kGpioPin,
                                  gpio.setPin, arguments);
};


// gpio.writePin(pinNumber, value[, callback])
// event: 'writePin'
GPIO.prototype.writePin = function(pinNumber, value, callback) {
  this._gpioControlWorker.writeGpio(GpioSettingType.kGpioPin,
                                    gpio.writePin, arguments);
};


// gpio.readPin(pinNumber[, callback])
// event: 'readPin'
GPIO.prototype.readPin = function(pinNumber, callback) {
  this._gpioControlWorker.readGpio(GpioSettingType.kGpioPin,
                                   gpio.readPin, arguments);
};


// gpio.setPort(portNumber, direction[, mode][, callback])
// event: 'setPort'
GPIO.prototype.setPort = function(portNumber, direction, mode, callback) {
  this._gpioControlWorker.setGpio(GpioSettingType.kGpioPort,
                                  gpio.setPort, arguments);
};


// gpio.writePort(portNumber, value[, callback])
// event: 'writePort'
GPIO.prototype.writePort = function(portNumber, value, callback) {
  this._gpioControlWorker.writeGpio(GpioSettingType.kGpioPort,
                                    gpio.writePort, arguments);
};


// gpio.readPort(portNumber[, callback])
// event: 'readPort'
GPIO.prototype.readPort = function(portNumber, callback) {
  this._gpioControlWorker.readGpio(GpioSettingType.kGpioPort,
                                   gpio.readPort, arguments);
};


GPIO.prototype.query = function(queryOption, callback) {
  assert.fail('GPIO.query() - not implemented');
};


function convertDirection(direction) {
  switch (direction) {
    case 'in': return gpio.kGpioDirectionIn;
    case 'out': return gpio.kGpioDirectionOut;
    case 'none': return gpio.kGpioDirectionNone;
  }
  throw new TypeError('Bad arguments - direction');
}


function convertMode(mode) {
  switch (mode) {
    case 'pullup': return gpio.kGpioModePullup;
    case 'pulldn':
    case 'pulldown': return gpio.kGpioModePulldown;
    case 'float': return gpio.kGpioModeFloat;
    case 'pushpull': return gpio.kGpioModePushpull;
    case 'opendrain': return gpio.kGpioModeOpendrain;
    case 'none': return gpio.kGpioModeNone;
  }
  throw new TypeError('bad arguments - mode');
}


module.exports = new GPIO();
