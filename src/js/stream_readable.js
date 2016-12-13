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


var Stream = require('stream').Stream;
var util = require('util');
var assert = require('assert');


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
};


function Readable(options) {
  if (!(this instanceof Readable)) {
    return new Readable(options);
  }

  this._readableState = new ReadableState(options);

  Stream.call(this);
};

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
    if (state.length > 0) {
      emitData(this, readBuffer(this));
    }
  }
  return this;
};


Readable.prototype.error = function(error) {
  emitError(this, error);
};


Readable.prototype.push = function(chunk, encoding) {
  var state = this._readableState;

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
      emitData(this, chunk);
    } else {
      state.length += chunk.length;
      state.buffer.push(chunk);
      emitReadable(this);
    }
  }
};


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
  } else {
    throw new Error('not implemented');
  }

  return res;
};


function emitEnd(stream) {
  var state = stream._readableState;

  if (stream.length > 0 || !state.ended) {
    throw new Error('stream ended on non-EOF stream');
  }
  if (!state.endEmitted) {
    state.endEmitted = true;
    stream.emit('end');
  }
};


function emitReadable(stream) {
  stream.emit('readable');
};


function emitData(stream, data) {
  var state = stream._readableState;

  assert.equal(readBuffer(stream), null);
  stream.emit('data', data);

  if (state.ended && state.length == 0) {
    emitEnd(stream);
  }
};


function emitError(stream, er) {
  stream.emit('error', er);
};


function onEof(stream) {
  var state = stream._readableState;

  state.ended = true;

  if (state.length == 0) {
    emitEnd(stream);
  }
};


module.exports = Readable;
