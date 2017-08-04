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

/* Copyright (C) 2015 Sandeep Mistry sandeep.mistry@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

var debug = console.log; // require('debug')('ble');

var events = require('events');
var util = require('util');

var uuidUtil = require('ble_uuid_util');

var PrimaryService = require('ble_primary_service');
var Characteristic = require('ble_characteristic');
var Descriptor = require('ble_descriptor');

var bindings = null;

var platform = process.platform;

if (platform === 'darwin') {
  // bindings = require('./mac/bindings');
} else if (platform === 'linux' || platform === 'win32' || platform === 'android') {
  bindings = require('ble_hci_socket_bindings');
} else {
  throw new Error('Unsupported platform');
}

function Bleno() {
  this.platform = 'unknown';
  this.state = 'unknown';
  this.address = 'unknown';
  this.rssi = 0;
  this.mtu = 20;

  this._bindings = bindings;

  this._bindings.on('stateChange', this.onStateChange.bind(this));
  this._bindings.on('platform', this.onPlatform.bind(this));
  this._bindings.on('addressChange', this.onAddressChange.bind(this));
  this._bindings.on('advertisingStart', this.onAdvertisingStart.bind(this));
  this._bindings.on('advertisingStop', this.onAdvertisingStop.bind(this));
  this._bindings.on('servicesSet', this.onServicesSet.bind(this));
  this._bindings.on('accept', this.onAccept.bind(this));
  this._bindings.on('mtuChange', this.onMtuChange.bind(this));
  this._bindings.on('disconnect', this.onDisconnect.bind(this));

  this._bindings.on('rssiUpdate', this.onRssiUpdate.bind(this));

  this._bindings.init();
}

util.inherits(Bleno, events.EventEmitter);

Bleno.prototype.PrimaryService = PrimaryService;
Bleno.prototype.Characteristic = Characteristic;
Bleno.prototype.Descriptor = Descriptor;

Bleno.prototype.onPlatform = function(platform) {
  debug('platform ' + platform);

  this.platform = platform;
};

Bleno.prototype.onStateChange = function(state) {
  debug('stateChange ' + state);

  this.state = state;

  this.emit('stateChange', state);
};

Bleno.prototype.onAddressChange = function(address) {
  debug('addressChange ' + address);

  this.address = address;
};

Bleno.prototype.onAccept = function(clientAddress) {
  debug('accept ' + clientAddress);
  this.emit('accept', clientAddress);
};

Bleno.prototype.onMtuChange = function(mtu) {
  debug('mtu ' + mtu);

  this.mtu = mtu;

  this.emit('mtuChange', mtu);
};

Bleno.prototype.onDisconnect = function(clientAddress) {
  debug('disconnect' + clientAddress);
  this.emit('disconnect', clientAddress);
};

Bleno.prototype.startAdvertising = function(name, serviceUuids, callback) {
  if (this.state !== 'poweredOn') {
    var error = new Error('Could not start advertising, state is ' + this.state + ' (not poweredOn)');

    if (typeof callback === 'function') {
      callback(error);
    } else {
      throw error;
    }
  } else {
    if (callback) {
      this.once('advertisingStart', callback);
    }

    var undashedServiceUuids = [];

    if (serviceUuids && serviceUuids.length) {
      for (var i = 0; i < serviceUuids.length; i++) {
        undashedServiceUuids[i] = uuidUtil.removeDashes(serviceUuids[i]);
      }
    }

    this._bindings.startAdvertising(name, undashedServiceUuids);
  }
};

Bleno.prototype.startAdvertisingIBeacon = function(uuid, major, minor, measuredPower, callback) {
  if (this.state !== 'poweredOn') {
    var error = new Error('Could not start advertising, state is ' + this.state + ' (not poweredOn)');

    if (typeof callback === 'function') {
      callback(error);
    } else {
      throw error;
    }
  } else {
    var undashedUuid =  uuidUtil.removeDashes(uuid);
    var uuidData = new Buffer(undashedUuid, 'hex');
    var uuidDataLength = uuidData.length;
    var iBeaconData = new Buffer(uuidData.length + 5);

    for (var i = 0; i < uuidDataLength; i++) {
      iBeaconData[i] = uuidData[i];
    }

    iBeaconData.writeUInt16BE(major, uuidDataLength);
    iBeaconData.writeUInt16BE(minor, uuidDataLength + 2);
    iBeaconData.writeInt8(measuredPower, uuidDataLength + 4);

    if (callback) {
      this.once('advertisingStart', callback);
    }

    debug('iBeacon data = ' + iBeaconData.toString('hex'));

    this._bindings.startAdvertisingIBeacon(iBeaconData);
  }
};

Bleno.prototype.onAdvertisingStart = function(error) {
  debug('advertisingStart: ' + error);

  if (error) {
    this.emit('advertisingStartError', error);
  }

  this.emit('advertisingStart', error);
};

Bleno.prototype.startAdvertisingWithEIRData = function(advertisementData, scanData, callback) {
  if (typeof scanData === 'function') {
    callback = scanData;
    scanData = null;
  }

  if (this.state !== 'poweredOn') {
    var error = new Error('Could not advertising scanning, state is ' + this.state + ' (not poweredOn)');

    if (typeof callback === 'function') {
      callback(error);
    } else {
      throw error;
    }
  } else {
    if (callback) {
      this.once('advertisingStart', callback);
    }

    this._bindings.startAdvertisingWithEIRData(advertisementData, scanData);
  }
};

Bleno.prototype.stopAdvertising = function(callback) {
  if (callback) {
    this.once('advertisingStop', callback);
  }
  this._bindings.stopAdvertising();
};

Bleno.prototype.onAdvertisingStop = function() {
  debug('advertisingStop');
  this.emit('advertisingStop');
};

Bleno.prototype.setServices = function(services, callback) {
  if (callback) {
    this.once('servicesSet', callback);
  }
  this._bindings.setServices(services);
};

Bleno.prototype.onServicesSet = function(error) {
  debug('servicesSet');

  if (error) {
    this.emit('servicesSetError', error);
  }

  this.emit('servicesSet', error);
};

Bleno.prototype.disconnect = function() {
  debug('disconnect');
  this._bindings.disconnect();
};

Bleno.prototype.updateRssi = function(callback) {
  if (callback) {
    this.once('rssiUpdate', function(rssi) {
      callback(null, rssi);
    });
  }

  this._bindings.updateRssi();
};

Bleno.prototype.onRssiUpdate = function(rssi) {
  this.emit('rssiUpdate', rssi);
};

module.exports = new Bleno();
