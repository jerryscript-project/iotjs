/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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

var tls = require('tls');
var net = require('net');
var ClientRequest = require('http_client').ClientRequest;
var HTTPServer = require('http_server');
var util = require('util');

exports.request = function(options, cb) {
  options.port = options.port || 443;
  // Create socket.
  var socket = new tls.TLSSocket(new net.Socket(), options);

  return new ClientRequest(options, cb, socket);
};

function Server(options, requestListener) {
  if (!(this instanceof Server)) {
    return new Server(options, requestListener);
  }
  options.allowHalfOpen = true;
  tls.Server.call(this, options, HTTPServer.connectionListener);

  HTTPServer.initServer.call(this, options, requestListener);
}
util.inherits(Server, tls.Server);

Server.prototype.setTimeout = function(ms, cb) {
  this.timeout = ms;
  if (cb) {
    this.on('timeout', cb);
  }
};

exports.createServer = function(options, requestListener) {
  return new Server(options, requestListener);
};

exports.get = function(options, cb) {
  var req = exports.request(options, cb);
  req.end();
  return req;
};
