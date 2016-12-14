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
var net = require('net');
var HTTPParser = process.binding(process.binding.httpparser).HTTPParser;
var IncomingMessage = require('http_incoming').IncomingMessage;
var OutgoingMessage = require('http_outgoing').OutgoingMessage;
var Buffer = require('buffer');
var common = require('http_common');


function ClientRequest(options, cb) {
  var self = this;
  OutgoingMessage.call(self);

  // get port, host and method.
  var port = options.port = options.port || 80;
  var host = options.host = options.hostname || options.host || '127.0.0.1';
  var method = options.method || 'GET';

  self.path = options.path || '/';

  // If `options` contains header information, save it.
  if (options.headers) {
    var keys = Object.keys(options.headers);
    for (var i = 0, l = keys.length; i < l; i++) {
      var key = keys[i];
      self.setHeader(key, options.headers[key]);
    }
  }

  // Register response event handler.
  if (cb) {
    self.once('response', cb);
  }

  // Create socket.
  var conn = new net.Socket();

  // connect server.
  conn.connect(port, host);

  // setup connection information.
  setupConnection(self, conn);

  // store first header line to be sent.
  var firstHeaderLine = method + ' ' + self.path + ' HTTP/1.1\r\n';
  self._storeHeader(firstHeaderLine);
}

util.inherits(ClientRequest, OutgoingMessage);

exports.ClientRequest = ClientRequest;


function setupConnection(req, socket) {
  var parser = common.createHTTPParser();
  parser.reinitialize(HTTPParser.RESPONSE);
  req.socket = socket;
  req.connection = socket;
  parser.socket = socket;
  parser.incoming = null;
  parser._headers = {};
  req.parser = parser;

  socket.parser = parser;
  socket._httpMessage = req;

  parser.onIncoming = parserOnIncomingClient;
  socket.on('error', socketOnError);
  socket.on('data', socketOnData);
  socket.on('end', socketOnEnd);
  socket.on('close', socketOnClose);

  // socket emitted when a socket is assigned to req
  process.nextTick(function() {
    req.emit('socket', socket);
  });
}


function socketOnClose() {
  var socket = this;
  var req = socket._httpMessage;
  var parser = socket.parser;

  socket.read();

  req.emit('close');

  if (req.res && req.res.readable) {
    // Socket closed before we emitted 'end'
    var res = req.res;
    res.on('end', function() {
      res.emit('close');
    });
    res.push(null);
  }
  else if (!req.res) {
    // socket closed before response starts.
    var err = new Error('socket hang up');
    req.emit('error', err);
  }

  if (parser) {
    // unref all links to parser, make parser GCed
    parser.finish();
    parser = null;
    socket.parser = null;
    req.parser = null;
  }
}


function socketOnError(er) {
  var socket = this;
  var parser = socket.parser;

  if (parser) {
    // unref all links to parser, make parser GCed
    parser.finish();
    parser = null;
    socket.parser = null;
  }
  socket.destroy();
}


function socketOnData(d) {
  var socket = this;
  var req = this._httpMessage;
  var parser = this.parser;

  var ret = parser.execute(d);
  if (ret instanceof Error) {
    // unref all links to parser, make parser GCed
    parser.finish();
    parser = null;
    socket.parser = null;
    req.parser = null;

    socket.destroy();
    req.emit('error', ret);
  }
}


function socketOnEnd() {
  var socket = this;
  var req = this._httpMessage;
  var parser = this.parser;

  if (parser) {
    // unref all links to parser, make parser GCed
    parser.finish();
    parser = null;
    socket.parser = null;
    req.parser = null;
  }

  socket.destroy();
}



// This is called by parserOnHeadersComplete after response header is parsed.
// TODO: keepalive support
function parserOnIncomingClient(res, shouldKeepAlive) {
  var socket = this.socket;
  var req = socket._httpMessage;

  if (req.res) {
    // server sent responses twice.
    socket.destroy();
    return;
  }
  req.res = res;

  res.req = req;

  res.on('end', responseOnEnd);

  req.emit('response', res);

  // response to HEAD req has no body
  var isHeadResponse =  (req.method == 'HEAD');

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


ClientRequest.prototype.setTimeout = function(ms, cb) {
  var self = this;

  if (cb) self.once('timeout', cb);

  var emitTimeout = function() {
    self.emit('timeout');
  };

  // In IoT.js, socket is already assigned,
  // thus, it is sufficient to trigger timeout on socket 'connect' event.
  this.socket.once('connect', function() {
    self.socket.setTimeout(ms, emitTimeout);
  });

};
