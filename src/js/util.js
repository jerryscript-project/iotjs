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


function isNull(arg) {
  return arg === null;
}


function isUndefined(arg) {
  return arg === undefined;
}


function isNullOrUndefined(arg) {
  return isNull(arg) || isUndefined(arg);
}


function isNumber(arg) {
  return typeof arg === 'number';
}


function isBoolean(arg) {
  return typeof arg === 'boolean';
}


function isString(arg) {
  return typeof arg === 'string';
}


function isObject(arg) {
  return typeof arg === 'object' && arg != null;
}


function isFunction(arg) {
  return typeof arg === 'function';
}


function isBuffer(arg) {
  return arg instanceof Buffer;
}


function inherits(ctor, superCtor) {
  ctor.prototype = Object.create(superCtor.prototype, {
    constructor: {
      value: ctor,
      enumerable: false,
      writable: true,
      configurable: true
    }
  });
};


function format(s) {
  if (!isString(s)) {
    var arrs = [];
    for (var i = 0; i < arguments.length; ++i) {
        arrs.push(formatValue(arguments[i]));
    }
    return arrs.join(' ');
  }

  var i = 1;
  var args = arguments;
  var str = String(s).replace(/%[sdj%]/g, function(m) {
    if (m === '%%') {
      return '%';
    }
    if (i >= args.length) {
      return m;
    }
    switch (m) {
      case '%s': return String(args[i++]);
      case '%d': return Number(args[i++]);
      case '%j': return '[JSON object]';
      default: return m;
    }
  });

  while (i < args.length) {
      str += ' ' + args[i++].toString();
  }

  return str;
}

function formatValue(v) {
  if (isUndefined(v)) {
    return 'undefined';
  } else if (isNull(v)) {
    return 'null';
  } else {
    return v.toString();
  }
}


function errnoException(err, syscall, original) {
  var errname = "error"; // uv.errname(err);
  var message = syscall + ' ' + errname;

  if (original)
    message += ' ' + original;

  var e = new Error(message);
  e.code = errname;
  e.errno = errname;
  e.syscall = syscall;

  return e;
};


function exceptionWithHostPort(err, syscall, address, port, additional) {
  var details;
  if (port && port > 0) {
    details = address + ':' + port;
  } else {
    details = address;
  }

  if (additional) {
    details += ' - Local (' + additional + ')';
  }

  var ex = exports.errnoException(err, syscall, details);
  ex.address = address;
  if (port) {
    ex.port = port;
  }

  return ex;
};


exports.isNull = isNull;
exports.isUndefined = isUndefined;
exports.isNullOrUndefined = isNullOrUndefined;
exports.isNumber = isNumber;
exports.isBoolean = isBoolean;
exports.isString = isString;
exports.isObject = isObject;
exports.isFunction = isFunction;
exports.isBuffer = isBuffer;
exports.isArray = Array.isArray;
exports.exceptionWithHostPort = exceptionWithHostPort;
exports.errnoException = errnoException;

exports.inherits = inherits;

exports.format = format;
