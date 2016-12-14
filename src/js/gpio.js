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

var gpio = process.binding(process.binding.gpio);
var eventEmiter = require('events').EventEmitter;
var util = require('util');
var assert  = require('assert');


var dev_open = false;


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
      return new GpioError(errno, operation, 'Failed to initialize GPIO');
    case gpio.kGpioErrNotInitialized:
      return new GpioError(errno, operation, 'GPIO not initialized');
    case gpio.kGpioErrWrongUse:
      return new GpioError(errno, operation, 'Wrong use of GPIO API');
    case gpio.kGpioErrSys:
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

    // Emit event according to result.
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

    // Emit event according to result.
    if (err) {
      self.emit('error', err);
    } else {
      self.emit('release');
    }
  };

  // Release GPIO device.
  gpio.release(afterRelease);
};


// gpio.open(pinNumber, direction[, mode][, callback])
// event: 'open'
GPIO.prototype.open = function(pinNumber, direction, mode, callback) {
  var self = this;

  // Check initialized.
  if (!self._initialized) {
    throw CreateGpioError('open', gpio.kGpioErrNotInitialized);
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

  if (pinNumber < 0) {
    throw new Error('Invalid pinNumber');
  }

  var dirCode = convertDirection(direction);
  var modeCode = convertMode(mode);

  // Open result handler.
  var afterOpen = function(errno) {
    var err = CreateGpioError('open', errno);

    // If callback was given, calls back.
    if (util.isFunction(callback)) {
      callback(err);
    }

    // Emit event according to result.
    if (err) {
      self.emit('error', err);
    } else {
      self.emit('open', pinNumber, direction, mode);
    }
  };

  // Open pin configuration.
  gpio.open(pinNumber, dirCode, modeCode, afterOpen);
};


// gpio.write(pinNumber, value[, callback])
GPIO.prototype.write = function(pinNumber, value, callback) {
  var self = this;

  // Check initialized.
  if (!self._initialized) {
    throw CreateGpioError('write', gpio.kGpioErrNotInitialized);
  }

  // Check arguments.
  if (!util.isNumber(pinNumber)) {
    throw new TypeError('Bad arguments - pinNumber');
  }

  if (pinNumber < 0) {
    throw new Error('Invalid pinNumber');
  }

  // Make value boolean.
  value = !!value;

  // After write handler.
  var afterWrite = function(errno) {
    var err = CreateGpioError('write', errno);

    // If callback was given, calls back.
    if (util.isFunction(callback)) {
      callback(err);
    }

    // Emit error.
    if (err) {
      self.emit('error', err);
    } else {
      self.emit('write', pinNumber, value);
    }
  };

  // Set pin value.
  gpio.write(pinNumber, value, afterWrite);
};


// gpio.read(pinNumber[, callback])
GPIO.prototype.read = function(pinNumber, callback) {
  var self = this;

  // Check initialized.
  if (!self._initialized) {
    throw CreateGpioError('read', gpio.kGpioErrNotInitialized);
  }

  // Check arguments.
  if (!util.isNumber(pinNumber)) {
    throw new TypeError('Bad arguments - pinNumber');
  }

  if (pinNumber < 0) {
    throw new Error('Invalid pinNumber');
  }

  // After read handler.
  var afterRead = function(errno, value) {
    var err = CreateGpioError('read', errno);

    // Calls back.
    if (util.isFunction(callback)) {
      callback(err, value);
    }

    // Emit error.
    if (err) {
      self.emit('error', err);
    } else {
      self.emit('read', pinNumber, value);
    }
  };

  // Read value from a GPIO pin.
  gpio.read(pinNumber, afterRead);
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
