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
var stream = require('stream');
var Buffer = require('buffer');
var httpsNative = process.binding(process.binding.https);
var HTTPParser = process.binding(process.binding.httpparser).HTTPParser;

function IncomingMessage(clientRequest) {
  stream.Readable.call(this);
  this.clientRequest = clientRequest;

  this.headers = {};
  this.started = false;
  this.completed = false;
  this.timeoutCallback = null;
  // for response (client)
  this.statusCode = null;
  this.statusMessage = null;
  this.url = null;
  this.method = null;

  this.parser = createHTTPParser(this);

  this.onData = cbOnData;
  this.onClosed = cbOnClosed;
  this.onEnd = cbOnEnd;
  this.onError = cbOnError;
  this.onSocket = cbOnSocket;
  this.onTimeout = cbOnTimeout;
  this.onWritable = cbOnWritable;
}

util.inherits(IncomingMessage, stream.Readable);
exports.IncomingMessage = IncomingMessage;

IncomingMessage.prototype.read = function(n) {
  this.read = stream.Readable.prototype.read;
  return this.read(n);
};

IncomingMessage.prototype.setTimeout = function(ms, cb) {
  if (cb) {
    this.timeoutCallback = cb;
    this.once('timeout', cb);
  }
  httpsNative.setTimeout(ms, this.clientRequest);
};

IncomingMessage.prototype.addHeaders = function(headers) {
  if (!this.headers) {
    this.headers = {};
  }
  if (!headers) {
    return;
  }

  for (var i = 0; i < headers.length; i = i + 2) {
    this.headers[headers[i]] = headers[i + 1];
  }
};

// HTTP PARSER Constructor
var createHTTPParser = function(incoming) {
  var parser = new HTTPParser(HTTPParser.RESPONSE);
  parser.incoming = incoming;
  // cb during http parsing from C side(http_parser)
  parser.OnHeaders = parserOnHeaders;
  parser.OnHeadersComplete = parserOnHeadersComplete;
  parser.OnBody = parserOnBody;
  parser.OnMessageComplete = parserOnMessageComplete;
  return parser;
};

// ------------- HTTP PARSER CALLBACKS -------------
// This is called when http header is fragmented and
// HTTPParser sends it to JS in separate pieces.
function parserOnHeaders(headers, url) {
  var parser = this;
  parser.incoming.addHeaders(headers);
}

// This is called when header part in http msg is parsed.
function parserOnHeadersComplete(info) {
  var parser = this;
  var headers = info.headers;

  if (!headers) {
    headers = parser._headers;
    parser.incoming.addHeaders(headers);
    parser._headers = {};
  } else {
    parser.incoming.addHeaders(headers);
  }

  // add header fields of headers to incoming.headers
  parser.incoming.addHeaders(headers);
  parser.incoming.statusCode = info.status;
  parser.incoming.statusMessage = info.status_msg;
  parser.incoming.started = true;

  // For client side, if response to 'HEAD' request, we will skip parsing body
  if (parser.incoming.statusCode == 100) {
    return false;
  }
  return parser.incoming.clientRequest.headersComplete();
}

// parserOnBody is called when HTTPParser parses http msg(incoming) and
// get body part(buf from start at length of len)
function parserOnBody(buf, start, len) {
  var parser = this;
  var incoming = parser.incoming;

  if (!incoming) {
    return;
  }

  // Push body part into incoming stream, which will emit 'data' event
  var body = buf.slice(start, start + len);
  incoming.push(body);
}

// This is called when parsing of incoming http msg done
function parserOnMessageComplete() {
  var parser = this;
  var incoming = parser.incoming;

  if (incoming) {
    if (incoming.statusCode == 100) {
      incoming.headers = {};
      incoming.statusCode = null;
      incoming.statusMessage = null;
      incoming.started = false;
    } else {
      incoming.completed = true;
      // no more data from incoming, stream will emit 'end' event
      incoming.push(null);
    }
  }
}

//------------ LIBCURL PARSER CALLBACKS -----------------
// Called by libcurl when Request is Done. Finish parser and unref
function cbOnEnd() {
  var incoming = this;
  var parser = incoming.parser;
  if (parser) {
    // Unref all links to parser, make parser GCed
    parser.finish();
    parser = null;
    incoming.parser = null;
  }
}

function cbOnClosed() {
  var incoming = this;
  var parser = incoming.parser;
  var clientRequest = incoming.clientRequest;

  if (incoming.started && !incoming.completed) {
    // Socket closed before we emitted 'end'
    incoming.on('end', function() {
      incoming.emit('close');
      clientRequest.emit('close');
    });
    incoming.push(null);
  } else if (!incoming.started) {
    incoming.emit('close');
    clientRequest.emit('close');
    // Socket closed before response starts.
    var err = new Error('Could Not Start Connection');
    clientRequest.onError(err);
  } else {
    clientRequest.emit('close');
  }

  if (parser) {
    // Unref all links to parser, make parser GCed
    parser.finish();
    parser = null;
    incoming.parser = null;
  }
}

// Called by libcurl when Request is Done. Finish parser and unref
function cbOnData(chunk) {
  var incoming = this;

  if (!incoming) {
    return false;
  }

  var parser = incoming.parser;
  var clientRequest = incoming.clientRequest;

  chunk = new Buffer(chunk);
  var ret = parser.execute(chunk);

  if (ret instanceof Error) {
    parser.finish();
    // Unref all links to parser, make parser GCed
    parser = null;
    clientRequest.onError(ret);
    return false;
  }
  return true;
}

function cbOnError(er) {
  var incoming = this;
  var clientRequest = incoming.clientRequest;
  var err = new Error(er);
  clientRequest.onError(err);
  clientRequest.abort(true);
  incoming.emit('error', err);
}

function cbOnTimeout() {
  var incoming = this;
  var clientRequest = incoming.clientRequest;
  incoming.emit('timeout');
  if (!incoming.timeoutCallback) {
    clientRequest.abort.call(clientRequest, false);
  }
  incoming.emit('aborted');
}

function cbOnSocket() {
  var incoming = this;
  var clientRequest = incoming.clientRequest;
  clientRequest.emit('socket');
}

function cbOnWritable() {
  var incoming = this;
  var clientRequest = incoming.clientRequest;
  clientRequest._readyToWrite();
}
