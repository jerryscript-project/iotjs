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
var dnsBuiltin = process.binding(process.binding.dns);

function dnsException(err, syscall, hostname) {
  var ex = new Error(syscall + ' ' + err + (hostname ? ' ' + hostname : ''));
  // TODO(hanjoung.lee@samsung.com) err should be a string (currently a number)
  ex.code = err;
  ex.errno = err;
  ex.syscall = syscall;
  if (hostname) {
    ex.hostname = hostname;
  }
  return ex;
}


exports.lookup = function lookup(hostname, options, callback) {
  var hints = 0;
  var family = -1;

  // Parse arguments
  if (!util.isString(hostname)) {
    throw TypeError('invalid argument: hostname must be a string');
  }
  if (util.isFunction(options)) {
    callback = options;
    family = 0;
  } else if (!util.isFunction(callback)) {
    throw TypeError('invalid argument: callback must be passed');
  } else if (util.isObject(options)) {
    hints = options.hints >>> 0;
    family = options.family >>> 0;

    if (hints < 0 || hints > (exports.ADDRCONFIG | exports.V4MAPPED)) {
      throw new TypeError('invalid argument: invalid hints flags');
    }
  } else if (util.isNumber(options)) {
    family = ~~options;
  } else {
    throw TypeError(
        'invalid argument: options must be either an object or number');
  }

  if (family !== 0 && family !== 4 && family !== 6)
    throw new TypeError('invalid argument: family must be 4 or 6');

  var err = dnsBuiltin.getaddrinfo(
      hostname,
      family,
      hints,
      function(err, address, family) {
        var errObj = null;
        if (err) {
          errObj = dnsException(err, 'getaddrinfo', hostname);
        }
        return callback(errObj, address, family);
      });
  return err;
};


// uv_getaddrinfo flags
exports.ADDRCONFIG = dnsBuiltin.AI_ADDRCONFIG;
exports.V4MAPPED = dnsBuiltin.AI_V4MAPPED;
