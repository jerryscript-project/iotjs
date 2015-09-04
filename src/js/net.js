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


var EventEmitter = require('events').EventEmitter;
var stream = require('stream');
var util = require('util');
var timers = require('timers');

var TCP = process.binding(process.binding.tcp);


function createTCP(socket) {
  var tcp = new TCP(socket);
  return tcp;
}


function SocketState(options) {
  // 'true' during connection handshaking.
  this.connecting = false;

  // become 'true' when connection established.
  this.connected = false;

  this.writable = true;
  this.readable = true;

  this.destroyed = false;

  this.allowHalfOpen = options && options.allowHalfOpen || false;
}


function Socket(options) {
  if (!(this instanceof Socket)) {
    return new Socket(options);
  }

  if (util.isUndefined(options)) {
    options = {};
  }

  stream.Duplex.call(this, options);

  this._timer = null;
  this._timeout = 0;

  this._socketState = new SocketState(options);

  if (options.handle) {
    this._handle = options.handle;
    this._handle._setHolder(this);
  }

  this.on('finish', onSocketFinish);
  this.on('end', onSocketEnd);
}


// Socket inherits Duplex.
util.inherits(Socket, stream.Duplex);


Socket.prototype.connect = function() {
  var self = this;
  var state = self._socketState;

  var args = normalizeConnectArgs(arguments);
  var options = args[0];
  var callback = args[1];

  if (state.connecting || state.connected) {
    return self;
  }

  if (!self._handle) {
    self._handle = createTCP(this);
  }

  if (util.isFunction(callback)) {
    self.once('connect', callback);
  }

  self._resetTimeout();

  state.connecting = true;

  var dns = require('dns');
  var host = options.host ? options.host : 'localhost';
  var port = options.port;
  var dnsopts = {
    family: options.family >>> 0,
    hints: 0
  };

  if (!util.isNumber(port) || port < 0 || port > 65535)
    throw new RangeError('port should be >= 0 and < 65536: ' + options.port);

  if (dnsopts.family !== 0 && dnsopts.family !== 4 && dnsopts.family !== 6)
    throw new RangeError('port should be 4 or 6: ' + dnsopts.family);

  self._host = host;
  dns.lookup(host, dnsopts, function(err, ip, family) {
    self.emit('lookup', err, ip, family);

    if (err) {
      process.nextTick(function() {
        self.emit('error', err);
        self._destroy();
      });
    } else {
      self._resetTimeout();
      self._handle.connect(ip, port, afterConnect);
    }

  });

  return self;
};


Socket.prototype.write = function(data, callback) {
  if (!util.isString(data) && !util.isBuffer(data)) {
    throw new TypeError('invalid argument');
  }

  return stream.Duplex.prototype.write.call(this, data, callback);
};


Socket.prototype._write = function(chunk, callback) {
  var self = this;

  self._resetTimeout();

  var cb = function(status) {
    self._onwrite(status);

    if (util.isFunction(callback)) {
      callback(status);
    }
  };

  this._handle.write(chunk, cb);
};


Socket.prototype.end = function(data, callback) {
  var self = this;
  var state = self._socketState;

  // end of writable stream.
  stream.Writable.prototype.end.call(self, data, callback);

  // this socket is no longer writable.
  state.writable = false;
};


// Destroy this socket as fast as possible.
Socket.prototype.destroy = function() {
  var self = this;
  var state = self._socketState;

  if (state.destroyed) {
    return;
  }

  if (state.writable) {
    self.end();
  }

  // unset timeout
  self._clearTimeout();

  if (self._writableState.ended) {
    self._handle.close();
    state.destroyed = true;
  } else {
    self.once('finish', function() {
      self.destroy();
    });
  }
};


// Destroy this socket as fast as possible if this socket is no longer readable.
Socket.prototype.destroySoon = function() {
  var self = this;
  var state = self._socketState;

  if (state.writable) {
    self.end();
  }

  if (self._writableState.finished) {
    self.destroy();
  } else {
    self.once('finish', self.destroy);
  }
}


Socket.prototype.setKeepAlive = function(enable, delay) {
  var self = this;
  enable = +Boolean(enable);
  if (self._handle && self._handle.setKeepAlive) {
    self._handle.setKeepAlive(enable, ~~(delay / 1000));
  }
};


Socket.prototype._onread = function(nread, isEOF, buffer) {
  var self = this;
  var state = self._socketState;

  self._resetTimeout();

  if (isEOF) {
    // pushing readable stream null means EOF.
    stream.Readable.prototype.push.call(this, null);

    if (self._readableState.length == 0) {
      // this socket is no longer readable.
      state.readable = false;
      // destory if this socket is not writable.
      maybeDestroy(self);
    }
  } else if (nread < 0) {
    var err = new Error('read error: ' + nread);
    stream.Readable.prototype.error.call(this, err);
  } else if (nread > 0) {
    stream.Readable.prototype.push.call(this, buffer);
  }
};


Socket.prototype._onclose = function() {
  this.emit('close');

  if (this.server) {
    var sockets = this.server._sockets;
    var idx = sockets.indexOf(this);
    delete sockets[idx];
  }
};


Socket.prototype._clearTimeout = function() {
  var self = this;
  if (self._timer) {
    timers.clearTimeout(self._timer);
    self._timer = null;
  }
};


Socket.prototype._resetTimeout = function() {
  var self = this;

  if (!self.destroyed) {
    // start timeout over again
    self._clearTimeout();
    self._timer = timers.setTimeout(function() {
      self.emit('timeout');
      self._clearTimeout();
    }, self._timeout);
  }
};


