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


function checkInt(buffer, value, offset, ext, max, min) {
  if (value > max || value < min)
    throw new TypeError('value is out of bounds');
  if (offset + ext > buffer.length)
    throw new RangeError('index out of range');
}


function checkOffset(offset, ext, length) {
  if (offset + ext > length)
    throw new RangeError('index out of range');
}


function getEncodingType(encoding) {
  switch (encoding) {
    case 'hex':
      return 0;
    case 'base64':
      return 1;
    default:
      return -1;
  }
}


// Buffer constructor
// [1] new Buffer(size)
// [2] new Buffer(buffer)
// [3] new Buffer(string)
// [4] new Buffer(string, encoding)
// [5] new Buffer(array)
function Buffer(subject, encoding) {
  if (!Buffer.isBuffer(this)) {
    return new Buffer(subject, encoding);
  }

  if (typeof subject === 'number') {
    this.length = subject > 0 ? subject >>> 0 : 0;
  } else if (typeof subject === 'string') {
    this.length = Buffer.byteLength(subject, encoding);
  } else if (Buffer.isBuffer(subject) || Array.isArray(subject)) {
    this.length = subject.length;
  } else {
    throw new TypeError('Bad arguments: Buffer(string|number|Buffer|Array)');
  }

  // 'native' is the buffer object created via the C API.
  native(this, this.length);

  if (typeof subject === 'string') {
    if (typeof encoding === 'string') {
      encoding = getEncodingType(encoding);
      if (encoding != -1) {
        native.writeDecode(this, encoding, subject, 0, this.length);
      } else {
        this.write(subject);
      }
    } else {
      this.write(subject);
    }
  } else if (Buffer.isBuffer(subject)) {
    subject.copy(this);
  } else if (Array.isArray(subject)) {
    for (var i = 0; i < this.length; ++i) {
      native.writeUInt8(this, subject[i], i);
    }
  }
}


// Buffer.byteLength(string, encoding)
Buffer.byteLength = function(str, encoding) {
  var bytes = native.byteLength(str);

  if (typeof encoding === 'string') {
    /* Might be invalid for incorrectly encoded strings. */
    switch (encoding) {
      case 'hex':
        return bytes >>> 1;
      case 'base64':
        var len = str.length;

        if (len >= 4 && str.charCodeAt(len - 1) === 0x3D) {
           len--;

          if (str.charCodeAt(len - 2) === 0x3D) {
            len--;
          }
        }

        return len;
    }
  }
  return bytes;
};


// Buffer.concat(list)
Buffer.concat = function(list) {
  if (!Array.isArray(list)) {
    throw new TypeError('Bad arguments: Buffer.concat([Buffer])');
  }

  var length = 0;
  var i;
  for (i = 0; i < list.length; ++i) {
    if (!Buffer.isBuffer(list[i])) {
      throw new TypeError('Bad arguments: Buffer.concat([Buffer])');
    }
    length += list[i].length;
  }

  var buffer = new Buffer(length);
  var pos = 0;
  for (i = 0; i < list.length; ++i) {
    list[i].copy(buffer, pos);
    pos += list[i].length;
  }

  return buffer;
};


// Buffer.isBuffer(object)
Buffer.isBuffer = function(arg) {
  return arg instanceof Buffer;
};


// buffer.equals(otherBuffer)
Buffer.prototype.equals = function(otherBuffer) {
  if (!Buffer.isBuffer(otherBuffer)) {
    throw new TypeError('Bad arguments: buffer.equals(Buffer)');
  }

  return native.compare(this, otherBuffer) == 0;
};


// buffer.compare(otherBuffer)
Buffer.prototype.compare = function(otherBuffer) {
  if (!Buffer.isBuffer(otherBuffer)) {
    throw new TypeError('Bad arguments: buffer.compare(Buffer)');
  }

  return native.compare(this, otherBuffer);
};


// buffer.copy(target[, targetStart[, sourceStart[, sourceEnd]]])
// [1] buffer.copy(target)
// [2] buffer.copy(target, targetStart)
// [3] buffer.copy(target, targetStart, sourceStart)
// [4] buffer.copy(target, targetStart, sourceStart, sourceEnd)
// * targetStart - default to 0
// * sourceStart - default to 0
// * sourceEnd - default to buffer.length
Buffer.prototype.copy = function(target, targetStart, sourceStart, sourceEnd) {
  if (!Buffer.isBuffer(target)) {
    throw new TypeError('Bad arguments: buff.copy(Buffer)');
  }

  targetStart = targetStart === undefined ? 0 : ~~targetStart;
  sourceStart = sourceStart === undefined ? 0 : ~~sourceStart;
  sourceEnd = sourceEnd === undefined ? this.length : ~~ sourceEnd;

  if ((sourceEnd > sourceStart) && (targetStart < 0)) {
    throw new RangeError('Attempt to write outside buffer bounds');
  }

  return native.copy(this, target, targetStart, sourceStart, sourceEnd);
};


// buffer.write(string[, offset[, length]])
// [1] buffer.write(string)
// [2] buffer.write(string, offset)
// [3] buffer.write(string, offset, length)
// * offset - default to 0
// * length - default to buffer.length - offset
Buffer.prototype.write = function(string, offset, length, encoding) {
  if (typeof string !== 'string') {
    throw new TypeError('Bad arguments: buff.write(string)');
  }

  offset = offset === undefined ? 0 : ~~offset;
  if (string.length > 0 && (offset < 0 || offset >= this.length)) {
    throw new RangeError('Attempt to write outside buffer bounds');
  }

  var remaining = this.length - offset;
  length = length === undefined ? remaining : ~~length;

  if (typeof encoding === 'string') {
    encoding = getEncodingType(encoding);
    if (encoding != -1) {
      return native.writeDecode(this, encoding, string, offset, length);
    }
  }

  return native.write(this, string, offset, length);
};


