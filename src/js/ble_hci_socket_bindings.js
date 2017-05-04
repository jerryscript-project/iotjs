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

var debug = console.log; // require('debug')('ble_hci-socket_bindings');

var events = require('events');
var util = require('util');

var AclStream = require('ble_hci_socket_acl_stream');
var Hci = require('ble_hci_socket_hci');
var Gap = require('ble_hci_socket_gap');
var Gatt = require('ble_hci_socket_gatt');

var BlenoBindings = function() {
  this._state = null;

  this._advertising = false;

  this._hci = new Hci();
  this._gap = new Gap(this._hci);
  this._gatt = new Gatt(this._hci);

  this._address = null;
  this._handle = null;
  this._aclStream = null;
};

util.inherits(BlenoBindings, events.EventEmitter);

BlenoBindings.prototype.startAdvertising = function(name, serviceUuids) {
  this._advertising = true;

  this._gap.startAdvertising(name, serviceUuids);
};

BlenoBindings.prototype.startAdvertisingIBeacon = function(data) {
  this._advertising = true;

  this._gap.startAdvertisingIBeacon(data);
};

BlenoBindings.prototype.startAdvertisingWithEIRData = function(advertisementData, scanData) {
  this._advertising = true;

  this._gap.startAdvertisingWithEIRData(advertisementData, scanData);
};

BlenoBindings.prototype.stopAdvertising = function() {
  this._advertising = false;

  this._gap.stopAdvertising();
};

BlenoBindings.prototype.setServices = function(services) {
  this._gatt.setServices(services);

  this.emit('servicesSet');
};

BlenoBindings.prototype.disconnect = function() {
  if (this._handle) {
    debug('disconnect by server');

    this._hci.disconnect(this._handle);
  }
};

BlenoBindings.prototype.updateRssi = function() {
  if (this._handle) {
    this._hci.readRssi(this._handle);
  }
};

BlenoBindings.prototype.init = function() {
  this.onSigIntBinded = this.onSigInt.bind(this);

  process.on('SIGINT', this.onSigIntBinded);
  process.on('exit', this.onExit.bind(this));

  this._gap.on('advertisingStart', this.onAdvertisingStart.bind(this));
  this._gap.on('advertisingStop', this.onAdvertisingStop.bind(this));

  this._gatt.on('mtuChange', this.onMtuChange.bind(this));

  this._hci.on('stateChange', this.onStateChange.bind(this));
  this._hci.on('addressChange', this.onAddressChange.bind(this));
  this._hci.on('readLocalVersion', this.onReadLocalVersion.bind(this));

  this._hci.on('leConnComplete', this.onLeConnComplete.bind(this));
  this._hci.on('leConnUpdateComplete', this.onLeConnUpdateComplete.bind(this));
  this._hci.on('rssiRead', this.onRssiRead.bind(this));
  this._hci.on('disconnComplete', this.onDisconnComplete.bind(this));
  this._hci.on('encryptChange', this.onEncryptChange.bind(this));
  this._hci.on('leLtkNegReply', this.onLeLtkNegReply.bind(this));
  this._hci.on('aclDataPkt', this.onAclDataPkt.bind(this));

  this.emit('platform', process.platform);

  this._hci.init();
};

BlenoBindings.prototype.onStateChange = function(state) {
  if (this._state === state) {
    return;
  }
  this._state = state;

  if (state === 'unauthorized') {
    console.log('bleno warning: adapter state unauthorized, please run as root or with sudo');
    console.log('               or see README for information on running without root/sudo:');
    console.log('               https://github.com/sandeepmistry/bleno#running-on-linux');
  } else if (state === 'unsupported') {
    console.log('bleno warning: adapter does not support Bluetooth Low Energy (BLE, Bluetooth Smart).');
    console.log('               Try to run with environment variable:');
    console.log('               [sudo] BLENO_HCI_DEVICE_ID=x node ...');
  }

  this.emit('stateChange', state);
};

BlenoBindings.prototype.onAddressChange = function(address) {
  this.emit('addressChange', address);
};

BlenoBindings.prototype.onReadLocalVersion = function(hciVer, hciRev, lmpVer, manufacturer, lmpSubVer) {
  if (manufacturer === 93) {
    // Realtek Semiconductor Corporation
    this._gatt.maxMtu = 23;
  }
};

BlenoBindings.prototype.onAdvertisingStart = function(error) {
  this.emit('advertisingStart', error);
};

BlenoBindings.prototype.onAdvertisingStop = function() {
  this.emit('advertisingStop');
};

BlenoBindings.prototype.onLeConnComplete = function(status, handle, role, addressType, address, interval, latency, supervisionTimeout, masterClockAccuracy) {
  if (role !== 1) {
    // not slave, ignore
    return;
  }

  this._address = address;
  this._handle = handle;
  this._aclStream = new AclStream(this._hci, handle, this._hci.addressType, this._hci.address, addressType, address);
  this._gatt.setAclStream(this._aclStream);

  this.emit('accept', address);
};

BlenoBindings.prototype.onLeConnUpdateComplete = function(handle, interval, latency, supervisionTimeout) {
  // no-op
};

BlenoBindings.prototype.onDisconnComplete = function(handle, reason) {
  if (this._aclStream) {
    this._aclStream.push(null, null);
  }

  var address = this._address;

  this._address = null;
  this._handle = null;
  this._aclStream = null;

  if (address) {
    this.emit('disconnect', address); // TODO: use reason
  }

  if (this._advertising) {
    this._gap.restartAdvertising();
  }
};

BlenoBindings.prototype.onEncryptChange = function(handle, encrypt) {
  if (this._handle === handle && this._aclStream) {
    this._aclStream.pushEncrypt(encrypt);
  }
};

BlenoBindings.prototype.onLeLtkNegReply = function(handle) {
  if (this._handle === handle && this._aclStream) {
    this._aclStream.pushLtkNegReply();
  }
};

BlenoBindings.prototype.onMtuChange = function(mtu) {
  this.emit('mtuChange', mtu);
};

BlenoBindings.prototype.onRssiRead = function(handle, rssi) {
  this.emit('rssiUpdate', rssi);
};

BlenoBindings.prototype.onAclDataPkt = function(handle, cid, data) {
  if (this._handle === handle && this._aclStream) {
    this._aclStream.push(cid, data);
  }
};

BlenoBindings.prototype.onSigInt = function() {
  var sigIntListeners = process.listeners('SIGINT');

  if (sigIntListeners[sigIntListeners.length - 1] === this.onSigIntBinded) {
    // we are the last listener, so exit
    // this will trigger onExit, and clean up
    process.exit(1);
  }
};

BlenoBindings.prototype.onExit = function() {
  this._gap.stopAdvertising();

  this.disconnect();
};

module.exports = new BlenoBindings();
