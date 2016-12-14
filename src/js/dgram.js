/* Copyright 2016-present Samsung Electronics Co., Ltd. and other contributors
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
var util = require('util');

var UDP = process.binding(process.binding.udp);

var BIND_STATE_UNBOUND = 0;
var BIND_STATE_BINDING = 1;
var BIND_STATE_BOUND = 2;

// lazily loaded
var dns = null;

function lookup(address, family, callback) {
  if (!dns)
    dns = require('dns');

  return dns.lookup(address, family, callback);
}


function lookup4(address, callback) {
  return lookup(address || '0.0.0.0', 4, callback);
}


function newHandle(type) {
  if (type == 'udp4') {
    var handle = new UDP();
    handle.lookup = lookup4;
    return handle;
  }

  throw new Error('Bad socket type specified. Valid types are: udp4');
}


function Socket(type, listener) {
  EventEmitter.call(this);
  var options = undefined;

  if (util.isObject(type)) {
    options = type;
    type = options.type;
  }

  var handle = newHandle(type);
  handle.owner = this;

  this._handle = handle;
  this._receiving = false;
  this._bindState = BIND_STATE_UNBOUND;
  this.type = type;
  this.fd = null; // compatibility hack

  // If true - UV_UDP_REUSEADDR flag will be set
  this._reuseAddr = options && options.reuseAddr;

  if (util.isFunction(listener))
    this.on('message', listener);
}
util.inherits(Socket, EventEmitter);
exports.Socket = Socket;

exports.createSocket = function(type, listener) {
  return new Socket(type, listener);
};


function startListening(socket) {
  socket._handle.onmessage = onMessage;
  // Todo: handle errors
  socket._handle.recvStart();
  socket._receiving = true;
  socket._bindState = BIND_STATE_BOUND;
  socket.fd = -42; // compatibility hack

  socket.emit('listening');
}


Socket.prototype.bind = function(port /*, address, callback*/) {
  var self = this;

  self._healthCheck();

  if (this._bindState != BIND_STATE_UNBOUND)
    throw new Error('Socket is already bound');

  this._bindState = BIND_STATE_BINDING;

  if (util.isFunction(arguments[arguments.length - 1]))
    self.once('listening', arguments[arguments.length - 1]);

  var address;
  if (util.isObject(port)) {
    address = port.address || '';
    port = port.port;
  } else {
    address = util.isFunction(arguments[1]) ? '' : arguments[1];
  }

  // defaulting address for bind to all interfaces
  if (!address && self._handle.lookup === lookup4) {
    address = '0.0.0.0';
  }

  // resolve address first
  self._handle.lookup(address, function(err, ip) {
    if (err) {
      self._bindState = BIND_STATE_UNBOUND;
      self.emit('error', err);
      return;
    }

    if (!self._handle)
      return; // handle has been closed in the mean time

    self._handle._reuseAddr = self._reuseAddr;

    var err = self._handle.bind(ip, port || 0);
    if (err) {
      var ex = util.exceptionWithHostPort(err, 'bind', ip, port);
      self.emit('error', ex);
      self._bindState = BIND_STATE_UNBOUND;
      // Todo: close?
      return;
    }

    startListening(self);
  });

  return self;
}


// thin wrapper around `send`, here for compatibility with dgram_legacy.js
Socket.prototype.sendto = function(buffer, offset, length, port, address,
                                   callback) {
  if (!(util.isNumber(offset)) || !(util.isNumber(length)))
    throw new Error('send takes offset and length as args 2 and 3');

  if (!(util.isString(address)))
    throw new Error(this.type + ' sockets must send to port, address');

  this.send(buffer, offset, length, port, address, callback);
};


function sliceBuffer(buffer, offset, length) {
  if (util.isString(buffer))
    buffer = new Buffer(buffer);
  else if (!(util.isBuffer(buffer)))
    throw new TypeError('First argument must be a buffer or string');

  offset = offset >>> 0;
  length = length >>> 0;

  return buffer.slice(offset, offset + length);
}


function fixBufferList(list) {
  var newlist = new Array(list.length);

  for (var i = 0, l = list.length; i < l; i++) {
    var buf = list[i];
    if (util.isString(buf))
      newlist[i] = new Buffer(buf);
    else if (!(util.isBuffer(buf)))
      return null;
    else
      newlist[i] = buf;
  }

  return newlist;
}


function enqueue(self, toEnqueue) {
  // If the send queue hasn't been initialized yet, do it, and install an
  // event handler that flushes the send queue after binding is done.
  if (!self._queue) {
    self._queue = [];
    self.once('listening', clearQueue);
  }
  self._queue.push(toEnqueue);
  return;
}


function clearQueue() {
  var queue = this._queue;
  this._queue = undefined;

  // Flush the send queue.
  for (var i = 0; i < queue.length; i++)
    queue[i]();
}


