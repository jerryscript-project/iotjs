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


var bufferBuiltin = process.binding(process.binding.buffer);
var util = require('util');


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


// Buffer constructor
// [1] new Buffer(size)
// [2] new Buffer(buffer)
// [3] new Buffer(string)
// [4] new Buffer(string, encoding)
// [5] new Buffer(array)
function Buffer(subject, encoding) {
  if (!util.isBuffer(this)) {
    return new Buffer(subject, encoding);
  }

  if (util.isNumber(subject)) {
    this.length = subject > 0 ? subject >>> 0 : 0;
  } else if (util.isString(subject)) {
    this.length = Buffer.byteLength(subject, encoding);
  } else if (util.isBuffer(subject) || util.isArray(subject)) {
    this.length = subject.length;
  } else {
    throw new TypeError('Bad arguments: Buffer(string|number|Buffer|Array)');
  }

  this._builtin = new bufferBuiltin(this, this.length);

  if (util.isString(subject)) {
    if (encoding !== undefined && util.isString(encoding)) {
      switch (encoding) {
        case 'hex':
          if (this._builtin.hexWrite(subject, 0, this.length) != this.length) {
            throw new TypeError('Invalid hex string');
          }
          break;
        default:
          this.write(subject);
      }
    } else {
      this.write(subject);
    }
  } else if (util.isBuffer(subject)) {
    subject.copy(this);
  } else if (util.isArray(subject)) {
    for (var i = 0; i < this.length; ++i) {
      this._builtin.writeUInt8(subject[i], i);
    }
  }
}


// Buffer.byteLength(string)
Buffer.byteLength = function(str, encoding) {
  var len = bufferBuiltin.byteLength(str);

  if (encoding !== undefined && util.isString(encoding)) {
    switch (encoding) {
      case 'hex':
        return len >>> 1;
    }
  }
  return len;
};


