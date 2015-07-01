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
  , Module   = Native.require('module')
  , util     = require('util')
  , dev_file = process.iotjs.gpiodevice
  , dev_open = false
  , gpiomap;


function GPIO() {

}


GPIO.readconfig = function(path) {
  var res = Module.load(path, this);

  if (!res) {
    var err = new Error('GPIO error, not a valid config file');
    throw err;
  }
  if (!res.CTRL || res.CTRL.IN === undefined || res.CTRL.OUT === undefined
                || res.CTRL.FLOAT === undefined ) {
    var err = new Error('GPIO error, invalid CTRL');
    throw err;
  }
  if (!res.PINS) {
    var err = new Error('GPIO error, invalid PINS');
    throw err;
  }
  return res;
};


GPIO.initialize = function(mapper, callback) {
  process.on('exit', function() {
    if (dev_open) {
      gpioctl.release();
      dev_open = false;
    }
  });

  if (gpioctl.initialize(dev_file) < 0) {
    var err = new Error('GPIO error, failed to initialize');
    throw err;
  }

  dev_open = true;
  gpiomap = mapper;
  if (util.isFunction(callback)) {
    process.nextTick(function() {
      callback();
    });
  }
};


GPIO.release = function() {
  if (dev_open) {
    gpioctl.release();
    dev_open = false;
  }

  gpiomap = undefined;
};


function checkValidity(portname) {
  if (!gpiomap) {
    var err = new Error('GPIO error, mapper not defined');
    throw err;
  }
  if (!gpiomap.PINS[portname]) {
    var err = new Error('GPIO error, portname not defined: ' + portname);
    throw err;
  }
}


GPIO.setpin = function(portname, direction, callback) {
  checkValidity(portname);

  var port = gpiomap.PINS[portname].GPIO;
  if (direction == "out" || direction == "output" ) {
    port |= gpiomap.CTRL.OUT;
  }
  else {
    port |= gpiomap.CTRL.IN;
  }
  var err = gpioctl.pinmode(port);

  if (util.isFunction(callback)) {
    process.nextTick(function() {
      callback(err);
    });
  }
};


GPIO.resetpin = function(portname, callback) {
  checkValidity(portname);

  var port = gpiomap.PINS[portname].GPIO;
  port |= gpiomap.CTRL.FLOAT;
  var err = gpioctl.pinmode(port);

  if (util.isFunction(callback)) {
    process.nextTick(function() {
      callback(err);
    });
  }
};


GPIO.write = function(portname, val, callback) {
  checkValidity(portname);

  var port = gpiomap.PINS[portname].GPIO;
  var err = gpioctl.writepin(port, val);

  if (util.isFunction(callback)) {
    process.nextTick(function() {
      callback(err);
    });
  }
};


GPIO.read = function(portname, callback) {
  checkValidity(portname);

  var port = gpiomap.PINS[portname].GPIO;
  var value = gpioctl.readpin(port);

  if (util.isFunction(callback)) {
    process.nextTick(function() {
      callback(0, value);
    });
  }
};


module.exports = GPIO;