// valid combinations
// send(buffer, offset, length, port, address, callback)
// send(buffer, offset, length, port, address)
// send(buffer, offset, length, port)
// send(bufferOrList, port, address, callback)
// send(bufferOrList, port, address)
// send(bufferOrList, port)
Socket.prototype.send = function(buffer, offset, length, port, address,
                                 callback) {
  var self = this;
  var list;

  if (address || (port && !(util.isFunction(port)))) {
    buffer = sliceBuffer(buffer, offset, length);
  } else {
    callback = port;
    port = offset;
    address = length;
  }

  if (!util.isArray(buffer)) {
    if (util.isString(buffer)) {
      list = [ new Buffer(buffer) ];
    } else if (!(buffer instanceof Buffer)) {
      throw new TypeError('First argument must be a buffer or a string');
    } else {
      list = [ buffer ];
    }
  } else if (!(list = fixBufferList(buffer))) {
    throw new TypeError('Buffer list arguments must be buffers or strings');
  }

  port = port >>> 0;
  if (port === 0 || port > 65535)
    throw new RangeError('Port should be > 0 and < 65536');

  // Normalize callback so it's either a function or undefined but not anything
  // else.
  if (!(util.isFunction(callback)))
    callback = undefined;

  self._healthCheck();

  if (self._bindState === BIND_STATE_UNBOUND)
    self.bind(0, null);

  if (list.length === 0)
    list.push(new Buffer(0));

  // If the socket hasn't been bound yet, push the outbound packet onto the
  // send queue and send after binding is complete.
  if (self._bindState !== BIND_STATE_BOUND) {
    enqueue(self, self.send.bind(self, list, port, address, callback));
    return;
  }

  self._handle.lookup(address, function afterDns(ex, ip) {
    doSend(ex, self, ip, list, address, port, callback);
  });
};


function doSend(ex, self, ip, list, address, port, callback) {
  if (ex) {
    if (util.isFunction(callback)) {
      callback(ex);
      return;
    }

    self.emit('error', ex);
    return;
  } else if (!self._handle) {
    return;
  }

  var buf = Buffer.concat(list);

  var err = self._handle.send(buf, port, ip, function (err, length) {
    if (err) {
      err = util.exceptionWithHostPort(err, 'send', address, port);
    } else {
      err = null;
    }

    if (util.isFunction(callback)) {
      callback(err, length);
    }
  });

  if (err && callback) {
    // don't emit as error, dgram_legacy.js compatibility
    var ex = exceptionWithHostPort(err, 'send', address, port);
    process.nextTick(callback, ex);
  }
}


Socket.prototype.close = function(callback) {
  if (util.isFunction(callback))
    this.on('close', callback);

  if (this._queue) {
    this._queue.push(this.close.bind(this));
    return this;
  }

  this._healthCheck();
  this._stopReceiving();
  this._handle.close();
  this._handle = null;
  var self = this;
  process.nextTick(function() {
    self.emit('close');
  });

  return this;
};


Socket.prototype.address = function() {
  this._healthCheck();

  var out = {};
  var err = this._handle.getsockname(out);
  if (err) {
    throw util.errnoException(err, 'getsockname');
  }

  return out;
};


Socket.prototype.setBroadcast = function(arg) {
  var err = this._handle.setBroadcast(arg ? 1 : 0);
  if (err) {
    throw util.errnoException(err, 'setBroadcast');
  }
};


Socket.prototype.setTTL = function(arg) {
  if (!(util.isNumber(arg))) {
    throw new TypeError('Argument must be a number');
  }

  var err = this._handle.setTTL(arg);
  if (err) {
    throw util.errnoException(err, 'setTTL');
  }

  return arg;
};


Socket.prototype.setMulticastTTL = function(arg) {
  if (!(util.isNumber(arg))) {
    throw new TypeError('Argument must be a number');
  }

  var err = this._handle.setMulticastTTL(arg);
  if (err) {
    throw util.errnoException(err, 'setMulticastTTL');
  }

  return arg;
};


Socket.prototype.setMulticastLoopback = function(arg) {
  var err = this._handle.setMulticastLoopback(arg ? 1 : 0);
  if (err) {
    throw util.errnoException(err, 'setMulticastLoopback');
  }

  return arg; // 0.4 compatibility
};


Socket.prototype.addMembership = function(multicastAddress,
                                          interfaceAddress) {
  this._healthCheck();

  if (!multicastAddress) {
    throw new Error('multicast address must be specified');
  }

  var err = this._handle.addMembership(multicastAddress, interfaceAddress);
  if (err) {
    throw util.errnoException(err, 'addMembership');
  }
};


Socket.prototype.dropMembership = function(multicastAddress,
                                           interfaceAddress) {
  this._healthCheck();

  if (!multicastAddress) {
    throw new Error('multicast address must be specified');
  }

  var err = this._handle.dropMembership(multicastAddress, interfaceAddress);
  if (err) {
    throw util.errnoException(err, 'dropMembership');
  }
};


Socket.prototype._healthCheck = function() {
  if (!this._handle)
    throw new Error('Not running'); // error message from dgram_legacy.js
};


Socket.prototype._stopReceiving = function() {
  if (!this._receiving)
    return;

  this._handle.recvStop();
  this._receiving = false;
  this.fd = null; // compatibility hack
};


function onMessage(nread, handle, buf, rinfo) {
  var self = handle.owner;
  if (nread < 0) {
    return self.emit('error', errnoException(nread, 'recvmsg'));
  }

  rinfo.size = buf.length; // compatibility
  self.emit('message', buf, rinfo);
}


Socket.prototype.ref = function() {
  if (this._handle)
    this._handle.ref();

  return this;
};


Socket.prototype.unref = function() {
  if (this._handle)
    this._handle.unref();

  return this;
};
