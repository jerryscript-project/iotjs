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


var util = require('util');
var stream = require('stream');


function OutgoingMessage() {
  stream.Stream.call(this);

  this.writable = true;

  this._hasBody = true;

  this.finished = false;
  this._sentHeader = false;

  this.socket = null;
  this.connection = null;
  // response header string : same 'content' as this._headers
  this._header = null;
  // response header obj : (key, value) pairs
  this._headers = {};

}

util.inherits(OutgoingMessage, stream.Stream);

exports.OutgoingMessage = OutgoingMessage;


OutgoingMessage.prototype.end = function(data, encoding, callback) {
  if (util.isFunction(data)) {
    callback = data;
    data = null;
  } else if (util.isFunction(encoding)) {
    callback = encoding;
    encoding = null;
  }


  // flush header
  if (!this._header) {
    this._implicitHeader();
  }

  if (data) {
    this.write(data, encoding);
  }

  var self = this;
  var emitFinish = function() {
    self.emit('finish');
  };

  if (this.finished) {
    return false;
  }


  if (util.isFunction(callback))
    this.once('finish', callback);


  // emit finish
  process.nextTick(emitFinish);


  this.finished = true;

  this._finish();

  return true;
};


OutgoingMessage.prototype._finish = function() {
  this.emit('prefinish');
};


// This sends chunk directly into socket
// TODO: buffering of chunk in the case of socket is not available
OutgoingMessage.prototype._send = function(chunk, encoding, callback) {
  if (util.isFunction(encoding)) {
    callback = encoding;
  }


  if (!this._sentHeader) {
    if (util.isBuffer(chunk)) {
      chunk = chunk.toString();
    }
    chunk = this._header + "\r\n" + chunk;
    this._sentHeader = true;
  }

  if (util.isBuffer(chunk)) {
    chunk = chunk.toString();
  }

  return this.connection.write(chunk, encoding, callback);
};


OutgoingMessage.prototype.write = function(chunk, encoding, callback) {
  if (!this._header) {
    this._implicitHeader();
  }

  if (!this._hasBody) {
    return true;
  }
  var ret = this._send(chunk, encoding, callback);

  return ret;

};


// Stringfy header fields of _headers into _header
OutgoingMessage.prototype._storeHeader = function(statusLine) {
  var headerStr = '';

  var keys;
  if (this._headers) {
    keys = Object.keys(this._headers);
    for (var i=0; i<keys.length; i++) {
      var key = keys[i];
      headerStr += key + ": " + this._headers[key] + '\r\n';
    }
  }

  this._header = statusLine + headerStr;

};


OutgoingMessage.prototype.setHeader = function(name, value) {

  if (this._headers === null) {
    this._headers = {};
  }

  this._headers[name] = value;

};


OutgoingMessage.prototype.removeHeader = function(name) {
  if (this._headers === null) {
    return;
  }

  delete this._headers[name];
};


OutgoingMessage.prototype.getHeader = function(name) {
  return this._headers[name];
};
