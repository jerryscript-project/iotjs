/* Copyright (c) 2013, Kelly Korevec <korevec@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the author nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Copyright 2016-present Samsung Electronics Co., Ltd. and other contributors
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

/* This file includes all APIs in 'node-i2c'(https://github.com/kelly/node-i2c).
 * Some functions are translated from coffee script(i2c.coffee) in 'node-i2c'.
 */

var EventEmiter = require('events').EventEmitter;
var util = require('util');
var assert  = require('assert');
var i2c = process.binding(process.binding.i2c);

function I2C(address, options, callback) {
  if (!(this instanceof I2C)) {
    return new I2C(address, options);
  }

  if (util.isUndefined(address)) {
    address = 0x23; // Default device I2C address
  }

  if (util.isUndefined(options)) {
    options = {};
  }

  assert(util.isObject(options));

  if (util.isUndefined(options.device)){
    options.device = '/dev/i2c-1';
  }

  this._i2c = null;
  this.address = address;
  this.options = options;

  EventEmiter.call(this);

  process.on('exit', (function(_this) {
    return function() {
      return _this.close();
    };
  })(this));

  this.on('data', (function(_this) {
    return function(data) {
      return _this.history.push(data);
    };
  })(this));

  this.open(this.options.device, (function(_this) {
    return function(err) {
      if(!err) {
        return _this.setAddress(_this.address, callback);
      }
      else {
        throw err;
      }
    };
  })(this));
}

util.inherits(I2C, EventEmiter);

I2C.prototype.history = [];

I2C.prototype.scan = function(callback) {
  return this._i2c.scan(function(err, data) {
    return process.nextTick(function() {
      var result = [];
      for(var i = 0; i < data.length; i++) {
        if(data[i] == 1) result.push(i);
      }
      return callback(err, result);
    });
  });
};

I2C.prototype.setAddress = function(address, callback) {
  this._i2c.setAddress(address);

  callback && callback();

  return this.address = address;
};

I2C.prototype.open = function(device, callback) {
  if (util.isNull(this._i2c)) {
    this._i2c = new i2c(device, function(err) {
      return process.nextTick(function() {
        return callback(err);
      });
    });
  }
};

I2C.prototype.close = function() {
  return this._i2c.close();
};

I2C.prototype.write = function(array, callback) {
  this.setAddress(this.address);
  return this._i2c.write(array, function(err) {
    return process.nextTick(function() {
      return callback(err);
    });
  });
};

I2C.prototype.writeByte = function(byte, callback) {
  this.setAddress(this.address);
  return this._i2c.writeByte(byte, function(err) {
    return process.nextTick(function() {
      return callback(err);
    });
  });
};

I2C.prototype.writeBytes = function(cmd, array, callback) {
  this.setAddress(this.address);
  return this._i2c.writeBlock(cmd, array, function(err) {
    return process.nextTick(function() {
      return callback(err);
    });
  });
};

I2C.prototype.read = function(len, callback) {
  this.setAddress(this.address);
  return this._i2c.read(len, function(err, data) {
    return process.nextTick(function() {
      return callback(err, data);
    });
  });
};

I2C.prototype.readByte = function(callback) {
  this.setAddress(this.address);
  return this._i2c.readByte(function(err, data) {
    return process.nextTick(function() {
      return callback(err, data);
    });
  });
};

I2C.prototype.readBytes = function(cmd, len, callback) {
  this.setAddress(this.address);
  return this._i2c.readBlock(cmd, len, 0, function(err, resArray) {
    return process.nextTick(function() {
      return callback(err, resArray);
    });
  });
};

I2C.prototype.stream = function(cmd, len, delay) {
  if (util.isNull(delay)) {
    delay = 100;
  }
  this.setAddress(this.address);
  this._i2c.readBlock(cmd, len, delay, (function(_this) {
    return function(err, data) {
      if (err) {
        _this.emit('error', err);
      } else {
        _this.emit('data', {
          address: _this.address,
          data: data,
          cmd: cmd,
          length: len,
          timestamp: Date.now()
        });
        _this.stream(cmd, len, delay);
      }
    };
  })(this));
};

module.exports = I2C;
