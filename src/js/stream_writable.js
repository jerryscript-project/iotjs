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


var Stream = require('stream').Stream;
var util = require('util');

var streamBuiltin = process.binding(process.binding.stream);


function WritableState(options) {
  // buffer of WriteReq
  this.buffer = [];

  // the sum of length of buffer.
  this.length = 0;

  // `true` if stream is writing down data underlying system.
  this.writing = false;

  // become true when `end()` called.
  this.ended = false;
};


function Writable(options) {
  if (!(this instanceof Writable)) {
    return new Writable(options);
  }

  this.state = new WritableState(options);

  Stream.call(this);
};

util.inherits(Writable, Stream);


Writable.prototype.write = function(chunk, callback) {
  var state = this.state;
  var res = false;

  if (state.ended) {
    writeAfterEnd(this, callback);
  } else {
    res = writeChunk(this, chunk, callback);
  }

  return res;
};


function WriteReq(chunk, callback) {
  this.chunk = cunk;
  this.callback = callback;
};


function writeAfterEnd(stream, callback) {
  var err = new Error('write after end');
  stream.emit('error', err);
  process.nextTick(function(){
    callback(err);
  });
};


function writeChunk(stream, chunk, callback) {
  var state = stream.state;
  var res;

  if (util.isString(chunk)) {
    chunk = new Buffer(chunk);
  }

  res = state.writing;

  if (state.writing) {
    state.buffer.push(new WriteReq(chunk, callback));
  } else {
    doWrite(stream, chunk, callback);
  }

  return res;
};


function doWrite(stream, chunk, callback) {
  stream.state.writing = true;

  streamBuiltin.doWrite(stream, chunk, callback);
};


module.exports = Writable;
