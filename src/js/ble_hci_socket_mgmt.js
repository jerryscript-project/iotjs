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

var debug = console.log; // require('debug')('ble_mgmt');

var events = require('events');
var util = require('util');

var BluetoothHciSocket = require('ble_hci_socket');

var LTK_INFO_SIZE = 36;

var MGMT_OP_LOAD_LONG_TERM_KEYS = 0x0013;

function Mgmt() {
  this._socket = new BluetoothHciSocket();
  this._ltkInfos = [];

  this._socket.on('data', this.onSocketData.bind(this));
  this._socket.on('error', this.onSocketError.bind(this));

  this._socket.bindControl();
  this._socket.start();
}

Mgmt.prototype.onSocketData = function(data) {
  debug('on data ->' + data.toString('hex'));
};

Mgmt.prototype.onSocketError = function(error) {
  debug('on error ->' + error.message);
};

Mgmt.prototype.addLongTermKey = function(address, addressType, authenticated, master, ediv, rand, key) {
  var ltkInfo = new Buffer(LTK_INFO_SIZE);

  address.copy(ltkInfo, 0);
  ltkInfo.writeUInt8(addressType.readUInt8(0) + 1, 6); // BDADDR_LE_PUBLIC = 0x01, BDADDR_LE_RANDOM 0x02, so add one

  ltkInfo.writeUInt8(authenticated, 7);
  ltkInfo.writeUInt8(master, 8);
  ltkInfo.writeUInt8(key.length, 9);

  ediv.copy(ltkInfo, 10);
  rand.copy(ltkInfo, 12);
  key.copy(ltkInfo, 20);

  this._ltkInfos.push(ltkInfo);

  this.loadLongTermKeys();
};

Mgmt.prototype.clearLongTermKeys = function() {
  this._ltkInfos = [];

  this.loadLongTermKeys();
};

Mgmt.prototype.loadLongTermKeys = function() {
  var numLongTermKeys = this._ltkInfos.length;
  var op = new Buffer(2 + numLongTermKeys * LTK_INFO_SIZE);

  op.writeUInt16LE(numLongTermKeys, 0);

  for (var i = 0; i < numLongTermKeys; i++) {
    this._ltkInfos[i].copy(op, 2 + i * LTK_INFO_SIZE);
  }

  this.write(MGMT_OP_LOAD_LONG_TERM_KEYS, 0, op);
};

Mgmt.prototype.write = function(opcode, index, data) {
  var length = 0;

  if (data) {
    length = data.length;
  }

  var pkt = new Buffer(6 + length);

  pkt.writeUInt16LE(opcode, 0);
  pkt.writeUInt16LE(index, 2);
  pkt.writeUInt16LE(length, 4);

  if (length) {
    data.copy(pkt, 6);
  }

  debug('writing -> ' + pkt.toString('hex'));
  this._socket.write(pkt);
};

module.exports = new Mgmt();