Socket.prototype.setTimeout = function(msecs, callback) {
  var self = this;
  self._timeout = msecs;
  self._clearTimeout();
  if (msecs === 0) {
    if (callback) {
      self.removeListener('timeout', callback);
    }
  } else {
    self._timer = timers.setTimeout(function() {
      self.emit('timeout');
      self._clearTimeout();
    }, msecs);
    if (callback) {
      self.once('timeout', callback);
    }
  }
};


function emitError(socket, err) {
  socket.emit('error', err);
}


function maybeDestroy(socket) {
  var state = socket._socketState;

  if (!state.connecting &&
      !state.writable &&
      !state.readable) {
    socket.destroy();
  }
}


function onSocketConnect(socket) {
  var state = socket._socketState;

  state.connecting = false;
  state.connected = true;

  socket._readyToWrite();

  // `readStart` on next tick, after connection event handled.
  process.nextTick(function() {
    socket._handle.readStart();
  });
}


// After socket connection.
function afterConnect(status) {
  var self = this;
  var state = self._socketState;

  state.connecting = false;

  if (status == 0) {
    self._resetTimeout();
    onSocketConnect(self);

    // emit 'connect' event
    self.emit('connect');
  } else {
    emitError(self, new Error('connect failed - status: ' + status));
  }
}


// Writable stream finished.
function onSocketFinish() {
  var self = this;
  var state = self._socketState;
  if (!state.readable || self._readableState.ended) {
    // no readable steram or ended, destory(close) socket.
    return self.destroy();
  } else {
    // Readable stream alive, shutdown only outgoing stream.
    var err = self._handle.shutdown(onShutdown);
  }
}


// Readable stream ended.
function onSocketEnd() {
  var state = this._socketState;

  maybeDestroy(this);

  if (!state.allowHalfOpen) {
    this.destroySoon();
  }
}


function onShutdown(status) {
 var self = this;

 if (self._readableState.ended) {
  self.destroy();
 }
}



function Server(options, connectionListener) {
  if (!(this instanceof Server)) {
    return new Server(options, connectionListener);
  }

  EventEmitter.call(this);

  if (util.isFunction(options)) {
    connectionListener = options;
    options = {};
  } else {
    options = options || {};
  }

  if (util.isFunction(connectionListener)) {
    this.on('connection', connectionListener);
  }

  this._handle = null;
  this._sockets = [];

  this.allowHalfOpen = options.allowHalfOpen || false;
}

// Server inherits EventEmitter.
util.inherits(Server, EventEmitter);


// This is needed for native handler to create TCP instance.
// Jerry API does not provide functionality for creating object via specific
// constructor hence native handler could not create such instance by itself.
Server.prototype._createTCP = createTCP;


Server.prototype.listen = function() {
  var self = this;

  var args = normalizeListenArgs(arguments);

  var options = args[0];
  var callback = args[1];

  var port = options.port;
  var host = util.isString(options.host) ? options.host : '0.0.0.0';
  var backlog = util.isNumber(options.backlog) ? options.backlog : 511;

  if (!util.isNumber(port)) {
    throw new Error('invalid argument - need port number');
  }

  // register listening event linstener.
  if (util.isFunction(callback)) {
    self.once('listening', callback);
  }

  // Create server handle.
  if (!self._handle) {
    self._handle = createTCP(self);
  }

  // bind port
  var err = self._handle.bind(host, port);
  if (err) {
    self._handle.close();
    return err;
  }

  // listen
  err = self._handle.listen(backlog);
  if (err) {
    self._handle.close();
    return err;
  }

  process.nextTick(function() {
    if (self._handle) {
      self.emit('listening');
    }
  });
};


Server.prototype.close = function(callback) {
  if (util.isFunction(callback)) {
    this.once('close', callback);
  }
  if (this._handle) {
    this._handle.close();
  }
  return this;
};


// This function is called after server accepted connection request
// from a client.
//  Parameters
//   * status - status code
//   * clientHandle - client socket handle (tcp).
Server.prototype._onconnection = function(status, clientHandle) {
  if (status) {
    this.emit('error', new Error('accept error: ' + status));
    return;
  }

  // Create socket object for connecting client.
  var socket = new Socket({
    handle: clientHandle,
    allowHalfOpen: this.allowHalfOpen
  });

  socket.server = this;
  onSocketConnect(socket);

  this._sockets.push(socket);

  this.emit('connection', socket);
};


Server.prototype._onclose = function() {
  this.emit('close');
};


function normalizeListenArgs(args) {
  var options = {};

  if (util.isObject(args[0])) {
    options = args[0];
  } else {
    var idx = 0;
    options.port = args[idx++];
    if (util.isString(args[idx])) {
      options.host = args[idx++];
    }
    if (util.isNumber(args[idx])) {
      options.backlog = args[idx++];
    }
  }

  var cb = args[args.length - 1];

  return util.isFunction(cb) ? [options, cb] : [options];
}


function normalizeConnectArgs(args) {
  var options = {};

  if (util.isObject(args[0])) {
    options = args[0];
  } else {
    options.port = args[0];
    if (util.isString(args[1])) {
      options.host = args[1];
    }
  }

  var cb = args[args.length - 1];

  return util.isFunction(cb) ? [options, cb] : [options];
}


exports.createServer = function(options, callback) {
  return new Server(options, callback);
};


// net.connect(options[, connectListenr])
// net.connect(port[, host][, connectListener])
exports.connect = exports.createConnection = function() {
  var args = normalizeConnectArgs(arguments);
  var socket = new Socket(args[0]);
  return Socket.prototype.connect.apply(socket, args);
};


module.exports.Socket = Socket;
module.exports.Server = Server;
