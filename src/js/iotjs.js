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

(function(process) {

  this.global = this;
  global.process = process;


  function start_iotjs() {
    init_global();

    init_process();

    var module = Native.require('module');
    module.runMain();
  };


  function init_global() {
    global.process = process;
    global.global = global;
    global.GLOBAL = global;
    global.root = global;
    global.console =  process.binding(process.binding.console);
    global.Buffer = Native.require('buffer');
  };


  function init_process() {

    initNextTick();

    function initNextTick() {
      var nextTickQueue = [];

      process.nextTick = nextTick;
      process._onNextTick = _onNextTick;

      function _onNextTick() {
        // clone nextTickQueue to new array object, and calles function
        // iterating the cloned array. This is becuase,
        // during processing nextTick
        // a callback could add another next tick callback using
        // `process.nextTick()`, if we calls back iterating original
        // `nextTickQueue` that could turn into infinify loop.

        // FIXME: var callbacks = nextTickQueue.slice(0);
        var callbacks = [];
        for (var i = 0; i < nextTickQueue.length; ++i) {
          callbacks.push(nextTickQueue[i]);
        }
        nextTickQueue = [];

        for (var i = 0; i < callbacks.length; ++i) {
          var callback = callbacks[i];
          callback();
        }
      }

      function nextTick(callback) {
        nextTickQueue.push(callback);

      }
    }
  };


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


  Native.wrap = function(script) {
    var temp1 = Native.wrapper[0] + script;
    temp1 = temp1 + Native.wrapper[1];
    return temp1;
  };


  Native.wrapper = [
    '(function (a, b, c) { function wwwwrap(exports, require, module) {',
    ' }; wwwwrap(a, b, c); });'
  ];


  Native.prototype.compile = function() {
    var source = process.native_sources[this.id];
    source = Native.wrap(source);
    var fn = process.compile(source);
    fn(this.exports, Native.require, this);
  };


  // temp impl. before JSON.parse is done
  process.JSONParse = function(text) {
      return process.compile("(" + text + ");");
  };


  start_iotjs();

});
