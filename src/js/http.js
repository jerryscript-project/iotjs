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

var Server = require('http_server').Server;
var client = require('http_client');
var HTTPParser = process.binding(process.binding.httpparser).HTTPParser;


var ClientRequest = exports.ClientRequest = client.ClientRequest;


exports.request = function(options, cb) {
  return new ClientRequest(options, cb);
};


exports.createServer = function(requestListener){
  return new Server(requestListener);
};


exports.METHODS = HTTPParser.methods;


exports.get = function(options, cb) {
  var req = exports.request(options, cb);
  req.end();
  return req;
};
