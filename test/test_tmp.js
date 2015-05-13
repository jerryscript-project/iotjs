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

// Module Kind
var MODULE_BUFFER = 0;
var MODULE_CONSOLE = 1;
var MODULE_FS = 2;
var MODULE_PROCESS= 3;
var MODULE_TIMER = 4;

// global object
this.global = this;
var global = this.global;


// start entry
this.startIoTjs = function(process) {

global.process = process;


function init_console() {
  global.console = process.binding(MODULE_CONSOLE);
};
init_console();


function init_util() {
  var exports = {};

  function isNull(arg) {
    return arg === null;
  };
  exports.isNull = isNull;

  function isUndefined(arg) {
    return arg === undefined;
  };
  exports.isUndefined = isUndefined;

  function isNullOrUndefined(arg) {
    return isNull(arg) || isUndefined(arg);
  };
  exports.isNullOrUndefined = isNullOrUndefined;

  function isNumber(arg) {
    return typeof arg === 'number';
  };
  exports.isNumber = isNumber;

  function isString(arg) {
    return typeof arg === 'string';
  };
  exports.isString = isString;

  function isObject(arg) {
    return typeof arg === 'object' && arg != null;
  };
  exports.isObject = isObject;

  function isFunction(arg) {
    return typeof arg === 'function';
  };
  exports.isFunction = isFunction;

  function isBuffer(arg) {
    return arg instanceof Buffer;
  };
  exports.isBuffer = isBuffer;

  exports.isArray = Array.isArray;

  function inherits(ctor, superCtor) {
    ctor.prototype = new superCtor();
  };
  exports.inherits = inherits;

  global.util = exports;
};
init_util();


function init_process(process) {

  initNextTick();

  function initNextTick() {
    var nextTickQueue = [];

    process.nextTick = nextTick;
    process._onNextTick = _onNextTick;

    function _onNextTick() {
      // clone nextTickQueue to new array object, and calles function
      // iterating the cloned array. This is becuase, during processing nextTick
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
      if (util.isFunction(callback)) {
        nextTickQueue.push(callback);
      }
    }
  }
};
init_process(process);


function init_buffer() {
  var buffer = global.process.binding(MODULE_BUFFER);
  var alloc = buffer.alloc;
  var kMaxLength = buffer.kMaxLength;

  function Buffer(subject, encoding) {
    if (!(this instanceof Buffer)) {
      return new Buffer(subject, encoding);
    }

    if (util.isNumber(subject)) {
      this.length = subject > 0 ? subject >>> 0 : 0;
    } else if (util.isString(subject)) {
      this.length = Buffer.byteLength(subject);
    }

    alloc(this, this.length);

    if (util.isString(subject)) {
      this.write(subject, encoding);
    } else if (util.isBuffer(subject)) {
      subject.copy(this, 0, 0, this.length);
    }
  };

  Buffer.byteLength = function(str, enc) {
    return str.length;
  }

  Buffer.concat = function(list) {
    if (!util.isArray(list)) {
      throw new TypeError(
          '1st parameter for Buffer.concat() should be array of Buffer');
    }

    length = 0;
    for (var i = 0; i < list.length; ++i) {
      if (!util.isBuffer(list[i])) {
        throw new TypeError(
            '1st parameter for Buffer.concat() should be array of Buffer');
      }
      length += list[i].length;
    }

    var buffer = new Buffer(length);
    var pos = 0;
    for (var i = 0; i < list.length; ++i) {
      list[i].copy(buffer, pos);
      pos += list[i].length;
    }

    return buffer;
  }

  Buffer.prototype.write = function(string, offset, length, encoding) {
    // buffer.write(string)
    if (util.isUndefined(offset)) {
      encoding = 'utf8';
      length = this.length;
      offset = 0;
    }
    // buffer.write(string, encoding)
    if (util.isUndefined(length) && util.isString(offset)) {
      encoding = offset;
      length = this.length;
      offset = 0;
    }
    // buffer.write(string, offset, length, encoding)
    offset = offset >>> 0;
    if (util.isNumber(length)) {
      length = length >>> 0;
    } else {
      encoding = length;
      length = undefined;
    }

    var remaining = this.length - offset;
    if (util.isUndefined(length) || length > remaining) {
      length = remaining;
    }
    //encoding = !!encoding ? (encoding + '').toLowerCase() : 'utf8';

    if (length < 0 || offset < 0) {
      throw new Error('attempt to write outside buffer bounds');
    }

    return this._write(string, offset, length);
  };

  Buffer.prototype.toString = function(encoding, start, end) {
    return this._toString();
  };

  buffer.setupBufferJs(Buffer);

  global.Buffer = Buffer;
};
init_buffer();


function init_events() {
  function EventEmitter() {
    this._events = {};
  };

  // TODO: using arguments instead of arg1, arg2.
  EventEmitter.prototype.emit = function(type, arg1, arg2) {
    if (!this._events) {
      this._events = {};
    }

    var handler = this._events[type];
    if (util.isUndefined(handler)) {
      return false;
    } else if (util.isFunction(handler) || util.isObject(handler)) {
      if (util.isFunction(handler)) {
        handler.call(this, arg1, arg2);
      } else {
        listeners = handler;
        for (i = 0; i < listeners.length; ++i) {
          listeners[i].call(this, arg1, arg2);
        }
      }
    }

    return true;
  };

  EventEmitter.prototype.addListener = function(type, listener) {
    if (!util.isFunction(listener)) {
      throw new TypeError('linster must be a function');
    }

    if (!this._events) {
      this._events = {};
    }

    if (!this._events[type]) {
      this._events[type] = listener;
    } else if (util.isObject(this._events[type])) {
      this._events[type].push(listener);
    } else {
      this._events[type] = [this._events[type], listener];
    }

    return this;
  };

  EventEmitter.prototype.on = EventEmitter.prototype.addListener;

  global.EventEmitter = EventEmitter;
};
init_events();


function init_stream() {
  var EE = global.EventEmitter;

  function Stream() {
    EE.call(this);
  };

  util.inherits(Stream, EE);


  function ReadableState(options) {
    options = options || {};

    // the internal array of buffers.
    this.buffer = [];

    // the sum of length of buffers.
    this.length = 0;

    this.defaultEncoding = options.defaultEncoding || 'utf8';

    // true if in flowing mode.
    this.flowing = false;

    // become true when the stream meet EOF.
    this.ended = false;

    // become true just before emit 'end' event.
    this.endEmitted = false;
  };

  function Readable(options) {
    if (!(this instanceof Readable)) {
      return new Readable();
    }

    this.state = new ReadableState(options);

    Stream.call(this);
  };

  util.inherits(Readable, Stream);


  Readable.prototype.read = function(n) {
    var state = this.state;
    var res;

    if (!util.isNumber(n) || n > state.length) {
      n = state.length;
    } else if (n < 0) {
      n = 0;
    }

    if (n > 0) {
      res = readBuffer(this, n);
    } else {
      res = null;
    }

    if (state.ended && state.length == 0) {
      emitEnd(this);
    }

    return res;
  };

  Readable.prototype.on = function(ev, cb) {
    var res = Stream.prototype.on.call(this, ev, cb);
    if (ev === 'data') {
      this.resume();
    }
    return res;
  };

  Readable.prototype.resume = function() {
    var state = this._state;
    if (!state.flowing) {
      state.flowing = true;
      var self = this;
      process.nextTick(function() {
        self.read(0);
      });
    }
  };

  Readable.prototype.push = function(chunk, encoding) {
    var state = this.state;

    if (!util.isString(chunk) &&
        !util.isBuffer(chunk) &&
        !util.isNull(chunk)) {
      emitError(this, TypeError('Invalid chunk'));
    } else if (util.isNull(chunk)) {
      onEof(this);
    } else if (state.ended) {
      emitError(this, Error('stream.push() after EOF'));
    } else {
      if (util.isString(chunk)) {
        encoding = encoding || state.defaultEncoding;
        chunk = new Buffer(chunk, encoding);
      }
      if (state.flowing) {
        emitData(stream, chunk);
      } else {
        state.length += chunk.length;
        state.buffer.push(chunk);
        emitReadable(this);
      }
    }
  };

  function readBuffer(stream, n) {
    var state = stream.state;
    var res;

    if (state.buffer.length === 0 || state.length === 0) {
      res = null;
    } else if (n >= state.length) {
      res = Buffer.concat(state.buffer);
      state.buffer = [];
      state.length = 0;
    } else {
      throw new Error('not implemented');
    }

    return res;
  }

  function emitEnd(stream) {
    var state = stream.state;

    if (stream.length > 0 || !state.ended) {
      throw new Error('stream ended on non-EOF stream');
    }
    if (!state.endEmitted) {
      state.endEmitted = true;
      process.nextTick(function(){
        stream.emit('end');
      })
    }
  }

  function emitReadable(stream) {
    stream.emit('readable');
  };

  function emitData(stream, data) {
    stream.emit('data', data);
  };

  function emitError(stream, er) {
    stream.emit('error', er);
  };

  function onEof(stream) {
    if (stream.state.ended) {
      return;
    }
    stream.state.ended = true;
  };

  global.Stream = Stream;
  global.ReadableStream = Readable;
};
init_stream();


process.nextTick(function() {
  var buffer = new Buffer("buffer test");
  console.log(buffer.toString());

  var foo = function(x) {
    console.log("foo emitted: " + x);
  };

  var emitter = new EventEmitter();
  emitter.on('test', foo);
  emitter.emit('test', 1);
  emitter.addListener('test', foo);
  emitter.emit('test', 2);


  var readable = new ReadableStream();
  readable.on('readable', function() {
    var data = readable.read();
    console.log('read: ' + data.toString());
  })

  readable.push('abcde');
  readable.push('12345');


  var Timer = process.binding(MODULE_TIMER);
  var timerobj = new Timer();
  timerobj.start(500, 0, function() {
    console.log("JS Timer fired");
  });
  console.log("JS Timer timeout in 500 msec...");

  // FIXME: this nextTick callback should be triggred before above timer event.
  process.nextTick(function() {
    console.log("next tick");
  });
});


}; // end of start iot.js
