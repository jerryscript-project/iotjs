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

var Timer = process.binding(process.binding.timer);

var util = require('util');

var TIMEOUT_MAX = 2147483647; // 2^31-1


function Timeout(after) {
  this.after = after;
  this.isrepeat = false;
  this.callback = null;
  this.handler = null;
}


Timer.prototype.handleTimeout = function() {
  var timeout = this.timeoutObj; // 'this' is Timer object
  if (timeout && timeout.callback) {
    timeout.callback();
    if (!timeout.isrepeat) {
      timeout.close();
    }
  }
};


Timeout.prototype.activate = function() {
  var repeat = 0;
  var handler = new Timer();

  if (this.isrepeat) {
    repeat = this.after;

  }

  handler.timeoutObj = this;
  this.handler = handler;

  handler.start(this.after, repeat);
};


Timeout.prototype.close = function() {
  this.callback = undefined;
  if (this.handler) {
    this.handler.timeoutObj = undefined;
    this.handler.stop();
    this.handler = undefined;
  }
};


exports.setTimeout = function(callback, delay) {
  if (!util.isFunction(callback)) {
    throw new TypeError('Bad arguments: callback must be a Function');
  }

  delay *= 1;
  if (delay < 1 || delay > TIMEOUT_MAX) {
    delay = 1;
  }

  var timeout = new Timeout(delay);

  // set timeout handler.
  if (arguments.length <= 2) {
    timeout.callback = callback;
  } else {
    var args = Array.prototype.slice.call(arguments, 2);
    timeout.callback = function() {
      callback.apply(timeout, args);
    };
  }

  timeout.activate();

  return timeout;
};


exports.clearTimeout = function(timeout) {
  if (timeout && timeout.callback && (timeout instanceof Timeout))
    timeout.close();
  else
    throw new Error('clearTimeout() - invalid timeout');
};


exports.setInterval = function(callback, repeat) {
  if (!util.isFunction(callback)) {
    throw new TypeError('Bad arguments: callback must be a Function');
  }

  repeat *= 1;
  if (repeat < 1 || repeat > TIMEOUT_MAX) {
    repeat = 1;
  }
  var timeout = new Timeout(repeat);

  // set interval timeout handler.
  if (arguments.length <= 2) {
    timeout.callback = callback;
  } else {
    var args = Array.prototype.slice.call(arguments, 2);
    timeout.callback = function() {
      callback.apply(timeout, args);
    };
  }
  timeout.isrepeat = true;
  timeout.activate();

  return timeout;
};


exports.clearInterval = function(timeout) {
  if (timeout && timeout.isrepeat)
    timeout.close();
  else
    throw new Error('clearInterval() - invalid interval');
};
