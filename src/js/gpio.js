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


function GpioError(code, operation, message) {
  this.name = 'GpioError';
  this.code = code;
  this.operation = operation
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
    case gpio.kGpioErrSysErr:
      return new GpioError(errno, operation, 'System error');
  }
  return new GpioError(errno, operation, 'Unknown error');
}


function GPIO() {
  eventEmiter.call(this);

  this._initializing = false;
  this._initialized = false;
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
    };

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
// event: 'setpin'
GPIO.prototype.setPin = function(pinNumber, direction, mode, callback) {
  var self = this;

  // Check initialized.
  if (!self._initialized) {
    throw CreateGpioError('setPin', gpio.kGpioErrNotInitialized);
  }

  // Check arguments.
  if (!util.isNumber(pinNumber)) {
    throw new TypeError('Bad arguments - pinNumber');
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

  var dirCode = convertDirection(direction);
  var modeCode = convertMode(mode);

  // setPin result handler.
  var afterSetPin = function(errno) {
    var err = CreateGpioError('setPin', errno);

    // If callback was given, calls back.
    if (util.isFunction(callback)) {
      callback(err);
    }

    // Emits event according to result.
    if (err) {
      self.emit('error', err);
    } else {
      self.emit('setPin', pinNumber, direction, mode);
    }
  };

  // Set pin configuration.
  gpio.setPin(pinNumber, dirCode, modeCode, afterSetPin);
};


// gpio.writePin(pinNumber, value[, callback])
GPIO.prototype.writePin = function(pinNumber, value, callback) {
  var self = this;

  // Check initialized.
  if (!self._initialized) {
    throw CreateGpioError('writePin', gpio.kGpioErrNotInitialized);
  }

  // Check arguments.
  if (!util.isNumber(pinNumber)) {
    throw new TypeError('Bad arguments - pinNumber');
  }

  // Make value boolean.
  value = !!value;

  // After writePin handler.
  var afterWritePin = function(errno) {
    var err = CreateGpioError('writePin', errno);

    // If callback was given, calls back.
    if (util.isFunction(callback)) {
      callback(err);
    }

    // Emits error.
    if (err) {
      self.emit('error', err);
    } else {
      self.emit('writePin', pinNumber, value);
    }
  };

  // write pin a value.
  gpio.writePin(pinNumber, value, afterWritePin);
};


// gpio.readPin(pinNumber[, callback])
GPIO.prototype.readPin = function(pinNumber, callback) {
  var self = this;

  // Check initialized.
  if (!self._initialized) {
    throw CreateGpioError('readPin', gpio.kGpioErrNotInitialized);
  }

  // Check arguments.
  if (!util.isNumber(pinNumber)) {
    throw new TypeError('Bad arguments - pinNumber');
  }

  // After readPin handler.
  var afterReadPin = function(errno, value) {
    var err = CreateGpioError('readPin', errno);

    // calls back.
    if (util.isFunction(callback)) {
      callback(err, value);
    }

    // Emits error.
    if (err) {
      self.emit('error', err);
    } else {
      self.emit('readPin', pinNumber, value);
    }
  };

  // Read value from a GPIO pin.
  gpio.readPin(pinNumber, afterReadPin);
};


GPIO.prototype.setPort = function(portNumber, direction, mode, callback) {
  assert.fail('GPIO.setPort() - not implemented');
};


GPIO.prototype.writePort = function(portNumber, value, callback) {
  assert.fail('GPIO.writePort() - not implemented');
};


GPIO.prototype.readPort = function(portNumber, callback) {
  assert.fail('GPIO.readPort() - not implemented');
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