// buff.slice([start[, end]])
// [1] buff.slice()
// [2] buff.slice(start)
// [3] buff.slice(start, end)
// * start - default to 0
// * end - default to buff.length
Buffer.prototype.slice = function(start, end) {
  start = start === undefined ? 0 : ~~start;
  end = end === undefined ? this.length : ~~end;

  return native.slice(this, start, end);
};


// buff.toString([encoding,[,start[, end]]])
// [1] buff.toString()
// [2] buff.toString(encoding)
// [3] buff.toString(encoding, start)
// [4] buff.toString(encoding, start, end)
// * start - default to 0
// * end - default to buff.length
Buffer.prototype.toString = function(encoding, start, end) {
  if (typeof encoding === 'string') {
    encoding = getEncodingType(encoding);
  } else {
    encoding = -1;
  }

  start = start === undefined ? 0 : ~~start;
  end = end === undefined ? this.length : ~~end;

  return native.toString(this, encoding, start, end);
};


// buff.writeUInt8(value, offset[,noAssert])
// [1] buff.writeUInt8(value, offset)
// [2] buff.writeUInt8(value, offset, noAssert)
Buffer.prototype.writeUInt8 = function(value, offset, noAssert) {
  value = +value;
  offset = offset >>> 0;
  if (!noAssert)
    checkInt(this, value, offset, 1, 0xff, 0);
  native.writeUInt8(this, value & 0xff, offset);
  return offset + 1;
};


// buff.writeUInt16LE(value, offset[,noAssert])
// [1] buff.writeUInt16LE(value, offset)
// [2] buff.writeUInt16LE(value, offset, noAssert)
Buffer.prototype.writeUInt16LE = function(value, offset, noAssert) {
  value = +value;
  offset = offset >>> 0;
  if (!noAssert)
    checkInt(this, value, offset, 2, 0xffff, 0);
  native.writeUInt8(this, value & 0xff, offset);
  native.writeUInt8(this, (value >>> 8) & 0xff, offset + 1);
  return offset + 2;
};


// buff.writeUInt32LE(value, offset[,noAssert])
// [1] buff.writeUInt32LE(value, offset)
// [2] buff.writeUInt32LE(value, offset, noAssert)
Buffer.prototype.writeUInt32LE = function(value, offset, noAssert) {
  value = +value;
  offset = offset >>> 0;
  if (!noAssert)
    checkInt(this, value, offset, 4, -1 >>> 0, 0);
  native.writeUInt8(this, (value >>> 24) & 0xff, offset + 3);
  native.writeUInt8(this, (value >>> 16) & 0xff, offset + 2);
  native.writeUInt8(this, (value >>> 8) & 0xff, offset + 1);
  native.writeUInt8(this, value & 0xff, offset);
  return offset + 4;
};


// buff.readUInt8(offset[,noAssert])
// [1] buff.readUInt8(offset)
// [2] buff.readUInt8(offset, noAssert)
Buffer.prototype.readUInt8 = function(offset, noAssert) {
  offset = offset >>> 0;
  if (!noAssert)
    checkOffset(offset, 1, this.length);
  return native.readUInt8(this, offset);
};


// buff.readInt8(offset[,noAssert])
// [1] buff.readInt8(offset)
// [2] buff.readInt8(offset, noAssert)
Buffer.prototype.readInt8 = function(offset, noAssert) {
  offset = offset >>> 0;
  if (!noAssert)
    checkOffset(offset, 1, this.length);
  var val = native.readUInt8(this, offset);
  return !(val & 0x80) ? val : (0xff - val + 1) * -1;
};


// buff.readUInt16LE(offset[,noAssert])
// [1] buff.readUInt16LE(offset)
// [2] buff.readUInt16LE(offset, noAssert)
Buffer.prototype.readUInt16LE = function(offset, noAssert) {
  offset = offset >>> 0;
  if (!noAssert)
    checkOffset(offset, 2, this.length);
  return native.readUInt8(this, offset) |
         (native.readUInt8(this, offset + 1) << 8);
};


// buff.fill(value)
Buffer.prototype.fill = function(value) {
  if (typeof value === 'number') {
    value = value & 255;
    for (var i = 0; i < this.length; i++) {
      native.writeUInt8(this, value, i);
    }
  }
  return this;
};


// Method: Buffer.from()
// Buffer.from(Array)
// Buffer.from(string,encoding)
// Buffer.from(Buffer)
// Buffer.from(ArrayBuffer)
function from(value, encoding, length) {

  var arrayBuffer = native.fromArrayBuffer(value, encoding, length);

  if (arrayBuffer) {
    return arrayBuffer;
  }
  if (Buffer.isBuffer(value) || (typeof value) === 'string'
      || Array.isArray(value)) {
    return new Buffer(value, encoding);
  }
  throw new TypeError('First argument must be' +
  'a string, Buffer, ArrayBuffer, Array, or array-like object');
}


/* Register the Buffer object back to the native C
 * so the other side can get the prototype in a consistent
 * and safe manner.
 */
native.Buffer = Buffer;

module.exports = Buffer;
module.exports.Buffer = Buffer;
module.exports.from = from;
