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

this.global = this;

function Native(id) {
  this.id = id;
  this.filename = id + '.js';
  this.exports = {};
}


Native.cache = {};


Native.require = function(id) {
  if (id == 'native') {
    return Native;
  }

  if (Native.cache[id]) {
    return Native.cache[id].exports;
  }

  var nativeMod = new Native(id);

  Native.cache[id] = nativeMod;
  nativeMod.compile();

  return nativeMod.exports;
}


Native.prototype.compile = function() {
  // process.native_sources has a list of pointers to
  // the source strings defined in 'iotjs_js.h', not
  // source strings.

  var fn = process.compileNativePtr(this.id);
  fn(this.exports, Native.require, this);
}

global.console = Native.require('console');
global.Buffer = Native.require('buffer');

(function() {
  var timers = undefined;

  var _timeoutHandler = function(mode) {
    if (timers == undefined) {
      timers = Native.require('timers');
    }
    return timers[mode].apply(this, Array.prototype.slice.call(arguments, 1));
  }

  global.setTimeout = _timeoutHandler.bind(this, 'setTimeout');
  global.setInterval = _timeoutHandler.bind(this, 'setInterval');
  global.clearTimeout = _timeoutHandler.bind(this, 'clearTimeout');
  global.clearInterval = _timeoutHandler.bind(this, 'clearInterval');
})();

var EventEmitter = Native.require('events').EventEmitter;

EventEmitter.call(process);

var keys = Object.keys(EventEmitter.prototype);
var keysLength = keys.length;
for (var i = 0; i < keysLength; ++i) {
  var key = keys[i];
  if (!process[key]) {
    process[key] = EventEmitter.prototype[key];
  }
}

var nextTickQueue = [];

process.nextTick = nextTick;
process._onNextTick = _onNextTick;


function _onNextTick() {
  // clone nextTickQueue to new array object, and calls function
  // iterating the cloned array. This is because,
  // during processing nextTick
  // a callback could add another next tick callback using
  // `process.nextTick()`, if we calls back iterating original
  // `nextTickQueue` that could turn into infinite loop.

  var callbacks = nextTickQueue.slice(0);
  nextTickQueue = [];

  var len = callbacks.length;
  for (var i = 0; i < len; ++i) {
    try {
      callbacks[i]();
    } catch (e) {
      process._onUncaughtException(e);
    }
  }

  return nextTickQueue.length > 0;
}


function nextTick(callback) {
  var args = Array.prototype.slice.call(arguments);
  args[0] = null;
  nextTickQueue.push(Function.prototype.bind.apply(callback, args));
}


process._onUncaughtException = _onUncaughtException;
function _onUncaughtException(error) {
  var event = 'uncaughtException';
  if (process._events[event] && process._events[event].length > 0) {
    try {
      // Emit uncaughtException event.
      process.emit('uncaughtException', error);
    } catch (e) {
      // Even uncaughtException handler thrown, that could not be handled.
      console.error('uncaughtException handler throws: ' + e);
      process.exit(1);
    }
  } else {
    // Exit if there are no handler for uncaught exception.
    console.error('uncaughtException: ' + error);
    process.exit(1);
  }
}


process.exitCode = 0;
process._exiting = false;
process.emitExit = function(code) {
  if (!process._exiting) {
    process._exiting = true;
    if (code || code == 0) {
      process.exitCode = code;
    }
    process.emit('exit', process.exitCode || 0);
  }
}


process.exit = function(code) {
  try {
    process.emitExit(code);
  } catch (e) {
    process.exitCode = 1;
    process._onUncaughtException(e);
  } finally {
    process.doExit(process.exitCode || 0);
  }
}


var module = Native.require('module');
module.runMain();
