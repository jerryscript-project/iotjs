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


(function(process) {

  this.global = this;
  global.process = process;


  function startIoTjs() {
    initGlobal();
    initTimers();

    initProcess();

    var module = Native.require('module');

    module.runMain();
  };


  function initGlobal() {
    global.process = process;
    global.global = global;
    global.GLOBAL = global;
    global.root = global;
    global.console = Native.require('console');
    global.Buffer = Native.require('buffer');
  };


  function initTimers() {
    global.setTimeout = function() {
      var t = Native.require('timers');
      return t.setTimeout.apply(this, arguments);
    };

    global.setInterval = function() {
      var t = Native.require('timers');
      return t.setInterval.apply(this, arguments);
    };

    global.clearTimeout = function() {
      var t = Native.require('timers');
      return t.clearTimeout.apply(this, arguments);
    };

    global.clearInterval = function() {
      var t = Native.require('timers');
      return t.clearInterval.apply(this, arguments);
    };
  }


  function initProcess() {
    initProcessArgv();
    initProcessEvents();
    initProcessNextTick();
    initProcessUncaughtException();
    initProcessExit();
  }


  // Initialize `process.argv`
  function initProcessArgv() {
    process.argv = [];
    process._initArgv();
  }


  function initProcessEvents() {
    var EventEmitter = Native.require('events').EventEmitter;

    EventEmitter.call(process);

    var keys = Object.keys(EventEmitter.prototype);
    for (var i = 0; i < keys.length; ++i) {
      var key = keys[i];
      if (!process[key]) {
        process[key] = EventEmitter.prototype[key];
      }
    }
  }


  function initProcessNextTick() {
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

      for (var i = 0; i < callbacks.length; ++i) {
        try {
          callbacks[i]();
        } catch (e) {
          process._onUncaughtExcecption(e);
        }
      }

      return nextTickQueue.length > 0;
    }

    function nextTick(callback) {
      nextTickQueue.push(callback);
    }
  }


  function initProcessUncaughtException() {
    process._onUncaughtExcecption = _onUncaughtExcecption;
    function _onUncaughtExcecption(error) {
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
  }


  function initProcessExit() {
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
        process._onUncaughtExcecption(e);
      } finally {
        process.doExit(process.exitCode || 0);
      }
    };
  }


  function Native(id) {
    this.id = id;
    this.filename = id + '.js';
    this.exports = {};
  };


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
  };


  Native.prototype.compile = function() {
    // process.native_sources has a list of pointers to
    // the source strings defined in 'iotjs_js.h', not
    // source strings.

    var fn = process.compileNativePtr(this.id);
    fn(this.exports, Native.require, this);
  };


  // temp impl. before JSON.parse is done
  process.JSONParse = function(text) {
    return JSON.parse(text);
  };

  startIoTjs();

});
