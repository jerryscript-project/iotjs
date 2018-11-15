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


var util = require('util');
var stream = require('stream');


function OutgoingMessage() {
  stream.Writable.call(this);

  this.writable = true;

  this._hasBody = true;

  this.finished = false;
  this._sentHeader = false;
  this._connected = false;

  // storage for chunks when there is no connection established
  this._chunks = [];

  this.socket = null;
  // response header string : same 'content' as this._headers
  this._header = null;
  // response header obj : (key, value) pairs
  this._headers = {};

}

util.inherits(OutgoingMessage, stream.Writable);

exports.OutgoingMessage = OutgoingMessage;


OutgoingMessage.prototype.end = function(data, encoding, callback) {
  var self = this;

  if (util.isFunction(data)) {
    callback = data;
    data = null;
  } else if (util.isFunction(encoding)) {
    callback = encoding;
    encoding = null;
  }

  if (this.finished) {
    return false;
  }

  // flush header
  if (!this._header) {
    this._implicitHeader();
  }

  if (data) {
    this.write(data, encoding);
  }

  // Register finish event handler.
  if (util.isFunction(callback)) {
    this.once('finish', callback);
  }

  // Force flush buffered data.
  // After all data was sent, emit 'finish' event meaning segment of header and
  // body were all sent finished. This means different from 'finish' event
  // emitted by net which indicate there will be no more data to be sent through
  // the connection. On the other hand emitting 'finish' event from http does
  // not neccessarily imply end of data transmission since there might be
  // another segment of data when connection is 'Keep-Alive'.
  this._send('', function() {
    self.emit('finish');
  });


  this.finished = true;

  this.emit('prefinish');

  return true;
};


// This sends chunk directly into socket
OutgoingMessage.prototype._send = function(chunk, encoding, callback) {
  if (util.isFunction(encoding)) {
    callback = encoding;
  }

  if (!this._sentHeader) {
    this._chunks.push(this._header + '\r\n');
    this._sentHeader = true;
  }

  if (!this._connected) {
    this._chunks.push(chunk);
    return false;
  } else {
    while (this._chunks.length) {
      this.socket.write(this._chunks.shift(), encoding, callback);
    }
  }

  if (this.socket) {
    return this.socket.write(chunk, encoding, callback);
  }

  return false;
};


OutgoingMessage.prototype.write = function(chunk, encoding, callback) {
  if (!this._header) {
    this._implicitHeader();
  }

  if (!this._hasBody) {
    return true;
  }

  return this._send(chunk, encoding, callback);
};


// Stringfy header fields of _headers into _header
OutgoingMessage.prototype._storeHeader = function(statusLine) {
  var headerStr = '';

  var keys;
  if (this._headers) {
    keys = Object.keys(this._headers);
    for (var i=0; i<keys.length; i++) {
      var key = keys[i];
      headerStr += key + ': ' + this._headers[key] + '\r\n';
    }
  }

  this._header = statusLine + headerStr;

};


OutgoingMessage.prototype.setHeader = function(name, value) {
  if ((typeof name) != 'string') {
    throw new TypeError('Name must be string.');
  }

  if (util.isNullOrUndefined(value)) {
    throw new Error('value required in setHeader(' + name + ', value)');
  }

  if (this._headers === null) {
    this._headers = {};
  }

  this._headers[name.toLowerCase()] = value;
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


OutgoingMessage.prototype.setTimeout = function(ms, cb) {
  if (cb) {
    this.once('timeout', cb);
  }

  if (!this.socket) {
    this.once('socket', function(socket) {
      socket.setTimeout(ms);
    });
  } else {
    this.socket.setTimeout(ms);
  }
};
