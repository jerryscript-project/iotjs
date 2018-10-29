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


var Stream = require('stream_internal');
var Writable = require('stream_writable');
var util = require('util');


function ReadableState(options) {
  options = options || {};

  // the internal array of buffers.
  this.buffer = [];

  // the sum of length of buffers.
  this.length = 0;

  this.defaultEncoding = options.defaultEncoding || 'utf8';

  // true if in flowing mode.
  this.flowing = false;

  // become `true` when the stream meet EOF.
  this.ended = false;

  // become `true` just before emit 'end' event.
  this.endEmitted = false;
}


function Readable(options) {
  if (!(this instanceof Readable)) {
    return new Readable(options);
  }

  this._readableState = new ReadableState(options);

  Stream.call(this);
}

util.inherits(Readable, Stream);


Readable.prototype.read = function(n) {
  var state = this._readableState;
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

  return res;
};


Readable.prototype.on = function(ev, cb) {
  var res = Stream.prototype.on.call(this, ev, cb);
  if (ev === 'data') {
    this.resume();
  }
  return res;
};


Readable.prototype.isPaused = function() {
  return !this._readableState.flowing;
};


Readable.prototype.pause = function() {
  var state = this._readableState;
  if (state.flowing) {
    state.flowing = false;
    this.emit('pause');
  }
  return this;
};


Readable.prototype.resume = function() {
  var state = this._readableState;
  if (!state.flowing) {
    state.flowing = true;
    this.read();
  }
  return this;
};


Readable.prototype.error = function(error) {
  this.emit('error', error);
};


Readable.prototype.push = function(chunk, encoding) {
  var state = this._readableState;

  if (chunk === null) {
    onEof(this);
  } else if (!util.isString(chunk) &&
             !util.isBuffer(chunk)) {
    this.error(TypeError('Invalid chunk'));
  } else if (state.ended) {
    this.error(Error('stream.push() after EOF'));
  } else {
    if (util.isString(chunk)) {
      encoding = encoding || state.defaultEncoding;
      chunk = new Buffer(chunk, encoding);
    }
    if (state.flowing) {
      emitData(this, chunk);
    } else {
      state.length += chunk.length;
      state.buffer.push(chunk);
      this.emit('readable');
    }
  }
};


Readable.prototype.pipe = function(destination, options) {
  if (!(destination instanceof Writable || isDuplex(destination))) {
    throw new TypeError('pipe excepts stream.Writable or' +
                        ' stream.Duplex as argument');
  }

  options = options || {'end': true};

  var listeners = {
    readableListener: readableListener.bind(this),
    dataListener: dataListener.bind(destination),
    endListener: endListener.bind(destination),
  };

  this.on('readable', listeners.readableListener);
  this.on('data', listeners.dataListener);

  if (options.end) {
    this.on('end', listeners.endListener);
  }

  this._piped = this._piped || [];
  this._piped.push(destination);

  this._piped_listeners = this._piped_listeners || [];
  this._piped_listeners.push(listeners);

  return destination;
};


Readable.prototype.unpipe = function(destination) {
  if (destination === undefined) {
    this.removeAllListeners();
    this._piped = undefined;
    this._piped_listeners = undefined;
    return;
  }

  var idx = this._piped.indexOf(destination);
  if (idx === -1) {
    return;
  }

  this._piped.splice(idx, 1);
  var listeners = this._piped_listeners.splice(idx, 1)[0];

  this.removeListener('readable', listeners.readableListener);
  this.removeListener('data', listeners.dataListener);
  this.removeListener('end', listeners.endListener);

  return destination;
};


function readableListener() {
  this.resume();
}


function dataListener(data) {
  this.write(data);
}


function endListener() {
  this.end();
}


function readBuffer(stream, n) {
  var state = stream._readableState;
  var res;

  if (n == 0 || util.isNullOrUndefined(n)) {
    n = state.length;
  }

  if (state.buffer.length === 0 || state.length === 0) {
    res = null;
  } else if (n >= state.length) {
    res = Buffer.concat(state.buffer);
    state.buffer = [];
    state.length = 0;
    emitData(stream, res);
  } else {
    throw new Error('not implemented');
  }

  return res;
}


function emitEnd(stream) {
  var state = stream._readableState;

  if (stream.length > 0 || !state.ended) {
    throw new Error('stream ended on non-EOF stream');
  }
  if (!state.endEmitted) {
    state.endEmitted = true;
    stream.emit('end');
  }
}


function emitData(stream, data) {
  var state = stream._readableState;

  if (state.buffer.length === 0 || state.length === 0) {
    stream.emit('data', data);
  }

  if (state.ended && state.length == 0) {
    emitEnd(stream);
  }
}


function onEof(stream) {
  var state = stream._readableState;

  state.ended = true;

  if (state.length == 0) {
    emitEnd(stream);
  }
}


function isDuplex(stream) {
  if (!(stream instanceof Readable)) {
    return false;
  }

  var wr_keys = Object.keys(Writable.prototype);
  for (var i = 0; i < wr_keys.length; i++) {
      var wr_key = wr_keys[i];
      if (!stream[wr_key]) {
          return false;
      }
  }

  return true;
}


module.exports = Readable;
