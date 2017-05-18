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
var HTTPParser = process.binding(process.binding.httpparser).HTTPParser;
var IncomingMessage = require('http_incoming').IncomingMessage;
var OutgoingMessage = require('http_outgoing').OutgoingMessage;



var createHTTPParser = function() {
  // REQUEST is the default type.
  // For RESPONSE, use HTTPParser.reinitialize(HTTPParser.RESPONSE)
  var parser = new HTTPParser(HTTPParser.REQUEST);
  // cb during  http parsing from C side(http_parser)
  parser.OnHeaders = parserOnHeaders;
  parser.OnHeadersComplete = parserOnHeadersComplete;
  parser.OnBody = parserOnBody;
  parser.OnMessageComplete = parserOnMessageComplete;
  return parser;
};

exports.createHTTPParser = createHTTPParser;


// This is called when parsing of incoming http msg done
function parserOnMessageComplete() {
  var stream = this.incoming;

  if (stream) {
    stream.complete = true;
    // no more data from incoming, stream will emit 'end' event
    stream.push(null);
  }

  stream.socket.resume();
}


// This is called when header part in http msg is parsed.
function parserOnHeadersComplete(info) {
  var headers = info.headers;
  var url = info.url;

  if (!url) {
    url = this._url;
    this.url = "";
  }

  if (!headers) {
    headers = this._headers;
    this._headers = [];
  }


  this.incoming = new IncomingMessage(this.socket);
  this.incoming.url = url;

  // add header fields of headers to incoming.headers
  this.incoming.addHeaders(headers);

  if (util.isNumber(info.method)) {
    // for server
    this.incoming.method = HTTPParser.methods[info.method];
  } else {
    // for client
    this.incoming.statusCode = info.status;
    this.incoming.statusMessage = info.status_msg;
  }

  // For client side, if response to 'HEAD' request, we will skip parsing body
  var skipBody = this.onIncoming(this.incoming, info.shouldkeepalive);

  return skipBody;
}


// parserOnBody is called when HTTPParser parses http msg(incoming) and
// get body part(buf from start at length of len)
function parserOnBody(buf, start, len) {
  var stream = this.incoming;

  if (!stream) {
    return;
  }

  // Push body part into incoming stream, which will emit 'data' event
  var body = buf.slice(start, start+len);
  stream.push(body);
}


// This is called when http header is fragmented and
// HTTPParser sends it to JS in separate pieces.
function parserOnHeaders(headers, url) {
  // push new header parts into existing array
  this._headers.push.apply(this._headers, headers);
  if (url) {
    this._url += url;
  }
}
