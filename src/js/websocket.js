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
var tls;
try {
  tls = require('tls');
} catch (e) {
  // tls remains undefined;
}

util.inherits(Websocket, EventEmitter);
util.inherits(WebsocketClient, EventEmitter);
util.inherits(Server, EventEmitter);

function WebSocketHandle(client) {
  this.client = client;
  this.pings = [];
  this.connected = false;

  native.wsInit(this);
}

function Websocket(options) {
  if (!(this instanceof Websocket)) {
    return new Websocket(options);
  }

  EventEmitter.call(this);
  this._firstMessage = true;
  this._handle = new WebSocketHandle(this);
  this._secure = false;
}

function WebsocketClient(socket, handle) {
  if (!(this instanceof WebsocketClient)) {
    return new WebsocketClient(socket, handle);
  }

  if ((tls && (socket instanceof tls.TLSSocket)) ||
      (socket instanceof net.Socket)) {
    this._socket = socket;
    this.readyState = 'CONNECTING';
  } else {
    this._firstMessage = true;
  }
  this._handle = handle;

  EventEmitter.call(this);
}

function ServerHandle() {
  this.clients = [];

  native.wsInit(this);
}

function connectionListener(socket) {
  var ws = new WebsocketClient(socket, this._serverHandle);
  this._serverHandle.clients.push(ws);
  var self = this;

  ws._socket.on('data', function(data) {
    if (ws.readyState === 'CONNECTING') {
      parseServerHandshakeData(data, ws, self);
    } else if (ws.readyState === 'OPEN') {
      self._serverHandle.ondata(data, ws);
    }
  });
}

function parseServerHandshakeData(data, client, server) {
  data = data.toString();
  var res = data.split('\r\n');
  var method = res[0].split(' ');

  // All header keys are converted to lower case
  // to ease the processing of the values.
  // Based on the HTTP/1.1 RFC (https://tools.ietf.org/html/rfc7230#section-3.2)
  // this conversion is ok as the header field names are case-insensitive.
  var headers = { 'connection': '',
                  'upgrade': '',
                  'host': '',
                  'sec-websocket-key': '',
                  'sec-websocket-version': -1,
                };

  for (var i = 1; i < res.length; i++) {
    var temp = res[i].split(': ');
    headers[temp[0].toLowerCase()] = temp[1];
  }

  var response = '';
  if (method[0] === 'GET' &&
      method[2] === 'HTTP/1.1' &&
      method[1] === server.path &&
      headers['connection'].toLowerCase().indexOf('upgrade') !== -1 &&
      headers['upgrade'].toLowerCase() === 'websocket' &&
      headers['sec-websocket-version'] === '13') {
    response = native.ReceiveHandshakeData(
      headers['sec-websocket-key']
    ).toString();
    client.readyState = 'OPEN';
    client._socket.write(response);
    server.emit('open', client);
  } else {
    response = method[2] + ' 400 Bad Request\r\nConnection: Closed\r\n\r\n';
    client._socket.write(response);
  }
}

function Server(options, listener) {
  if (!(this instanceof Server)) {
    return new Server(options);
  }

  EventEmitter.call(this);
  var emit_type = 'connection';
  this._netserver = null;

  if (options.server) {
    if (tls && (options.server instanceof tls.Server)) {
      this._netserver = options.server;
      emit_type = 'secureConnection';
    } else if (options.server instanceof net.Server) {
      this._netserver = options.server;
    }
  } else if (options.port) {
    if (options.secure == true) {
      if (!tls) {
        throw new Error('TLS module is required to create a secure server.');
      }
      this._netserver = tls.createServer(options);
      emit_type = 'secureConnection';
    } else {
      this._netserver = net.createServer(options);
    }

    this._netserver.listen(options.port);
  } else {
    throw new Error('One of port or server must be provided as option');
  }
  this._netserver.path = options.path || '/';

  this._netserver.on('error', this.onError);
  this._netserver.on(emit_type, connectionListener);
  this._netserver._serverHandle = new ServerHandle();

  if (listener) {
    this._netserver.on('open', listener);
  }

  this.options = options;
}

ServerHandle.prototype.ondata = function(data, client) {
  native.wsReceive(this, data, client);
};

ServerHandle.prototype.onmessage = function(msg, client) {
  client.emit('message', msg);
};

ServerHandle.prototype.pong = function(msg, client) {
  client.emit('ping', msg);
  this.sendFrame(native.ping(false, msg, true), client);
};

ServerHandle.prototype.onError = function(err, client) {
  client.emit('error', err);
};

ServerHandle.prototype.sendFrame = function(msg, client) {
  if (client._socket._socketState.writable) {
    client._socket.write(msg);
  } else {
    if (this.clients.indexOf(client) > -1) {
      this.onError('Underlying socket', client);
    }
  }
};

ServerHandle.prototype.onclose = function(msg, client) {
  client.readyState = 'CLOSING';
  if (msg) {
    // If there is msg we know the following:
    // 4 characters status code (1000-4999)
    // rest is msg payload
    var msg_str = msg.toString();
    msg = {
      code: msg_str.substr(0, 4),
      reason: msg_str.substr(4, msg_str.length),
    };
  } else {
    msg = {};
  }

  client.close(msg);
};

Server.prototype.close = function(reason, code) {
  var msg = {
    code: code || 1000,
    reason: reason || 'Connection successfully closed',
  };

  var i = 0;
  while (this._netserver._serverHandle.clients.length != 0) {
    this._netserver._serverHandle.clients[i].readyState = 'CLOSING';
    this._netserver._serverHandle.clients[i].close(msg);
  }

  this._netserver.close();
  this.emit('close', msg);
};

