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

//  VALIDATION ARRAYS
var BAUDRATE = [0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400
                , 4800, 9600, 19200, 38400, 57600, 115200, 230400];
var DATABITS = [5, 6, 7, 8];

var defaultSettings = {
  autoOpen: true,
  baudRate: 9600,
  dataBits: 8,
}

function UART(path, options, callback) { //constructor
  if (!(this instanceof UART)) {
    return new UART(path, options, callback);
  }

  if (util.isFunction(options)) {
    callback = options;
    options = {};
  }

  if (util.isUndefined(options)) {
    options = {};
  }

  var settings = {};
  for (var propname in defaultSettings) {
    settings[propname] = defaultSettings[propname];
  }
  for (var propname in options) {
    settings[propname] = options[propname];
  }

  EventEmitter.call(this);

  if (!path) {
    throw new TypeError('No path specified');
  }

  if (BAUDRATE.indexOf(settings.baudRate) === -1) {
    throw new TypeError("Invalid 'baudRate': " + settings.baudRate);
  }

  if (DATABITS.indexOf(settings.dataBits) === -1) {
    throw new TypeError("Invalid 'databits': " + settings.dataBits);
  }

  this.path = path;
  this.options = settings;

  if (settings.autoOpen) {
    this.open(callback);
  }
}

util.inherits(UART, EventEmitter);

UART.prototype.open = function(callback) {
  uart.open(this, this.path, this.options, function (err) {
    return process.nextTick(function() {
      return callback(err);
    });
  }.bind(this));
}

UART.prototype.close = function() {
  return uart.close();
}

UART.prototype.write = function(buffer, callback) {
  uart.write(buffer, function(err) {
    return process.nextTick(function() {
      return callback(err);
    });
  });
};

module.exports = UART;
