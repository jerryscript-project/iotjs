/* Copyright 2017 Samsung Electronics Co., Ltd. and other contributors
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
var coapBuiltin = process.binding(process.binding.coap);

exports.init = function () {
  var err = coapBuiltin.init();
  return err;
};

function Server() {
  //TODO: return server
}

exports.getServer = function () {
  return new Server();
};

function CoapError(message) {
  this.name = 'CoapError';
  this.message = message;
}

function Context(contextId) {
  this.contextId = contextId;
}
Context.prototype.addResource = function(resource) {
  //TODO: Implement
  throw new CoapError("Not implemented");
};

Context.prototype.listen = function() {
  //TODO: Implement
  throw new CoapError("Not implemented");
};

Server.prototype.newContext = function(address, port) {
  throw new CoapError("Not implemented");
  //TODO: Implement
  // Allocate context
  return new Context(null);
};

Server.prototype.sdeleteContext = function(context) {
  throw new CoapError("Not implemented");
  //TODO: Implement
  // Free context
};

function Response (nativeResponse) {
    //TODO: implement
}
//It is similar to HTTP codes
Response.prototype.setCode = function (code) {
  throw new CoapError("Not implemented");
    //TODO: implement
};

function Attribute(name, value) {
    //TODO: implement
}

Attribute.prototype.on = function (event)  {

};

function Resource (nativeResource) {
    //TODO: implement
}
Resource.prototype.addAttribute = function (name, value, handlers

Server.prototype.newResource = function(name) {
  throw new CoapError("Not implemented");
  //TODO: Implement
  return {
    resourceid: null,
    // Handlers is callbacks dictionary
    // {get, put, delete}
    add_attr: function (name, value, handlers) {
      //TODO: Implement
    };
    // Events get, put, delete
    // Callback will get request and response objects
    on: function (eventName, callback) {
      //TODO: Implement
    }
  };
};


Server.prototype.deleteResource = function(resource) {
  throw new CoapError("Not implemented");
  // Free context
};

