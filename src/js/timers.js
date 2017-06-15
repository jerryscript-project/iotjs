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
      timeout.unref();
    }
  }
};


Timeout.prototype.ref = function() {
  var repeat = 0;
  var handler = new Timer();

  if (this.isrepeat) {
    repeat = this.after;

  }

  handler.timeoutObj = this;
  this.handler = handler;

  handler.start(this.after, repeat);
};


Timeout.prototype.unref = function() {
  this.callback = undefined;
  if (this.handler) {
    this.handler.timeoutObj = undefined;
    this.handler.stop();
    this.handler = undefined;
  }
};

function timeoutConfigurator(isrepeat, callback, delay) {
  if (!util.isFunction(callback)) {
    throw new TypeError('Bad arguments: callback must be a Function');
  }

  delay *= 1;
  if (delay < 1 || delay > TIMEOUT_MAX) {
    delay = 1;
  }

  var timeout = new Timeout(delay);

  // set timeout handler.
  if (arguments.length <= 3) {
    timeout.callback = callback;
  } else {
    var args = Array.prototype.slice.call(arguments, 3);
    args.splice(0, 0, timeout);
    timeout.callback = callback.bind.apply(callback, args);
  }
  timeout.isrepeat = isrepeat;
  timeout.ref();

  return timeout;
}

exports.setTimeout = timeoutConfigurator.bind(undefined, false);
exports.setInterval = timeoutConfigurator.bind(undefined, true);

function clearTimeoutBase(timeoutType, timeout) {
  if (timeout) {
    if (timeout instanceof Timeout) {
      timeout.unref();
    } else {
       throw new Error(timeoutType + '() - invalid timeout');
    }
  }
}

exports.clearTimeout = clearTimeoutBase.bind(undefined, 'clearTimeout');
exports.clearInterval = clearTimeoutBase.bind(undefined, 'clearInterval');
