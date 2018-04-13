/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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

var net = require('net');
var util = require('util');
var EventEmitter = require('events').EventEmitter;

function TLSSocket(socket, options) {
  if (!(this instanceof TLSSocket)) {
    return new TLSSocket(socket, options);
  }

  if ('_tlsSocket' in socket) {
    throw Error('Socket already bound');
  }

  this._socket = socket;
  socket._tlsSocket = this;

  EventEmitter.call(this);

  this.authorized = false;
  this.secureConnectionDone = false;
  this.buff = [];

  this._socket.on('connect', this.onconnect);
  this._socket.on('data', this.ondata);
  this._socket.on('error', this.onerror);
  this._socket.on('close', this.onclose);
  this._socket.on('finish', this.onfinish);
  this._socket.on('end', this.onend);

  // Native handle
  var secureContext = options.secureContext;
  if (!secureContext) {
    secureContext = createSecureContext(options);
  }

  native.TlsInit(this, options, secureContext);
  this._socketState = socket._socketState;
}
util.inherits(TLSSocket, EventEmitter);

TLSSocket.prototype._read = native.read;
TLSSocket.prototype._write = native.write;
TLSSocket.prototype._connect = native.connect;

TLSSocket.prototype.connect = function(options, callback) {
  this._connect(options.servername || options.host || 'localhost');

  if (util.isFunction(callback)) {
    this.on('secureConnect', callback);
  }

  this._socket.connect(options);
};

TLSSocket.prototype.write = function(data, callback) {
  if (!Buffer.isBuffer(data)) {
    data = new Buffer(data);
  }
  if (!this.secureConnectionDone) {
    if (this.buff.length > 0) {
      this.buff = Buffer.concat([this.buff, data]);
      return;
    }
    this.buff = data;
    return;
  } else {
    data = this._write(data);
    return this._socket.write(data, callback);
  }
};

TLSSocket.prototype.pause = function() {
  this._socket.pause();
};

TLSSocket.prototype.resume = function() {
  this._socket.resume();
};

TLSSocket.prototype.end = function(data, callback) {
  if (data) {
    if (!Buffer.isBuffer(data)) {
      data = new Buffer(data);
    }
    data = this._write(data, true);
  } else {
    data = this._write(null, true);
  }

  this._socket.end(data, callback);
};

TLSSocket.prototype.destroy = function() {
  this._socket.destroy();
};

TLSSocket.prototype.destroySoon = function() {
  this._socket.destroySoon();
};

TLSSocket.prototype.onconnect = function() {
  var self = this._tlsSocket;
  self._read(null);
};

TLSSocket.prototype.encrypted = function() {
  return true;
};

TLSSocket.prototype.address = function() {
  return this._socket.address();
};

TLSSocket.prototype.localAddress = function() {
  return this._socket.address().address;
};

TLSSocket.prototype.setTimeout = function(msecs, callback) {
  return this._socket.setTimeout(msecs, callback);
};

TLSSocket.prototype.ondata = function(data) {
  var self = this._tlsSocket;
  self._read(data);
};

TLSSocket.prototype.onerror = function(error) {
  this._tlsSocket.emit('error', error);
};

TLSSocket.prototype.onclose = function() {
  this._tlsSocket.emit('close');
};

TLSSocket.prototype.onfinish = function() {
  this._tlsSocket.emit('finish');
};

TLSSocket.prototype.onend = function() {
  this._tlsSocket.emit('end');
};

TLSSocket.prototype.onwrite = function(data) {
  return this._socket.write(data);
};

TLSSocket.prototype.onread = function(chunk) {
  this.emit('data', chunk);
};

TLSSocket.prototype.onhandshakedone = function(error, authorized) {
  this.authorized = authorized;

  var server = this._server;

  if (error) {
    error = Error('handshake failed');

    if (server) {
      server.emit('tlsClientError', error, this);
    } else {
      this.emit('error', error);
    }
    this.end();
    return;
  }

  this.secureConnectionDone = true;

  if (server) {
    server.emit('secureConnection', this);
  } else {
    this.emit('secureConnect');
  }

  if (this.buff.length > 0) {
    var _chunk = {
      chunk: this._write(this.buff),
    };
    this._socket._writableState.buffer.push(_chunk);
    this._socket._readyToWrite();
  }
};

function tlsConnectionListener(rawSocket) {
  var tlsSocket = new TLSSocket(rawSocket, {
    isServer: true,
    secureContext: this._secureContext,
  });

  tlsSocket._server = this;
}

function Server(options, listener) {
  if (!(this instanceof Server)) {
    return new Server(options, listener);
  }

  this._secureContext = createSecureContext(options);

  // constructor call
  net.Server.call(this, options, tlsConnectionListener);

  if (listener) {
    this.on('secureConnection', listener);
  }
}
util.inherits(Server, net.Server);

function createSecureContext(options) {
  return new native.TlsContext(options);
}

function createServer(options, secureConnectionListener) {
  return new Server(options, secureConnectionListener);
}

function connect(arg0, arg1, arg2, callback) {
  var options;
  var tlsSocket;
  if (typeof arg0 == 'object') {
    options = Object.create(arg0, {
      isServer: { value: false, enumerable: true },
    });
    options.host = options.host || 'localhost';
    options.port = options.port || 443;
    options.rejectUnauthorized = options.rejectUnauthorized || false;
    callback = arg1;
  } else if (typeof arg0 == 'number') {
    if (typeof arg1 == 'string') {
      if (typeof arg2 == 'object') {
        options = Object.create(arg2, {
          isServer: { value: false, enumerable: true },
        });
        options.port = arg0;
        options.host = arg1;
        options.rejectUnauthorized = options.rejectUnauthorized || false;
      } else {
        options = {
          isServer: false,
          rejectUnauthorized: false,
          port: arg0,
          host: arg1,
        };
        callback = arg2;
      }
    } else if (typeof arg1 == 'object') {
      options = Object.create(arg1, {
        isServer: { value: false, enumerable: true },
      });
      options.port = arg0;
      options.host = options.host || 'localhost';
      options.rejectUnauthorized = options.rejectUnauthorized || false;
      callback = arg2;
    } else {
      options = {
        isServer: false,
        rejectUnauthorized: false,
        host: 'localhost',
        port: arg0,
      };
      callback = arg1;
    }
  }
  tlsSocket = new TLSSocket(new net.Socket(), options);
  tlsSocket.connect(options, callback);

  return tlsSocket;
}

exports.TLSSocket = TLSSocket;
exports.Server = Server;
exports.createSecureContext = createSecureContext;
exports.createServer = createServer;
exports.connect = connect;
