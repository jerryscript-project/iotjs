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

function Url(url) {
  this.protocol = null;
  this.auth = null;
  this.host = null;
  this.port = null;
  this.hostname = null;
  this.afterHost = null;
  if (url) {
    this.parse(url);
  }
}

var protocolPattern = /^[a-z0-9.+-]+:/i;
var portPattern = /:[0-9]*$/;

var hostnameMaxLen = 255;
var maxPortNumber = 65535;

var supportedProtocol = {
  'http': true,
  'http:': true,
  'https': true,
  'https:': true,
};

// The parser
function urlParse(url) {
  if (url instanceof Url) {
    return url;
  }

  var urlObject = new Url();
  urlObject.parse(url);
  return urlObject;
}


Url.prototype.parse = function(url) {
  if (typeof url !== 'string') {
    throw new TypeError('ERR_INVALID_ARG_TYPE', 'url', 'string', url);
  }

  var rest = url;

  // Get the protocol first
  var proto = protocolPattern.exec(rest);
  if (proto) {
    proto = proto[0];
    var lowerProto = proto.toLowerCase();
    if (!supportedProtocol[lowerProto]) {
      throw new TypeError('Unsupported protocol');
    }

    proto = lowerProto;
    rest = rest.slice(proto.length);

    // Because there is a protocol, there has to be double slashes
    // If there are not, the url is invalid
    var slashes = rest.charCodeAt(0) === 47 &&
                  rest.charCodeAt(1) === 47;
    if (!slashes) {
      throw new TypeError('Invalid URL: there is a protocol but no slashes');
    } else {
      rest = rest.slice(2);
      // If there is a colon at the end of the protocol, remove it
      if (lowerProto.charCodeAt(proto.length - 1) === 58) {
        proto = proto.slice(0, proto.length - 1);
      }
    }

    this.protocol = proto;
  } else {
    throw new TypeError('Invalid URL: no protocol');
  }

  this.host = rest;
  if (this.host === '') {
    throw new TypeError('Invalid Url: there is no host');
  }

  // Now we will check the host for authentication, port number
  // and the hostname itself
  var atSign = -1;
  for (var i = 0; i < rest.length; ++i) {
    switch (rest.charCodeAt(i)) {
      case 9:
      case 10:
      case 13:
      case 32:
      case 34:
      case 37:
      case 39:
      case 59:
      case 60:
      case 62:
      case 92:
      case 94:
      case 96:
      case 123:
      case 124:
      case 125:
        // Characters that are never ever allowed in a hostname from RFC 2396
        throw new TypeError('Invalid URL: prohibited character in the host');
      case 35:
      case 47:
      case 63:
        // These characters end the host ('#', '/', '?')
        // We don't need the parts after the host, so we get rid of it
        // this.afterHost will store it in case it is needed for something
        this.afterHost = rest.slice(i);
        this.host = rest = rest.slice(0, i);
        break;
      case 64:
        // '@' sign
        // At this point, either we have an explicit point where the
        // auth portion cannot go past, or the last @ char is the decider.
        atSign = i;
        break;
    }
  }

  // If there is an '@' sign, than we have an auth part
  if (atSign !== -1) {
    this.auth = decodeURIComponent(rest.slice(0, atSign));
    this.host = rest = rest.slice(this.auth.length + 1);
  }
  // Split the host into hostname and port number
  parseHost(this);

  if (this.hostname === '') {
    throw new TypeError('Invalid URL: there is no hostname');
  }

  // if hostname begins with [ and ends with ]
  // assume that it's an IPv6 address.
  var ipv6Hostname = this.hostname.charCodeAt(0) === 91 &&
                     this.hostname.charCodeAt(this.hostname.length - 1) === 93;

  if (!ipv6Hostname) {
    validateHostname(this.hostname);
  }

  if (this.hostname.length > hostnameMaxLen) {
    throw new TypeError('Invalid URL: the hostname is too long');
  }
};


function parseHost(obj) {
  if (!(obj instanceof Url)) {
    throw new TypeError('parseHost expects Url object as argument');
  }

  var host = obj.host;
  var port = portPattern.exec(host);
  if (port) {
    // exec returns an array, we only need the string itself
    port = port[0];
    port = port.slice(1);
    if (port > maxPortNumber) {
      throw new TypeError('Port number cannot be bigger than ' + maxPortNumber);
    }
    host = host.slice(0, host.length - port.length - 1);
    obj.port = port;
  }
  if (host) {
    obj.hostname = host;
  }
}


function validateHostname(hostname) {
  for (var i = 0; i < hostname.length; ++i) {
    var code = hostname.charCodeAt(i);
    var isValid = (code >= 97 && code <= 122) ||
                    code === 46 ||
                    (code >= 65 && code <= 90) ||
                    (code >= 48 && code <= 57) ||
                    code === 45 ||
                    code === 43 ||
                    code === 95 ||
                    code > 127;

    // Invalid host character
    if (!isValid) {
      throw new TypeError('Invalid URL: invalid character in hostname');
    }
  }
}


function urlFormat(urlObject) {
  // ensure it's an object, and not a string url.
  // If it's an object, this is a no-op.
  // this way, you can call urlParse() on strings
  // to clean up potentially wonky urls.
  if (typeof urlObject === 'string') {
    urlObject = urlParse(urlObject);
  } else if (typeof urlObject !== 'object' || urlObject === null) {
    throw new TypeError('ERR_INVALID_ARG_TYPE', 'urlObject',
                               ['Object', 'string'], urlObject);
  } else if (!(urlObject instanceof Url)) {
    throw new TypeError('urlFormat expects Url object as argument');
  }

  return urlObject.format();
}


Url.prototype.format = function format() {
  var auth = this.auth || '';
  if (auth) {
    auth += '@';
  }

  var protocol = this.protocol || '';
  var host = '';

  if (this.host) {
    host = auth + this.host;
  } else if (this.hostname) {
    host = auth + (
      this.hostname.indexOf(':') === -1 ?
        this.hostname :
        '[' + this.hostname + ']'
    );
    if (this.port) {
      host += ':' + this.port;
    }
  }

  if (protocol && protocol.charCodeAt(protocol.length - 1) !== 58) {
    protocol += ':';
  }

  return protocol + '//' + host;
};

exports.Url = Url;
exports.parse = urlParse;
exports.format = urlFormat;
