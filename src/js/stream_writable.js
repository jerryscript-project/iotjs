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


var stream = require('stream');
var util = require('util');
var Stream = stream.Stream;
var Duplex = stream.Duplex;

var defaultHighWaterMark = 128;


function WriteReq(chunk, callback) {
  this.chunk = chunk;
  this.callback = callback;
}


function WritableState(options) {
  // buffer of WriteReq
  this.buffer = [];

  // total length of messages not flushed yet.
  this.length = 0;

  // high water mark.
  // The point where write() starts retuning false.
  var hwm = options.highWaterMark;
  this.highWaterMark = (hwm || hwm === 0) ? hwm : defaultHighWaterMark;

  // 'true' if stream is ready to write.
  this.ready = false;

  // `true` if stream is writing down data underlying system.
  this.writing = false;

  // the length of message being writing.
  this.writingLength = 0;

  // turn 'true' when some messages are buffered. After buffered messages are
  // all sent, 'drain' event will be emitted.
  this.needDrain = false;

  // become `true` when `end()` called.
  this.ending = false;

  // become `true` when there are no date to write.
  this.ended = false;
}


function Writable(options) {
  if (!(this instanceof Writable) && !(this instanceof stream.Duplex)) {
    return new Writable(options);
  }

  this._writableState = new WritableState(options);

  Stream.call(this);
}


util.inherits(Writable, Stream);


// Write chunk of data to underlying system stream.
//  Control flow in general description
//    Writable.prototype.write()
//      ||
//    writeOrBuffer()
//      ||
//    doWrite()
//      ||
//    ConcreteStream.prototype._write()
//      |
//    underlying stream
//      |
//    Writable.prototype._onwrite()
Writable.prototype.write = function(chunk, callback) {
  var state = this._writableState;
  var res = false;

  if (state.ended) {
    writeAfterEnd(this, callback);
  } else {
    res = writeOrBuffer(this, chunk, callback);
  }

  return res;
};


// This function object never to be called. concrete stream should override
// this method.
Writable.prototype._write = function(chunk, callback, onwrite) {
  throw new Error('unreachable');
}


Writable.prototype.end = function(chunk, callback) {
  var state = this._writableState;

  // Because NuttX cannot poll 'EOF',so forcely raise EOF event.
  if (process.platform == 'nuttx') {
    if (!state.ending) {
      if (util.isNullOrUndefined(chunk)) {
        chunk = '\\e\\n\\d';
      } else {
        chunk += '\\e\\n\\d';
      }
    }
  }

  if (!util.isNullOrUndefined(chunk)) {
    this.write(chunk);
  }

  if (!state.ending) {
    endWritable(this, callback);
  }
};


// When stream is ready to write, concrete stream implementation should call
// this method to inform it.
Writable.prototype._readyToWrite = function() {
  var state = this._writableState;

  state.ready = true;
  writeBuffered(this);
};


// A chunk of data has been written down to stream.
Writable.prototype._onwrite = function(status) {
  var state = this._writableState;

  state.length -= state.writingLength;

  state.writing = false;
  state.writingLength = 0;

  writeBuffered(this);
};


// A write call occured after end.
function writeAfterEnd(stream, callback) {
  var err = new Error('write after end');
  stream.emit('error', err);
  if (util.isFunction(callback)) {
    process.nextTick(function(){
      callback(err);
    });
  }
}


function writeOrBuffer(stream, chunk, callback) {
  var state = stream._writableState;

  if (util.isString(chunk)) {
    chunk = new Buffer(chunk);
  }

  state.length += chunk.length;

  if (!state.ready || state.writing || state.buffer.length > 0) {
    // stream not yet ready or there is pending request to write.
    // push this request into write queue.
    state.buffer.push(new WriteReq(chunk, callback));
  } else {
    // here means there is no pending data. write out.
    doWrite(stream, chunk, callback);
  }

  // total length of buffered message exceeded high water mark.
  if (state.length >= state.highWaterMark) {
    state.needDrain = true;
  }

  return !state.needDrain;
}


function writeBuffered(stream) {
  var state = stream._writableState;
  if (!state.writing) {
    if (state.buffer.length == 0) {
      onEmptyBuffer(stream);
    } else {
      var req = state.buffer.shift();
      doWrite(stream, req.chunk, req.callback);
    }
  }
}


function doWrite(stream, chunk, callback) {
  var state = stream._writableState;

  if (state.writing) {
    return new Error('write during writing');
  }

  // The stream is now writing.
  state.writing = true;
  state.writingLength = chunk.length;

  var afterWrite = function(status) {
    stream._onwrite(status);
  };

  // Write down the chunk data.
  stream._write(chunk, callback, afterWrite);
}


// No more data to write. if this stream is being finishing, emit 'finish'.
function onEmptyBuffer(stream) {
  var state = stream._writableState;
  if (state.ending) {
    emitFinish(stream);
  } else if (state.needDrain) {
    emitDrain(stream);
  }
}


// Writable.prototype.end() was called. register callback for 'finish' event.
// After finish writing out buffered data, 'finish' event will be fired.
function endWritable(stream, callback) {
  var state = stream._writableState;
  state.ending = true;
  if (callback) {
    stream.once('finish', callback);
  }

  // If nothing left, emit finish event at next tick.
  if (!state.writing && state.buffer.length == 0) {
    process.nextTick(function(){
      emitFinish(stream);
    });
  }
}


// Emit 'drain' event
function emitDrain(stream) {
  var state = stream._writableState;
  if (state.needDrain) {
    state.needDrain = false;
    stream.emit('drain');
  }
}


// Emit 'finish' event to notify this stream is finished.
function emitFinish(stream) {
  var state = stream._writableState;
  if (!state.ended) {
    state.ended = true;
    stream.emit('finish');
  }
}


module.exports = Writable;
