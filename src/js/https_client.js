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

var util = require('util');
var incoming = require('https_incoming');
var stream = require('stream');
var Buffer = require('buffer');
var httpsNative = process.binding(process.binding.https);

var methods = {'0': 'DELETE', '1': 'GET', '2': 'HEAD', '3': 'POST',
    '4': 'PUT', '5': 'CONNECT', '6': 'OPTIONS', '7': 'TRACE'};
exports.METHODS = methods;

function ClientRequest(options, cb) {
  this.stream = stream.Writable.call(this, options);

  // get port, host and method.
  var port = options.port = options.port || 443;
  var host = options.host = options.hostname || options.host || '127.0.0.1';
  var path = options.path || '/';
  var protocol = options.protocol || 'https:';

  this.host = protocol + '//' + host + ':' + port + path;
  this.method = options.method || 'GET';
  this.ca = options.ca || '';
  this.cert = options.cert || '';
  this.key = options.key || '';

  if (options.rejectUnauthorized == null) {
    this.rejectUnauthorized = true;
  } else {
    this.rejectUnauthorized = options.rejectUnauthorized;
  }

  var isMethodGood = false;
  for (var key in methods) {
    if (methods.hasOwnProperty(key)) {
      if (this.method === methods[key]) {
        isMethodGood = true;
        break;
      }
    }
  }

  if (!isMethodGood) {
    var err = new Error('Incorrect options.method.')
    this.emit('error', err);
    return;
  }

  this._incoming = new incoming.IncomingMessage(this);
  this._incoming.url = this.host;
  this._incoming.method = this.method;
  this.aborted = null;

  // Register response event handler.
  if (cb) {
    this.once('response', cb);
  }
  this.once('finish', this.onFinish);

  httpsNative.createRequest(this);

  if (options.auth) {
    var headerString = 'Authorization: Basic ' + toBase64(options.auth);
    httpsNative.addHeader(headerString, this);
  }
  if (options.headers) {
    var keys = Object.keys(options.headers);
    for (var i = 0, l = keys.length; i < l; i++) {
      var key = keys[i];
      httpsNative.addHeader(key + ': ' + options.headers[key], this);
    }
  }
  httpsNative.sendRequest(this);
}

util.inherits(ClientRequest, stream.Writable);

// Concrete stream overriding the empty underlying _write method.
ClientRequest.prototype._write = function(chunk, callback, onwrite) {
  httpsNative._write(this, chunk.toString(), callback, onwrite);
};

ClientRequest.prototype.headersComplete = function() {
  var self = this;
  self.emit('response', self._incoming);
  return (self.method == 'HEAD');
};

ClientRequest.prototype.onError = function(ret) {
  this.emit('error', ret);
};

ClientRequest.prototype.onFinish = function() {
  httpsNative.finishRequest(this);
};

ClientRequest.prototype.setTimeout = function(ms, cb) {
  this._incoming.setTimeout(ms, cb);
};

ClientRequest.prototype.abort = function(doNotEmit) {
  if (!this.aborted) {
    httpsNative.abort(this);
    var date = new Date();
    this.aborted = date.getTime();

    if (this._incoming.parser) {
      this._incoming.parser.finish();
      this._incoming.parser = null;
    }

    if (!doNotEmit) {
      this.emit('abort');
    }
  }
};

exports.ClientRequest = ClientRequest;

function toBase64(input) {
  var output = '';
  var chr1, chr2, chr3, enc1, enc2, enc3, enc4;
  var i = 0;
  //Convert to UTF-8
  input = Buffer(input).toString();
  var _keyStr = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz' +
    '0123456789+/=';
  while (i < input.length) {
    chr1 = input.charCodeAt(i++);
    chr2 = input.charCodeAt(i++);
    chr3 = input.charCodeAt(i++);

    enc1 = chr1 >> 2;
    enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
    enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
    enc4 = chr3 & 63;

    if (isNaN(chr2)) {
      enc3 = enc4 = 64;
    } else if (isNaN(chr3)) {
      enc4 = 64;
    }

    output = output +
      _keyStr.charAt(enc1) + _keyStr.charAt(enc2) +
      _keyStr.charAt(enc3) + _keyStr.charAt(enc4);
  }
  return output;
}
