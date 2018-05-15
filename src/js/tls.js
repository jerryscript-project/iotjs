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
var Duplex = require('stream').Duplex;

function TLSSocket(socket, options) {
  if (!(this instanceof TLSSocket)) {
    return new TLSSocket(socket, options);
  }

  if ('_tlsSocket' in socket) {
    throw Error('Socket already bound');
  }

  this._socket = socket;
  socket._tlsSocket = this;

  Duplex.call(this);

  this.authorized = false;

  this._socket.on('connect', this.onconnect);
  this._socket.on('data', this.ondata);
  this._socket.on('error', this.onerror);
  this._socket.on('close', this.onclose);
  if (this._socket instanceof net.Socket) {
    this._socket.on('finish', this.onfinish);
  } else {
    this._socket.on('finish', this.onend);
  }
  this._socket.on('end', this.onend);

  // Native handle
  var secureContext = options.secureContext;
  if (!secureContext) {
    secureContext = createSecureContext(options);
  }

  native.TlsInit(this, options, secureContext);
  this._socketState = socket._socketState;

  var self = this;
  if (socket._writableState.ready && !options.isServer) {
    process.nextTick(function() {
      self._native_connect(options.servername || options.host || 'localhost');
      self._native_read(null);
    });
  }
}
util.inherits(TLSSocket, Duplex);

TLSSocket.prototype._native_read = native.read;
TLSSocket.prototype._native_write = native.write;
TLSSocket.prototype._native_connect = native.connect;

TLSSocket.prototype.connect = function(options, callback) {
  this._native_connect(options.servername || options.host || 'localhost');

  if (util.isFunction(callback)) {
    this.on('secureConnect', callback);
  }

  this._socket.connect(options);
};

TLSSocket.prototype._write = function(chunk, callback, onwrite) {
  chunk = this._native_write(chunk);
  this._socket.write(chunk, callback);
  onwrite();
};

TLSSocket.prototype.end = function(data, callback) {
  Duplex.prototype.end.call(this, data, callback);
  this._socket.end();
};

TLSSocket.prototype.destroy = function() {
  this._socket.destroy();
};

TLSSocket.prototype.destroySoon = function() {
  this._socket.destroySoon();
};

TLSSocket.prototype.onconnect = function() {
  this._tlsSocket._native_read(null);
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
  self._native_read(data);
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

  this._readyToWrite();

  if (server) {
    server.emit('secureConnection', this);
  } else {
    this.emit('secureConnect');
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
  var options = {};
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

  var tlsSocket = new TLSSocket(options.socket || new net.Socket(), options);
  if (tlsSocket._socket instanceof net.Socket) {
    tlsSocket.connect(options, callback);
  } else if (util.isFunction(callback)) {
    tlsSocket.on('secureConnect', callback);
  }

  return tlsSocket;
}

exports.TLSSocket = TLSSocket;
exports.Server = Server;
exports.createSecureContext = createSecureContext;
exports.createServer = createServer;
exports.connect = connect;
