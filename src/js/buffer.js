/* Copyright 2015 Samsung Electronics Co., Ltd.
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


var buffer = process.binding(process.binding.buffer);
var alloc = buffer.alloc;
var kMaxLength = buffer.kMaxLength;
var util = require('util');


function Buffer(subject, encoding) {
  if (!util.isBuffer(this)) {
    return new Buffer(subject, encoding);
  }

  if (util.isNumber(subject)) {
    this.length = subject > 0 ? subject >>> 0 : 0;
  } else if (util.isString(subject)) {
    this.length = Buffer.byteLength(subject);
  }

  alloc(this, this.length);

  if (util.isString(subject)) {
    this.write(subject, encoding);
  } else if (util.isBuffer(subject)) {
    subject.copy(this, 0, 0, this.length);
  }
};


Buffer.byteLength = function(str, enc) {
  return str.length;
};


Buffer.concat = function(list) {
  if (!util.isArray(list)) {
    throw new TypeError(
        '1st parameter for Buffer.concat() should be array of Buffer');
  }

  length = 0;
  for (var i = 0; i < list.length; ++i) {
    if (!util.isBuffer(list[i])) {
      throw new TypeError(
          '1st parameter for Buffer.concat() should be array of Buffer');
    }
    length += list[i].length;
  }

  var buffer = new Buffer(length);
  var pos = 0;
  for (var i = 0; i < list.length; ++i) {
    list[i].copy(buffer, pos);
    pos += list[i].length;
  }

  return buffer;
};


Buffer.prototype.write = function(string, offset, length, encoding) {
  // buffer.write(string)
  if (util.isUndefined(offset)) {
    encoding = 'utf8';
    length = this.length;
    offset = 0;
  }
  // buffer.write(string, encoding)
  if (util.isUndefined(length) && util.isString(offset)) {
    encoding = offset;
    length = this.length;
    offset = 0;
  }
  // buffer.write(string, offset, length, encoding)
  offset = offset >>> 0;
  if (util.isNumber(length)) {
    length = length >>> 0;
  } else {
    encoding = length;
    length = undefined;
  }

  var remaining = this.length - offset;
  if (util.isUndefined(length) || length > remaining) {
    length = remaining;
  }
  //encoding = !!encoding ? (encoding + '').toLowerCase() : 'utf8';

  if (length < 0 || offset < 0) {
    throw new Error('attempt to write outside buffer bounds');
  }

  return this._write(string, offset, length);
};


Buffer.prototype.toString = function(encoding, start, end) {
  return this._toString();
};


buffer.setupBufferJs(Buffer);

module.exports = Buffer;
module.exports.Buffer = Buffer;
