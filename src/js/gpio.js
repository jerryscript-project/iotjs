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


GPIO.initialize = function(callback) {
  var err = gpioctl.initialize();
  if (err >= 0) {
    dev_open = true;
  }
  if (util.isFunction(callback)) {
    process.nextTick(function() {
      callback(err);
    });
  }
  else {
    return err;
  }
};


GPIO.release = function() {
  if (dev_open) {
    gpioctl.release();
    dev_open = false;
  }
};


GPIO.pinmode = function(portpin, callback) {
  var err = gpioctl.pinmode(portpin);

  if (util.isFunction(callback)) {
    process.nextTick(function() {
      callback(err);
    });
  }
  else {
    return err;
  }
};


GPIO.write = function(portpin, val, callback) {
  var err = gpioctl.writepin(portpin, val);

  if (util.isFunction(callback)) {
    process.nextTick(function() {
      callback(err);
    });
  }
  else {
    return err;
  }
};


GPIO.read = function(portpin, callback) {
  var err = gpioctl.readpin(portpin);
  var value = err;

  if (util.isFunction(callback)) {
    process.nextTick(function() {
      callback(err, value);
    });
  }
  else {
    return err;
  }
};


module.exports = GPIO;
