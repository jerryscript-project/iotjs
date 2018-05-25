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
 var tls = require('tls');

util.inherits(Websocket, EventEmitter);

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

WebSocketHandle.prototype.onmessage = function(msg) {
  this.client.emit('message', msg);
};

WebSocketHandle.prototype.ondata = function(data) {
  native.wsReceive(this, data);
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
      var remaining_data = native.parseHandshakeData(data);
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