// Buffer.concat(list)
Buffer.concat = function(list) {
  if (!util.isArray(list)) {
    throw new TypeError('Bad arguments: Buffer.concat([Buffer])');
  }

  var length = 0;
  for (var i = 0; i < list.length; ++i) {
    if (!util.isBuffer(list[i])) {
      throw new TypeError('Bad arguments: Buffer.concat([Buffer])');
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


// Buffer.isBuffer(object)
Buffer.isBuffer = function(object) {
  return util.isBuffer(object);
};


// buffer.equals(otherBuffer)
Buffer.prototype.equals = function(otherBuffer) {
  if (!util.isBuffer(otherBuffer)) {
    throw new TypeError('Bad arguments: buffer.equals(Buffer)');
  }

  return this._builtin.compare(otherBuffer._builtin) == 0;
};


// buffer.compare(otherBuffer)
Buffer.prototype.compare = function(otherBuffer) {
  if (!util.isBuffer(otherBuffer)) {
    throw new TypeError('Bad arguments: buffer.compare(Buffer)');
  }

  return this._builtin.compare(otherBuffer._builtin);
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
  if (!util.isBuffer(target)) {
    throw new TypeError('Bad arguments: buff.copy(Buffer)');
  }

  targetStart = targetStart === undefined ? 0 : ~~targetStart;
  sourceStart = sourceStart === undefined ? 0 : ~~sourceStart;
  sourceEnd = sourceEnd === undefined ? this.length : ~~ sourceEnd;

  if ((sourceEnd > sourceStart) && (targetStart < 0)) {
    throw new RangeError('Attempt to write outside buffer bounds');
  }

  return this._builtin.copy(target, targetStart, sourceStart, sourceEnd);
};


// buffer.write(string[, offset[, length]])
// [1] buffer.write(string)
// [2] buffer.write(string, offset)
// [3] buffer.write(string, offset, length)
// * offset - default to 0
// * length - default to buffer.length - offset
Buffer.prototype.write = function(string, offset, length) {
  if (!util.isString(string)) {
    throw new TypeError('Bad arguments: buff.write(string)');
  }

  offset = offset === undefined ? 0 : ~~offset;
  if (string.length > 0 && (offset < 0 || offset >= this.length)) {
    throw new RangeError('Attempt to write outside buffer bounds');
  }

  var remaining = this.length - offset;
  length = length === undefined ? remaining : ~~length;

  return this._builtin.write(string, offset, length);
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

  return this._builtin.slice(start, end);
};


// buff.toString([encoding,[,start[, end]]])
// [1] buff.toString()
// [2] buff.toString(start)
// [3] buff.toString(start, end)
// [4] buff.toString('hex')
// * start - default to 0
// * end - default to buff.length
Buffer.prototype.toString = function(start, end) {
  if (util.isString(start) && start === "hex" && end === undefined) {
      return this._builtin.toHexString();
  }
  start = start === undefined ? 0 : ~~start;
  end = end === undefined ? this.length : ~~end;

  return this._builtin.toString(start, end);
};


// buff.writeUInt8(value, offset[,noAssert])
// [1] buff.writeUInt8(value, offset)
// [2] buff.writeUInt8(value, offset, noAssert)
Buffer.prototype.writeUInt8 = function(value, offset, noAssert) {
  value = +value;
  offset = offset >>> 0;
  if (!noAssert)
    checkInt(this, value, offset, 1, 0xff, 0);
  this._builtin.writeUInt8(value & 0xff, offset);
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
  this._builtin.writeUInt8(value & 0xff, offset);
  this._builtin.writeUInt8((value >>> 8) & 0xff, offset + 1);
  return offset + 2;
};


// buff.writeUInt32LE(value, offset[,noAssert])
// [1] buff.writeUInt32LE(value, offset)
// [2] buff.writeUInt32LE(value, offset, noAssert)
Buffer.prototype.writeUInt32LE = function(value, offset, noAssert) {
  value = +value;
  offset = offset >>> 0;
  if (!noAssert)
    checkInt(this, value, offset, 4, 0xffffffff, 0);
  this._builtin.writeUInt8((value >>> 24) & 0xff, offset + 3);
  this._builtin.writeUInt8((value >>> 16) & 0xff, offset + 2);
  this._builtin.writeUInt8((value >>> 8) & 0xff, offset + 1);
  this._builtin.writeUInt8(value & 0xff, offset);
  return offset + 4;
};


// buff.readUInt8(offset[,noAssert])
// [1] buff.readUInt8(offset)
// [2] buff.readUInt8(offset, noAssert)
Buffer.prototype.readUInt8 = function(offset, noAssert) {
  offset = offset >>> 0;
  if (!noAssert)
    checkOffset(offset, 1, this.length);
  return this._builtin.readUInt8(offset);
};


// buff.readInt8(offset[,noAssert])
// [1] buff.readInt8(offset)
// [2] buff.readInt8(offset, noAssert)
Buffer.prototype.readInt8 = function(offset, noAssert) {
  offset = offset >>> 0;
  if (!noAssert)
    checkOffset(offset, 1, this.length);
  var val = this._builtin.readUInt8(offset);
  return !(val & 0x80) ? val : (0xff - val + 1) * -1;
};


// buff.readUInt16LE(offset[,noAssert])
// [1] buff.readUInt16LE(offset)
// [2] buff.readUInt16LE(offset, noAssert)
Buffer.prototype.readUInt16LE = function(offset, noAssert) {
  offset = offset >>> 0;
  if (!noAssert)
    checkOffset(offset, 2, this.length);
  return this._builtin.readUInt8(offset) |
         (this._builtin.readUInt8(offset + 1) << 8);
};


// buff.fill(value)
Buffer.prototype.fill = function(value) {
  if (util.isNumber(value)) {
    value = value & 255;
    for (var i = 0; i < this.length; i++) {
      this._builtin.writeUInt8(value, i);
    }
  }
  return this;
};


module.exports = Buffer;
module.exports.Buffer = Buffer;