Server.prototype.broadcast = function(msg, options) {
  if (options) {
    var mask = options.mask || true;
    var binary = options.binary || false;
    var compress = options.compress;
    if (compress) {
      // Currently not supported, needs zlib
      this.onError('Compression is not supported');
    }
  }
  var buff = native.send(msg, binary, mask, compress);

  var self = this;
  this._netserver._serverHandle.clients.forEach(function each(client) {
    if (client.readyState === 'OPEN') {
      self._netserver._serverHandle.sendFrame(buff, client);
    }
  });
};

Server.prototype.address = function() {
  return this._netserver.address();
};

Server.prototype.onError = function(err) {
  this.emit('error', err);
};

WebsocketClient.prototype.send = function(message, opts) {
  if (opts) {
    var mask = opts.mask;
    var binary = opts.binary;
    var compress = opts.compress;
    if (compress) {
      // Currently not supported, needs zlib
      this._handle.onError('Compression is not supported');
    }
  }
  var buff = native.send(message, binary, mask, compress);
  if (buff) {
    this._handle.sendFrame(buff, this);
  }
};

WebsocketClient.prototype.close = function(msg) {
  msg = {
    reason: msg.reason || 'Connection successfully closed',
    code: msg.code || 1000,
  };

  var buff = native.close(msg.reason, msg.code);
  this._handle.sendFrame(buff, this);
  this.emit('close', msg);
  this._socket.end();
  var id = this._handle.clients.indexOf(this);
  this._handle.clients.splice(id, 1);
};

WebsocketClient.prototype.onError = function(err) {
  this.emit('error', err);
};

WebSocketHandle.prototype.onmessage = function(msg) {
  this.client.emit('message', msg);
};

WebSocketHandle.prototype.ondata = function(data) {
  native.wsReceive(this, data, this);
};

WebSocketHandle.prototype.onhandshakedone = function(remaining) {
  this.client.emit('open');
  this.client._firstMessage = false;
  if (remaining) {
    this.ondata(remaining);
  }
};

WebSocketHandle.prototype.onError = function(err) {
  this.client.emit('error', err);
};

WebSocketHandle.prototype.onclose = function(msg) {
  if (msg) {
    // If there is msg we know the following:
    // 4 characters status code (1000-4999)
    // rest is msg payload
    var msg_str = msg.toString();
    msg = {
      code: msg_str.substr(0, 4),
      reason: msg_str.substr(4, msg_str.length),
    };
  } else {
    msg = {};
  }

  this.client.emit('close', msg);
  for (var i = 0; i < this.pings.length; i++) {
    clearInterval(this.pings[i].timer);
  }
  this.client._socket.end();
};

WebSocketHandle.prototype.sendFrame = function(msg, cb) {
  if (this.connected) {
    if (typeof cb == 'function') {
      this.client._socket.write(msg, cb);
    } else {
      this.client._socket.write(msg);
    }
  } else {
    this.onError('Underlying socket connection is closed');
  }
};

WebSocketHandle.prototype.pong = function(msg) {
  this.client._socket.write(native.ping(false, msg, true));
};

WebSocketHandle.prototype.onpingresp = function(msg) {
  for (var i = 0; i < this.pings.length; i++) {
    if (this.pings[i].id == msg) {
      clearInterval(this.pings[i].timer);
      this.pings[i].callback(msg);
      this.pings.splice(i, 1);
      return;
    }
  }
};

function sendHandshake(jsref, host, path) {
  return native.prepareHandshake(jsref, host, path);
}

Websocket.prototype.connect = function(url, port, path, callback) {
  var host = url.toString() || '127.0.0.1';
  path = path || '/';

  var emit_type = 'connect';

  if (host.substr(0, 3) == 'wss') {
    this._secure = true;
    if (!tls) {
      this._handle.onError('TLS module was not found!');
    }
    port = port || 443;
    host = host.substr(6);
    this._socket = tls.connect(port, host);
    emit_type = 'secureConnect';
  } else if (host.substr(0, 2) == 'ws') {
    port = port || 80;
    this._socket = new net.Socket();
    host = host.substr(5);
  } else {
    port = port || 80;
    this._socket = new net.Socket();
  }

  if (typeof callback == 'function') {
    this.on('open', callback);
  }

  var self = this;

  this._socket.on(emit_type, function() {
    self._handle.connected = true;
    self._socket.write(sendHandshake(self._handle, host, path));
  });

  this._socket.on('end', function() {
    self._handle.connected = false;
  });
  if (emit_type == 'connect') {
    this._socket.connect(port, host);
  }

  this._socket.on('data', function(data) {
    if (self._firstMessage) {
      var remaining_data = native.parseHandshakeData(data, self._handle);
      self._handle.onhandshakedone(remaining_data);
    } else {
      self._handle.ondata(data);
    }
  });
};

Websocket.prototype.close = function(message, code, cb) {
  this._handle.sendFrame(native.close(message, code), cb);
};

Websocket.prototype.ping = function(message, mask, cb) {
  var self = this;
  var obj = {
    id: message,
    callback: cb,
    timer: setTimeout(function() {
      self.close('Ping timeout limit exceeded', 1002);
    }, 30000),
  };
  this._handle.pings.push(obj);
  this._handle.sendFrame(native.ping(true, message, mask));
};

Websocket.prototype.send = function(message, opts, cb) {
  if (opts) {
    var mask = opts.mask;
    var binary = opts.binary;
    var compress = opts.compress;
    if (compress) {
      // Currently not supported, needs zlib
      this._handle.onError('Compression is not supported');
    }
  }
  var buff = native.send(message, binary, mask, compress);
  if (buff) {
    this._handle.sendFrame(buff, cb);
  }
};

exports.Websocket = Websocket;
exports.Server = Server;
