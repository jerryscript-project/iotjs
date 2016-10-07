/* Copyright 2016 Samsung Electronics Co., Ltd.
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

var EventEmiter = require('events').EventEmitter;
var util = require('util');
var assert  = require('assert');
var i2c = process.binding(process.binding.i2c);

function I2C(address, options) {
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
        return _this.setAddress(_this.address);
      }
    };
  })(this));
}

util.inherits(I2C, EventEmiter);

I2C.prototype.history = [];

I2C.prototype.scan = function(callback) {
  return i2c.scan(function(err, data) {
    return process.nextTick(function() {
      var result = [];
      for(var i = 0; i < data.length; i++) {
        if(data[i] >= 0) result.push(data[i]);
      }
      return callback(err, result);
    });
  });
};

I2C.prototype.setAddress = function(address) {
  i2c.setAddress(address);
  return this.address = address;
};

I2C.prototype.open = function(device, callback) {
  return i2c.open(device, function(err) {
    return process.nextTick(function() {
      return callback(err);
    });
  });
};

I2C.prototype.close = function() {
  return i2c.close();
};

I2C.prototype.write = function(buf, callback) {
  this.setAddress(this.address);
  if (!Buffer.isBuffer(buf)) {
    buf = new Buffer(buf);
  }
  return i2c.write(buf, function(err) {
    return process.nextTick(function() {
      return callback(err);
    });
  });
};

I2C.prototype.writeByte = function(byte, callback) {
  this.setAddress(this.address);
  return i2c.writeByte(byte, function(err) {
    return process.nextTick(function() {
      return callback(err);
    });
  });
};

I2C.prototype.writeBytes = function(cmd, buf, callback) {
  this.setAddress(this.address);
  if (!Buffer.isBuffer(buf)) {
    buf = new Buffer(buf);
  }
  return i2c.writeBlock(cmd, buf, function(err) {
    return process.nextTick(function() {
      return callback(err);
    });
  });
};

I2C.prototype.read = function(len, callback) {
  this.setAddress(this.address);
  return i2c.read(len, function(err, data) {
    return process.nextTick(function() {
      return callback(err, data);
    });
  });
};

I2C.prototype.readByte = function(callback) {
  this.setAddress(this.address);
  return i2c.readByte(function(err, data) {
    return process.nextTick(function() {
      return callback(err, data);
    });
  });
};

I2C.prototype.readBytes = function(cmd, len, callback) {
  this.setAddress(this.address);
  return i2c.readBlock(cmd, len, null, function(err, actualBuffer) {
    return process.nextTick(function() {
      return callback(err, actualBuffer);
    });
  });
};

I2C.prototype.stream = function(cmd, len, delay) {
  if (util.isNull(delay)) {
    delay = 100;
  }
  this.setAddress(this.address);
  return i2c.readBlock(cmd, len, delay, (function(_this) {
    return function(err, data) {
      if (err) {
        return _this.emit('error', err);
      } else {
        return _this.emit('data', {
          address: _this.address,
          data: data,
          cmd: cmd,
          length: len,
          timestamp: Date.now()
        });
      }
    };
  })(this));
};

module.exports = I2C;
