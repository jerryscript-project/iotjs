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
var OutgoingMessage = require('http_outgoing').OutgoingMessage;
var common = require('http_common');
var HTTPParser = require('http_parser').HTTPParser;

function ClientRequest(options, cb, socket) {
  OutgoingMessage.call(this);

  // get port, host and method.
  var method = options.method || 'GET';
  var path = options.path || '/';
  options.host = options.hostname || options.host || '127.0.0.1';

  // If `options` contains header information, save it.
  if (options.headers) {
    var keys = Object.keys(options.headers);
    for (var i = 0, l = keys.length; i < l; i++) {
      var key = keys[i];
      this.setHeader(key, options.headers[key]);
    }
  }

  if (options.host && !this.getHeader('host')) {
    var hostHeader = options.host;
    if (this._port && +this._port !== 80) {
      hostHeader += ':' + this._port;
    }
    this.setHeader('Host', hostHeader);
  }

  // store first header line to be sent.
  this._storeHeader(method + ' ' + path + ' HTTP/1.1\r\n');

  // Register response event handler.
  if (cb) {
    this.once('response', cb);
  }

  this.socket = socket;
  this.options = options;
}

util.inherits(ClientRequest, OutgoingMessage);

exports.ClientRequest = ClientRequest;

ClientRequest.prototype.end = function(data, encoding, callback) {
  var self = this;

  // connect server.
  this.socket.connect(this.options, function() {
    self._connected = true;
    OutgoingMessage.prototype.end.call(self, data, encoding, callback);
  });

  // setup connection information.
  setupConnection(this);
};

function setupConnection(req) {
  var socket = req.socket;
  var parser = common.createHTTPParser(HTTPParser.RESPONSE);
  socket.parser = parser;
  socket._httpMessage = req;

  parser.socket = socket;
  parser.incoming = null;
  parser._headers = [];
  parser.onIncoming = parserOnIncomingClient;

  req.parser = parser;

  socket.on('error', socketOnError);
  socket.on('data', socketOnData);
  socket.on('end', socketOnEnd);
  socket.on('close', socketOnClose);

  // socket emitted when a socket is assigned to req
  process.nextTick(function() {
    req.emit('socket', socket);
  });
}

function cleanUpSocket(socket) {
  var parser = socket.parser;
  var req = socket._httpMessage;

  if (parser) {
    // unref all links to parser, make parser GCed
    parser.finish();
    parser = null;
    socket.parser = null;
    req.parser = null;
  }

  socket.destroy();
}

function emitError(socket, err) {
  var req = socket._httpMessage;

  if (err) {
    var host;
    if ((host = req.getHeader('host'))) {
      err.message += ': ' + host;
    }
    req.emit('error', err);
  }
}

function socketOnClose() {
  var socket = this;
  var req = socket._httpMessage;

  req.emit('close');

  if (req.res && req.res.readable) {
    // Socket closed before we emitted 'end'
    var res = req.res;
    res.on('end', function() {
      res.emit('close');
    });
    res.push(null);
  }

  cleanUpSocket(this);
}

function socketOnError(err) {
  cleanUpSocket(this);
  emitError(this, err);
}

function socketOnData(d) {
  var socket = this;
  var req = this._httpMessage;
  var parser = this.parser;

  var ret = parser.execute(d);
  if (ret instanceof Error) {
    cleanUpSocket(socket);
    req.emit('error', ret);
  }
}

function socketOnEnd() {
  cleanUpSocket(this);
}

// This is called by parserOnHeadersComplete after response header is parsed.
// TODO: keepalive support
function parserOnIncomingClient(res/* , shouldKeepAlive */) {
  var socket = this.socket;
  var req = socket._httpMessage;

  if (req.res) {
    // server sent responses twice.
    socket.destroy();
    return false;
  }
  req.res = res;

  res.req = req;

  res.on('end', responseOnEnd);

  req.emit('response', res);

  // response to HEAD req has no body
  var isHeadResponse = (req.method == 'HEAD');

  return isHeadResponse;

}

var responseOnEnd = function() {
  var res = this;
  var req = res.req;
  var socket = req.socket;

  if (socket._socketState.writable) {
    socket.destroySoon();
  }
};

ClientRequest.prototype.abort = function() {
  this.emit('abort');
  if (this.socket) {
    cleanUpSocket(this.socket);
  }
};

ClientRequest.prototype.setTimeout = function(ms, cb) {
  var self = this;

  if (cb) self.once('timeout', cb);

  setTimeout(function() {
    self.emit('timeout');
  }, ms);
};
