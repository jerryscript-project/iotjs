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


var EventEmitter = require('events');
var stream = require('stream');
var util = require('util');

var TCP = process.binding(process.binding.tcp);


function createTCP(socket) {
  var tcp = new TCP();
  tcp._socket = socket;
  return tcp;
}


function Socket(options) {
  if (!(this instanceof Socket)) {
    return new Socket(options);
  }

  if (util.isUndefined(options)) {
    options = {};
  }

  stream.Duplex.call(this, options);

  if (options.handle) {
    this._handle = options.handle;
    this._handle._socket = this;
  }
}

// Socket inherits Duplex.
util.inherits(Socket, stream.Duplex);

exports.Socket = Socket;


Socket.prototype.connect = function(port, host, callback) {
  if (!this._handle) {
    this._handle = createTCP(this);
  }

  this._handle.connect(host, port, callback);

  return this;
};


Socket.prototype.destroy = function() {
  var self = this;

  self._handle.close();
};


Socket.prototype._onclose = function() {
  this.emit('close');
};


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
}

// Server inherits EventEmitter.
util.inherits(Server, EventEmitter);

module.exports.Server = Server;


exports.createServer = function(options, callback) {
  return new Server(options, callback);
};


// This is needed for native handler to create TCP instance.
// Jerry API does not provide functionality for creating object via specific
// constructor hence native handler could not create such instance by itself.
Server.prototype._createTCP = createTCP;


Server.prototype.listen = function(options, callback) {
  var self = this;

  var address = "127.0.0.1";
  var port = 80;
  var backlog = 10;

  // FIXME: There is much more options for listen.
  if (util.isNumber(options.port)) {
    port = options.port;
  } else {
    throw new Error('Invalid listen argument');
  }

  // Create server handle.
  if (!self._handle) {
    self._handle = createTCP(self);
  }

  // bind port
  var err = self._handle.bind(address, port);
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

  if (util.isFunction(callback)) {
    // FIXME: This should be `once`.
    self.on('listening', callback);
  }

  process.nextTick(function() {
    if (self._handle) {
      self.emit('listening');
    }
  });
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
  });

  socket.server = this;

  this.emit('connection', socket);
};
