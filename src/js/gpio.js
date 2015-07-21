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

var gpioctl  = process.binding(process.binding.gpioctl)
  , util     = require('util')
  , dev_open = false;


function GPIO() {

}


GPIO.initialize = function() {
  var err = gpioctl.initialize();
  if (err >= 0) {
    dev_open = true;
  }
  return err;
};


GPIO.release = function() {
  if (dev_open) {
    gpioctl.release();
    dev_open = false;
  }
};


function getDirectionCode(direction) {
  if (direction == 'out') return gpioctl.OUT;
  if (direction == 'in') return gpioctl.IN;
  if (direction == 'none') return gpioctl.NONE;
  return -1;
}

function getModeCode(mode) {
  if (mode == 'pullup') return gpioctl.PULLUP;
  if (mode == 'pulldn') return gpioctl.PULLDN;
  if (mode == 'float') return gpioctl.FLOAT;
  if (mode == 'pushpull') return gpioctl.PUSHPULL;
  if (mode == 'opendrain') return gpioctl.OPENDRAIN;
  if (mode == undefined || mode == 'none' || mode == '') return gpioctl.NONE;
  return -1;
}

GPIO.setPin = function(pinNumber, direction, mode, callback) {
  if (util.isFunction(mode)) {
    callback = mode;
    mode = '';
  }
  var dircode = getDirectionCode(direction);
  if (dircode < 0) {
    return gpioctl.ERR_INVALIDPARAM;
  }
  var modecode = getModeCode(mode);
  if (modecode < 0) {
    return gpioctl.ERR_INVALIDPARAM;
  }
  return gpioctl.setPin(pinNumber, dircode, modecode, callback);
};


GPIO.writePin = function(pinNumber, value, callback) {
  return gpioctl.writepin(pinNumber, value, callback);
};


GPIO.readPin = function(pinNumber, callback) {
  return gpioctl.readPin(pinNumber, callback);
};


GPIO.setPort = function(portNumber, direction, mode, callback) {
  if (util.isFunction(mode)) {
    callback = mode;
    mode = ' ';
  }
  var dircode = getDirectionCode(direction);
  if (dircode < 0) {
    return gpioctl.ERR_INVALIDPARAM;
  }
  return gpioctl.setPort(portNumber, dircode, mode, callback);
};


GPIO.writePort = function(portNumber, value, callback) {
  return gpioctl.writePort(portNumber, value, callback);
};


GPIO.readPort = function(portNumber, callback) {
  return gpioctl.readPort(portNumber, callback);
};


GPIO.query = function(queryOption, callback) {
  return gpioctl.query(queryOption, callback);
}

module.exports = GPIO;
