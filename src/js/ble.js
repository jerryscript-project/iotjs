/* The MIT License (MIT)
 *
 * Copyright (c) 2013 Sandeep Mistry
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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

/* This file include some APIs in 'bleno'.
 * (https://github.com/sandeepmistry/bleno)
 */

var EventEmiter = require('events').EventEmitter;
var util = require('util');

var ble = process.binding(process.binding.ble);

// TODO: Implement PrimaryService
/* function PrimaryService(options) {
  this.uuid = options.uuid;
  this.characteristics = options.characteristics || [];
} */

function BLE() {
  this.state = 'unknown';
  // this.address = 'unknown';
  this._advertiseState = null;
  // this.rssi = 0;
  // this.mtu = 20;

  EventEmiter.call(this);

  this.on('stateChange', (function(_this) {
    return function(state) {
      _this.state = state;

      if (state == 'unauthorized') {
        console.log('ble warning: adapter state unauthorized.');
        console.log('             please run as root or with sudo.');
      }
    };
  })(this));

  // TODO: implement more events.
  // ...

  ble.init((function(_this) {
    return function(state) {
      _this.emit('stateChange', state);
    };
  })(this));

  process.on('exit', (function(_this) {
    return function() {
      if (_this._advertiseState != 'stoped') {
        _this.stopAdvertising();
      }
    };
  })(this));

}

util.inherits(BLE, EventEmiter);

BLE.prototype._runBleLoop = function() {
  if (this._advertiseState == 'started') {
    ble.runBleLoop((function(_this) {
      return function(curState) {
        if (curState != _this.state) {
          _this.emit('stateChange', curState);
        }
        setTimeout(_this._runBleLoop.bind(_this), 1000);
      };
    })(this));
  }
};

BLE.prototype.startAdvertising = function(name, serviceUuids, callback) {
  var advertisementDataLength = 3;
  var scanDataLength = 0;

  var serviceUuids16bit = [];
  var serviceUuids128bit = [];
  var i = 0;
  var j = 0;
  var k = 0;

  if (name && name.length) {
    scanDataLength += 2 + name.length;
  }

  if (serviceUuids && serviceUuids.length) {
    for (i = 0; i < serviceUuids.length; i++) {
      var convertedUuid = serviceUuids[i].match(/.{1,2}/g).reverse().join('');
      var serviceUuid = [];
      while (convertedUuid.length >= 2) {
        serviceUuid.push(parseInt(convertedUuid.substring(0, 2), 16));
        convertedUuid = convertedUuid.substring(2, convertedUuid.length);
      }

      if (serviceUuid.length === 2) {
        serviceUuids16bit.push(serviceUuid);
      } else if (serviceUuid.length === 16) {
        serviceUuids128bit.push(serviceUuid);
      }
    }
  }

  if (serviceUuids16bit.length) {
    advertisementDataLength += 2 + 2 * serviceUuids16bit.length;
  }

  if (serviceUuids128bit.length) {
    advertisementDataLength += 2 + 16 * serviceUuids128bit.length;
  }

  i = 0;
  var advertisementData = [];

  // flags
  advertisementData[i++] = 2;
  advertisementData[i++] = 0x01;
  advertisementData[i++] = 0x06;

  if (serviceUuids16bit.length) {
    advertisementData[i++] = 1 + 2 * serviceUuids16bit.length;
    advertisementData[i++] = 0x03;
    for (j = 0; j < serviceUuids16bit.length; j++) {
      for (k = 0; k < serviceUuids16bit[j].length; k++) {
        advertisementData[i++] = serviceUuids16bit[j][k];
      }
    }
  }

  if (serviceUuids128bit.length) {
    advertisementData[i++] = 1 + 16 * serviceUuids128bit.length;
    advertisementData[i++] = 0x06;
    for (j = 0; j < serviceUuids128bit.length; j++) {
      for (k = 0; k < serviceUuids128bit[j].length; k++) {
        advertisementData[i++] = serviceUuids128bit[j][k];
      }
    }
  }

  i = 0;
  var scanData = [];

  // name
  if (name && name.length) {
    scanData[i++] = name.length + 1;
    scanData[i++] = 0x08;
    for (j = 0; j < name.length; j++) {
      scanData[i++] = name[j].charCodeAt(0);
    }
  }

  this._advertiseState = 'started';

  ble.startAdvertising(advertisementData, scanData, function(err) {
    return process.nextTick(function() {
      return callback(err);
    });
  });

  setTimeout(this._runBleLoop.bind(this), 1000);
};

BLE.prototype.stopAdvertising = function(callback) {
  this._advertiseState = 'stoped';

  ble.stopAdvertising(function(err) {
    return process.nextTick(function() {
      return callback(err);
    });
  });
};

// TODO: Impelemnt setServices function.
BLE.prototype.setServices = function(services, callback) {
  ble.setServices(services, function(err) {
    return process.nextTick(function() {
      return callback(err);
    });
  });
};

// TODO: Implement these constructors.
// BLE.prototype.PrimaryService = PrimaryService;
// BLE.prototype.Characteristic = characteristic;
// BLE.prototype.Descriptor = descriptor;

module.exports = new BLE();
