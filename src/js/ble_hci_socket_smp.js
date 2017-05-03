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

var debug = console.log; // require('debug')('ble_hci_socket_smp');

var events = require('events');
var util = require('util');

var crypto = require('ble_hci_socket_crypto');
var mgmt = require('ble_hci_socket_mgmt');

var SMP_CID = 0x0006;

var SMP_PAIRING_REQUEST = 0x01;
var SMP_PAIRING_RESPONSE = 0x02;
var SMP_PAIRING_CONFIRM = 0x03;
var SMP_PAIRING_RANDOM = 0x04;
var SMP_PAIRING_FAILED = 0x05;
var SMP_ENCRYPT_INFO = 0x06;
var SMP_MASTER_IDENT = 0x07;

var SMP_UNSPECIFIED = 0x08;

var Smp = function(aclStream, localAddressType, localAddress, remoteAddressType, remoteAddress) {
  this._aclStream = aclStream;

  this._iat = new Buffer([(remoteAddressType === 'random') ? 0x01 : 0x00]);
  this._ia = new Buffer(remoteAddress.split(':').reverse().join(''), 'hex');
  this._rat = new Buffer([(localAddressType === 'random') ? 0x01 : 0x00]);
  this._ra = new Buffer(localAddress.split(':').reverse().join(''), 'hex');

  this._stk = null;
  this._random = null;
  this._diversifier = null;

  this.onAclStreamDataBinded = this.onAclStreamData.bind(this);
  this.onAclStreamEncryptChangeBinded = this.onAclStreamEncryptChange.bind(this);
  this.onAclStreamLtkNegReplyBinded = this.onAclStreamLtkNegReply.bind(this);
  this.onAclStreamEndBinded = this.onAclStreamEnd.bind(this);

  this._aclStream.on('data', this.onAclStreamDataBinded);
  this._aclStream.on('encryptChange', this.onAclStreamEncryptChangeBinded);
  this._aclStream.on('ltkNegReply', this.onAclStreamLtkNegReplyBinded);
  this._aclStream.on('end', this.onAclStreamEndBinded);
};

util.inherits(Smp, events.EventEmitter);

Smp.prototype.onAclStreamData = function(cid, data) {
  if (cid !== SMP_CID) {
    return;
  }

  var code = data.readUInt8(0);

  if (SMP_PAIRING_REQUEST === code) {
    this.handlePairingRequest(data);
  } else if (SMP_PAIRING_CONFIRM === code) {
    this.handlePairingConfirm(data);
  } else if (SMP_PAIRING_RANDOM === code) {
    this.handlePairingRandom(data);
  } else if (SMP_PAIRING_FAILED === code) {
    this.handlePairingFailed(data);
  }
};

Smp.prototype.onAclStreamEncryptChange = function(encrypted) {
  if (encrypted) {
    if (this._stk && this._diversifier && this._random) {
      this.write(Buffer.concat([
        new Buffer([SMP_ENCRYPT_INFO]),
        this._stk
      ]));

      this.write(Buffer.concat([
        new Buffer([SMP_MASTER_IDENT]),
        this._diversifier,
        this._random
      ]));
    }
  }
};

Smp.prototype.onAclStreamLtkNegReply = function() {
    this.write(new Buffer([
      SMP_PAIRING_FAILED,
      SMP_UNSPECIFIED
    ]));

    this.emit('fail');
};

Smp.prototype.onAclStreamEnd = function() {
  this._aclStream.removeListener('data', this.onAclStreamDataBinded);
  this._aclStream.removeListener('encryptChange', this.onAclStreamEncryptChangeBinded);
  this._aclStream.removeListener('ltkNegReply', this.onAclStreamLtkNegReplyBinded);
  this._aclStream.removeListener('end', this.onAclStreamEndBinded);
};

Smp.prototype.handlePairingRequest = function(data) {
  this._preq = data;

  this._pres = new Buffer([
    SMP_PAIRING_RESPONSE,
    0x03, // IO capability: NoInputNoOutput
    0x00, // OOB data: Authentication data not present
    0x01, // Authentication requirement: Bonding - No MITM
    0x10, // Max encryption key size
    0x00, // Initiator key distribution: <none>
    0x01  // Responder key distribution: EncKey
  ]);

  this.write(this._pres);
};

Smp.prototype.handlePairingConfirm = function(data) {
  this._pcnf = data;

  this._tk = new Buffer('00000000000000000000000000000000', 'hex');
  this._r = crypto.r();

  this.write(Buffer.concat([
    new Buffer([SMP_PAIRING_CONFIRM]),
    crypto.c1(this._tk, this._r, this._pres, this._preq, this._iat, this._ia, this._rat, this._ra)
  ]));
};

Smp.prototype.handlePairingRandom = function(data) {
  var r = data.slice(1);

  var pcnf = Buffer.concat([
    new Buffer([SMP_PAIRING_CONFIRM]),
    crypto.c1(this._tk, r, this._pres, this._preq, this._iat, this._ia, this._rat, this._ra)
  ]);

  if (this._pcnf.toString('hex') === pcnf.toString('hex')) {
    this._diversifier = new Buffer('0000', 'hex');
    this._random = new Buffer('0000000000000000', 'hex');
    this._stk = crypto.s1(this._tk, this._r, r);

    mgmt.addLongTermKey(this._ia, this._iat, 0, 0, this._diversifier, this._random, this._stk);

    this.write(Buffer.concat([
      new Buffer([SMP_PAIRING_RANDOM]),
      this._r
    ]));
  } else {
    this.write(new Buffer([
      SMP_PAIRING_FAILED,
      SMP_PAIRING_CONFIRM
    ]));

    this.emit('fail');
  }
};

Smp.prototype.handlePairingFailed = function(data) {
  this.emit('fail');
};

Smp.prototype.write = function(data) {
  this._aclStream.write(SMP_CID, data);
};

module.exports = Smp;
