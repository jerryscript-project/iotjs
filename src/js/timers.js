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

var Timer = process.binding(process.binding.timer);

var util = require('util');

var TIMEOUT_MAX = 2147483647; // 2^31-1


// Timeout holders
// Todo, profile and optimize
var timers_list = [];

function Timeout(after) {
  this.after = after;
  this.isrepeat = false;
  this.callback = null;
  this.handler = null;
}


var handleTimeout = function() {
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
  if (this.isrepeat)
    repeat = this.after;
  handler.timeoutObj = this;
  this.handler = handler;
  handler.start(this.after, repeat, handleTimeout);

  timers_list.push(this);
};


Timeout.prototype.close = function() {
  this.callback = undefined;
  if (this.handler) {
    this.handler.timeoutObj = undefined;
    this.handler.stop();
    this.handler = undefined;
  }

  // remove 'this' from list
  var idx = timers_list.indexOf(this);
  if (idx > -1) {
    timers_list.splice(idx,1);
  }
};


exports.setTimeout = function(callback, delay) {
  if (!util.isFunction(callback))
    throw new TypeError('callback must be a function');

  delay *= 1;
  if (delay < 1 || delay > TIMEOUT_MAX) {
    delay = 1;
  }

  var timeout = new Timeout(delay);
  timeout.callback = callback;
  // Todo, use apply when arguments are ready
  timeout.activate();

  return timeout;
};


exports.clearTimeout = function(timeout) {
  if (timeout && timeout.callback && (timeout instanceof Timeout))
    timeout.close();
  else
    throw new Error('Error clearTimeout, not a valid timeout Object');
};


exports.setInterval = function(callback, repeat) {
  if (!util.isFunction(callback))
    throw new TypeError('callback must be a function');

  repeat *= 1;
  if (repeat < 1 || repeat > TIMEOUT_MAX) {
    repeat = 1;
  }
  var timeout = new Timeout(repeat);
  timeout.callback = callback;
  timeout.isrepeat = true;
  timeout.activate();

  return timeout;
};


exports.clearInterval = function(timeout) {
  if (timeout && timeout.isrepeat)
    timeout.close();
  else
    throw new Error('Error clearInterval, not a valid interval Object');
};
