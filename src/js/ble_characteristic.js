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

var events = require('events');
var util = require('util');

var debug = console.log; // require('debug')('ble_characteristic');

var uuidUtil = require('ble_uuid_util');

function Characteristic(options) {
  this.uuid = uuidUtil.removeDashes(options.uuid);
  this.properties = options.properties || [];
  this.secure = options.secure || [];
  this.value = options.value || null;
  this.descriptors = options.descriptors || [];

  if (this.value && (this.properties.length !== 1 || this.properties[0] !== 'read')) {
    throw new Error('Characteristics with value can be read only!');
  }

  if (options.onReadRequest) {
    this.onReadRequest = options.onReadRequest;
  }

  if (options.onWriteRequest) {
    this.onWriteRequest = options.onWriteRequest;
  }

  if (options.onSubscribe) {
    this.onSubscribe = options.onSubscribe;
  }

  if (options.onUnsubscribe) {
    this.onUnsubscribe = options.onUnsubscribe;
  }

  if (options.onNotify) {
    this.onNotify = options.onNotify;
  }

  if (options.onIndicate) {
    this.onIndicate = options.onIndicate;
  }

  this.on('readRequest', this.onReadRequest.bind(this));
  this.on('writeRequest', this.onWriteRequest.bind(this));
  this.on('subscribe', this.onSubscribe.bind(this));
  this.on('unsubscribe', this.onUnsubscribe.bind(this));
  this.on('notify', this.onNotify.bind(this));
  this.on('indicate', this.onIndicate.bind(this));
}

util.inherits(Characteristic, events.EventEmitter);

Characteristic.RESULT_SUCCESS                  = Characteristic.prototype.RESULT_SUCCESS                  = 0x00;
Characteristic.RESULT_INVALID_OFFSET           = Characteristic.prototype.RESULT_INVALID_OFFSET           = 0x07;
Characteristic.RESULT_ATTR_NOT_LONG            = Characteristic.prototype.RESULT_ATTR_NOT_LONG            = 0x0b;
Characteristic.RESULT_INVALID_ATTRIBUTE_LENGTH = Characteristic.prototype.RESULT_INVALID_ATTRIBUTE_LENGTH = 0x0d;
Characteristic.RESULT_UNLIKELY_ERROR           = Characteristic.prototype.RESULT_UNLIKELY_ERROR           = 0x0e;

Characteristic.prototype.toString = function() {
  return JSON.stringify({
    uuid: this.uuid,
    properties: this.properties,
    secure: this.secure,
    value: this.value,
    descriptors: this.descriptors
  });
};

Characteristic.prototype.onReadRequest = function(offset, callback) {
  callback(this.RESULT_UNLIKELY_ERROR, null);
};

Characteristic.prototype.onWriteRequest = function(data, offset, withoutResponse, callback) {
  callback(this.RESULT_UNLIKELY_ERROR);
};

Characteristic.prototype.onSubscribe = function(maxValueSize, updateValueCallback) {
  this.maxValueSize = maxValueSize;
  this.updateValueCallback = updateValueCallback;
};

Characteristic.prototype.onUnsubscribe = function() {
  this.maxValueSize = null;
  this.updateValueCallback = null;
};

Characteristic.prototype.onNotify = function() {
};

Characteristic.prototype.onIndicate = function() {
};

module.exports = Characteristic;
