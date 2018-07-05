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

(function() {
  this.global = this;

  function Module(id) {
    this.id = id;
    this.exports = {};
  }

  Module.cache = {};
  Module.builtin_modules = {};

  mixin(Module.builtin_modules, process.builtin_modules);
  mixin(Module, process._private);
  process._private = undefined;

  Module.require = function(id) {
    if (id === 'builtin') {
      return Module;
    }

    if (Module.cache[id]) {
      return Module.cache[id].exports;
    }

    var module = new Module(id);

    Module.cache[id] = module;
    module.compile();

    return module.exports;
  };


  Module.prototype.compile = function() {
    Module.compileModule(this, Module.require);
  };


  global.console = Module.require('console');
  global.Buffer = Module.require('buffer');

  var timers = undefined;

  var _timeoutHandler = function(mode) {
    if (timers == undefined) {
      timers = Module.require('timers');
    }
    return timers[mode].apply(this, Array.prototype.slice.call(arguments, 1));
  };

  global.setTimeout = _timeoutHandler.bind(this, 'setTimeout');
  global.setInterval = _timeoutHandler.bind(this, 'setInterval');
  global.clearTimeout = _timeoutHandler.bind(this, 'clearTimeout');
  global.clearInterval = _timeoutHandler.bind(this, 'clearInterval');

  var EventEmitter = Module.require('events').EventEmitter;

  EventEmitter.call(process);

  mixin(process, EventEmitter.prototype);

  function mixin(target, source) {
    for (var prop in source) {
      if (source.hasOwnProperty(prop) && !target[prop]) {
        target[prop] = source[prop];
      }
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
      console.error(error);
      if (Array.isArray(error.stack)) {
        error.stack.forEach(function(line) {
          console.log('    at ' + line);
        });
      }

      process.exit(1);
    }
  }


  process.exitCode = 0;
  process._exiting = false;
  process.emitExit = function(code) {
    code = code || process.exitCode;
    if (typeof code !== 'number') {
      code = 0;
    }
    if (!process._exiting) {
      process._exiting = true;
      if (code || code == 0) {
        process.exitCode = code;
      }
      process.emit('exit', process.exitCode);
    }
  };


  process.exit = function(code) {
    if (!process._exiting) {
      try {
        process.emitExit(code);
      } catch (e) {
        process.exitCode = 1;
        process._onUncaughtException(e);
      } finally {
        process.doExit(process.exitCode);
      }
    }
  };

  var module = Module.require('module');
  module.runMain();
})();
