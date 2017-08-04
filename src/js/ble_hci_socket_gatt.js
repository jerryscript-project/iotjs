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

/*jshint loopfunc: true */

var debug = console.log; // require('debug')('ble_hci_socket_gatt');

var events = require('events');
var util = require('util');
var uuidUtil = require('ble_uuid_util');

var ATT_OP_ERROR                    = 0x01;
var ATT_OP_MTU_REQ                  = 0x02;
var ATT_OP_MTU_RESP                 = 0x03;
var ATT_OP_FIND_INFO_REQ            = 0x04;
var ATT_OP_FIND_INFO_RESP           = 0x05;
var ATT_OP_FIND_BY_TYPE_REQ         = 0x06;
var ATT_OP_FIND_BY_TYPE_RESP        = 0x07;
var ATT_OP_READ_BY_TYPE_REQ         = 0x08;
var ATT_OP_READ_BY_TYPE_RESP        = 0x09;
var ATT_OP_READ_REQ                 = 0x0a;
var ATT_OP_READ_RESP                = 0x0b;
var ATT_OP_READ_BLOB_REQ            = 0x0c;
var ATT_OP_READ_BLOB_RESP           = 0x0d;
var ATT_OP_READ_MULTI_REQ           = 0x0e;
var ATT_OP_READ_MULTI_RESP          = 0x0f;
var ATT_OP_READ_BY_GROUP_REQ        = 0x10;
var ATT_OP_READ_BY_GROUP_RESP       = 0x11;
var ATT_OP_WRITE_REQ                = 0x12;
var ATT_OP_WRITE_RESP               = 0x13;
var ATT_OP_WRITE_CMD                = 0x52;
var ATT_OP_PREP_WRITE_REQ           = 0x16;
var ATT_OP_PREP_WRITE_RESP          = 0x17;
var ATT_OP_EXEC_WRITE_REQ           = 0x18;
var ATT_OP_EXEC_WRITE_RESP          = 0x19;
var ATT_OP_HANDLE_NOTIFY            = 0x1b;
var ATT_OP_HANDLE_IND               = 0x1d;
var ATT_OP_HANDLE_CNF               = 0x1e;
var ATT_OP_SIGNED_WRITE_CMD         = 0xd2;

var GATT_PRIM_SVC_UUID              = 0x2800;
var GATT_INCLUDE_UUID               = 0x2802;
var GATT_CHARAC_UUID                = 0x2803;

var GATT_CLIENT_CHARAC_CFG_UUID     = 0x2902;
var GATT_SERVER_CHARAC_CFG_UUID     = 0x2903;

var ATT_ECODE_SUCCESS               = 0x00;
var ATT_ECODE_INVALID_HANDLE        = 0x01;
var ATT_ECODE_READ_NOT_PERM         = 0x02;
var ATT_ECODE_WRITE_NOT_PERM        = 0x03;
var ATT_ECODE_INVALID_PDU           = 0x04;
var ATT_ECODE_AUTHENTICATION        = 0x05;
var ATT_ECODE_REQ_NOT_SUPP          = 0x06;
var ATT_ECODE_INVALID_OFFSET        = 0x07;
var ATT_ECODE_AUTHORIZATION         = 0x08;
var ATT_ECODE_PREP_QUEUE_FULL       = 0x09;
var ATT_ECODE_ATTR_NOT_FOUND        = 0x0a;
var ATT_ECODE_ATTR_NOT_LONG         = 0x0b;
var ATT_ECODE_INSUFF_ENCR_KEY_SIZE  = 0x0c;
var ATT_ECODE_INVAL_ATTR_VALUE_LEN  = 0x0d;
var ATT_ECODE_UNLIKELY              = 0x0e;
var ATT_ECODE_INSUFF_ENC            = 0x0f;
var ATT_ECODE_UNSUPP_GRP_TYPE       = 0x10;
var ATT_ECODE_INSUFF_RESOURCES      = 0x11;

var ATT_CID = 0x0004;

var Gatt = function() {
  this.maxMtu = 256;
  this._mtu = 23;
  this._preparedWriteRequest = null;

  this.setServices([]);

  this.onAclStreamDataBinded = this.onAclStreamData.bind(this);
  this.onAclStreamEndBinded = this.onAclStreamEnd.bind(this);
};

util.inherits(Gatt, events.EventEmitter);

Gatt.prototype.setServices = function(services) {
  var deviceName = process.env.BLENO_DEVICE_NAME || process.platform;

  // base services and characteristics
  var allServices = [
    {
      uuid: '1800',
      characteristics: [
        {
          uuid: '2a00',
          properties: ['read'],
          secure: [],
          value: new Buffer(deviceName),
          descriptors: []
        },
        {
          uuid: '2a01',
          properties: ['read'],
          secure: [],
          value: new Buffer([0x80, 0x00]),
          descriptors: []
        }
      ]
    },
    {
      uuid: '1801',
      characteristics: [
        {
          uuid: '2a05',
          properties: ['indicate'],
          secure: [],
          value: new Buffer([0x00, 0x00, 0x00, 0x00]),
          descriptors: []
        }
      ]
    }
  ].concat(services);

  this._handles = [];

  var handle = 0;
  var i;
  var j;

  for (i = 0; i < allServices.length; i++) {
    var service = allServices[i];

    handle++;
    var serviceHandle = handle;

    this._handles[serviceHandle] = {
      type: 'service',
      uuid: service.uuid,
      attribute: service,
      startHandle: serviceHandle
      // endHandle filled in below
    };

    for (j = 0; j < service.characteristics.length; j++) {
      var characteristic = service.characteristics[j];

      var properties = 0;
      var secure = 0;

      if (characteristic.properties.indexOf('read') !== -1) {
        properties |= 0x02;

        if (characteristic.secure.indexOf('read') !== -1) {
          secure |= 0x02;
        }
      }

      if (characteristic.properties.indexOf('writeWithoutResponse') !== -1) {
        properties |= 0x04;

        if (characteristic.secure.indexOf('writeWithoutResponse') !== -1) {
          secure |= 0x04;
        }
      }

      if (characteristic.properties.indexOf('write') !== -1) {
        properties |= 0x08;

        if (characteristic.secure.indexOf('write') !== -1) {
          secure |= 0x08;
        }
      }

      if (characteristic.properties.indexOf('notify') !== -1) {
        properties |= 0x10;

        if (characteristic.secure.indexOf('notify') !== -1) {
          secure |= 0x10;
        }
      }

      if (characteristic.properties.indexOf('indicate') !== -1) {
        properties |= 0x20;

        if (characteristic.secure.indexOf('indicate') !== -1) {
          secure |= 0x20;
        }
      }

      handle++;
      var characteristicHandle = handle;

      handle++;
      var characteristicValueHandle = handle;

      this._handles[characteristicHandle] = {
        type: 'characteristic',
        uuid: characteristic.uuid,
        properties: properties,
        secure: secure,
        attribute: characteristic,
        startHandle: characteristicHandle,
        valueHandle: characteristicValueHandle
      };

      this._handles[characteristicValueHandle] = {
        type: 'characteristicValue',
        handle: characteristicValueHandle,
        value: characteristic.value
      };

      if (properties & 0x30) { // notify or indicate
        // add client characteristic configuration descriptor

        handle++;
        var clientCharacteristicConfigurationDescriptorHandle = handle;
        this._handles[clientCharacteristicConfigurationDescriptorHandle] = {
          type: 'descriptor',
          handle: clientCharacteristicConfigurationDescriptorHandle,
          uuid: '2902',
          attribute: characteristic,
          properties: (0x02 | 0x04 | 0x08), // read/write
          secure: (secure & 0x10) ? (0x02 | 0x04 | 0x08) : 0,
          value: new Buffer([0x00, 0x00])
        };
      }

      for (var k = 0; k < characteristic.descriptors.length; k++) {
        var descriptor = characteristic.descriptors[k];

        handle++;
        var descriptorHandle = handle;

        this._handles[descriptorHandle] = {
          type: 'descriptor',
          handle: descriptorHandle,
          uuid: descriptor.uuid,
          attribute: descriptor,
          properties: 0x02, // read only
          secure: 0x00,
          value: descriptor.value
        };
      }
    }

    this._handles[serviceHandle].endHandle = handle;
  }

  var debugHandles = [];
  for (i = 0; i < this._handles.length; i++) {
    handle = this._handles[i];

    debugHandles[i] = {};
    for(j in handle) {
      if (Buffer.isBuffer(handle[j])) {
        debugHandles[i][j] = handle[j] ? 'Buffer(\'' + handle[j].toString('hex') + '\', \'hex\')' : null;
      } else if (j !== 'attribute') {
        debugHandles[i][j] = handle[j];
      }
    }
  }

  debug('handles = ' + JSON.stringify(debugHandles, null, 2));
};

Gatt.prototype.setAclStream = function(aclStream) {
  this._mtu = 23;
  this._preparedWriteRequest = null;

  this._aclStream = aclStream;

  this._aclStream.on('data', this.onAclStreamDataBinded);
  this._aclStream.on('end', this.onAclStreamEndBinded);
};

Gatt.prototype.onAclStreamData = function(cid, data) {
  if (cid !== ATT_CID) {
    return;
  }

  this.handleRequest(data);
};

Gatt.prototype.onAclStreamEnd = function() {
  this._aclStream.removeListener('data', this.onAclStreamDataBinded);
  this._aclStream.removeListener('end', this.onAclStreamEndBinded);
};

Gatt.prototype.send = function(data) {
  debug('send: ' + data.toString('hex'));
  this._aclStream.write(ATT_CID, data);
};

Gatt.prototype.errorResponse = function(opcode, handle, status) {
  var buf = new Buffer(5);

  buf.writeUInt8(ATT_OP_ERROR, 0);
  buf.writeUInt8(opcode, 1);
  buf.writeUInt16LE(handle, 2);
  buf.writeUInt8(status, 4);

  return buf;
};

Gatt.prototype.handleRequest = function(request) {
  debug('handing request: ' + request.toString('hex'));

  var requestType = request.readUInt8(0); //buf[0];
  var response = null;

  switch(requestType) {
    case ATT_OP_MTU_REQ:
      response = this.handleMtuRequest(request);
      break;

    case ATT_OP_FIND_INFO_REQ:
      response = this.handleFindInfoRequest(request);
      break;

    case ATT_OP_FIND_BY_TYPE_REQ:
      response = this.handleFindByTypeRequest(request);
      break;

    case ATT_OP_READ_BY_TYPE_REQ:
      response = this.handleReadByTypeRequest(request);
      break;

    case ATT_OP_READ_REQ:
    case ATT_OP_READ_BLOB_REQ:
      response = this.handleReadOrReadBlobRequest(request);
      break;

    case ATT_OP_READ_BY_GROUP_REQ:
      response = this.handleReadByGroupRequest(request);
      break;

    case ATT_OP_WRITE_REQ:
    case ATT_OP_WRITE_CMD:
      response = this.handleWriteRequestOrCommand(request);
      break;

    case ATT_OP_PREP_WRITE_REQ:
      response = this.handlePrepareWriteRequest(request);
      break;

    case ATT_OP_EXEC_WRITE_REQ:
      response = this.handleExecuteWriteRequest(request);
      break;

    case ATT_OP_HANDLE_CNF:
      response = this.handleConfirmation(request);
      break;

    default:
    case ATT_OP_READ_MULTI_REQ:
    case ATT_OP_SIGNED_WRITE_CMD:
      response = this.errorResponse(requestType, 0x0000, ATT_ECODE_REQ_NOT_SUPP);
      break;
  }

  if (response) {
    debug('response: ' + response.toString('hex'));

    this.send(response);
  }
};

Gatt.prototype.handleMtuRequest = function(request) {
  var mtu = request.readUInt16LE(1);

  if (mtu < 23) {
    mtu = 23;
  } else if (mtu > this.maxMtu) {
    mtu = this.maxMtu;
  }

  this._mtu = mtu;

  this.emit('mtuChange', this._mtu);

  var response = new Buffer(3);

  response.writeUInt8(ATT_OP_MTU_RESP, 0);
  response.writeUInt16LE(mtu, 1);

  return response;
};

Gatt.prototype.handleFindInfoRequest = function(request) {
  var response = null;

  var startHandle = request.readUInt16LE(1);
  var endHandle = request.readUInt16LE(3);

  var infos = [];
  var uuid = null;

  for (i = startHandle; i <= endHandle; i++) {
    var handle = this._handles[i];

    if (!handle) {
      break;
    }

    uuid = null;

    if ('service' === handle.type) {
      uuid = '2800';
    } else if ('includedService' === handle.type) {
      uuid = '2802';
    } else if ('characteristic' === handle.type) {
      uuid = '2803';
    } else if ('characteristicValue' === handle.type) {
      uuid = this._handles[i - 1].uuid;
    } else if ('descriptor' === handle.type) {
      uuid = handle.uuid;
    }

    if (uuid) {
      infos.push({
        handle: i,
        uuid: uuid
      });
    }
  }

  if (infos.length) {
    var uuidSize = infos[0].uuid.length / 2;
    var numInfo = 1;

    for (i = 1; i < infos.length; i++) {
      if (infos[0].uuid.length !== infos[i].uuid.length) {
        break;
      }
      numInfo++;
    }

    var lengthPerInfo = (uuidSize === 2) ? 4 : 18;
    var maxInfo = Math.floor((this._mtu - 2) / lengthPerInfo);
    numInfo = Math.min(numInfo, maxInfo);

    response = new Buffer(2 + numInfo * lengthPerInfo);

    //response[0] = ATT_OP_FIND_INFO_RESP;
    //response[1] = (uuidSize === 2) ? 0x01 : 0x2;
    response.writeUInt8(ATT_OP_FIND_INFO_RESP, 0);
    response.writeUInt8((uuidSize === 2) ? 0x01 : 0x2, 1);

    for (i = 0; i < numInfo; i++) {
      var info = infos[i];

      response.writeUInt16LE(info.handle, 2 + i * lengthPerInfo);

      uuid = new Buffer(uuidUtil.reverseByteOrder(info.uuid, ''), 'hex');
      for (var j = 0; j < uuid.length; j++) {
        //response[2 + i * lengthPerInfo + 2 + j] = uuid[j];
        response.writeUInt8(uuid[j], 2 + i * lengthPerInfo + 2 + j);
      }
    }
  } else {
    response = this.errorResponse(ATT_OP_FIND_INFO_REQ, startHandle, ATT_ECODE_ATTR_NOT_FOUND);
  }

  return response;
};

Gatt.prototype.handleFindByTypeRequest = function(request) {
  var response = null;

  var startHandle = request.readUInt16LE(1);
  var endHandle = request.readUInt16LE(3);
  var uuid = uuidUtil.reverseByteOrder(request.slice(5, 7).toString('hex'), '');
  var value = uuidUtil.reverseByteOrder(request.slice(7).toString('hex'), '');

  var handles = [];
  var handle;

  for (var i = startHandle; i <= endHandle; i++) {
    handle = this._handles[i];

    if (!handle) {
      break;
    }

    if ('2800' === uuid && handle.type === 'service' && handle.uuid === value) {
      handles.push({
        start: handle.startHandle,
        end: handle.endHandle
      });
    }
  }

  if (handles.length) {
    var lengthPerHandle = 4;
    var numHandles = handles.length;
    var maxHandles = Math.floor((this._mtu - 1) / lengthPerHandle);

    numHandles = Math.min(numHandles, maxHandles);

    response = new Buffer(1 + numHandles * lengthPerHandle);

    //response[0] = ATT_OP_FIND_BY_TYPE_RESP;
    response.writeUInt8(ATT_OP_FIND_BY_TYPE_RESP, 0);

    for (i = 0; i < numHandles; i++) {
      handle = handles[i];

      response.writeUInt16LE(handle.start, 1 + i * lengthPerHandle);
      response.writeUInt16LE(handle.end, 1 + i * lengthPerHandle + 2);
    }
  } else {
    response = this.errorResponse(ATT_OP_FIND_BY_TYPE_REQ, startHandle, ATT_ECODE_ATTR_NOT_FOUND);
  }

  return response;
};

Gatt.prototype.handleReadByGroupRequest = function(request) {
  var response = null;

  var startHandle = request.readUInt16LE(1);
  var endHandle = request.readUInt16LE(3);
  var uuid = uuidUtil.reverseByteOrder(request.slice(5).toString('hex'), '');

  debug('read by group: startHandle = 0x' + startHandle.toString(16) + ', endHandle = 0x' + endHandle.toString(16) + ', uuid = 0x' + uuid.toString(16));

  if ('2800' === uuid || '2802' === uuid) {
    var services = [];
    var type = ('2800' === uuid) ? 'service' : 'includedService';
    var i;

    for (i = startHandle; i <= endHandle; i++) {
      var handle = this._handles[i];

      if (!handle) {
        break;
      }

      if (handle.type === type) {
        services.push(handle);
      }
    }

    if (services.length) {
      var uuidSize = services[0].uuid.length / 2;
      var numServices = 1;

      for (i = 1; i < services.length; i++) {
        if (services[0].uuid.length !== services[i].uuid.length) {
          break;
        }
        numServices++;
      }

      var lengthPerService = (uuidSize === 2) ? 6 : 20;
      var maxServices = Math.floor((this._mtu - 2) / lengthPerService);
      numServices = Math.min(numServices, maxServices);

      response = new Buffer(2 + numServices * lengthPerService);

      //response[0] = ATT_OP_READ_BY_GROUP_RESP;
      //response[1] = lengthPerService;
      response.writeUInt8(ATT_OP_READ_BY_GROUP_RESP, 0);
      response.writeUInt8(lengthPerService, 1);

      for (i = 0; i < numServices; i++) {
        var service = services[i];

        response.writeUInt16LE(service.startHandle, 2 + i * lengthPerService);
        response.writeUInt16LE(service.endHandle, 2 + i * lengthPerService + 2);

        var serviceUuid = new Buffer(uuidUtil.reverseByteOrder(service.uuid, ''), 'hex');
        for (var j = 0; j < serviceUuid.length; j++) {
          //response[2 + i * lengthPerService + 4 + j] = serviceUuid[j];
          response.writeUInt8(serviceUuid.readUInt8(j), 2 + i * lengthPerService + 4 + j);
        }
      }
    } else {
      response = this.errorResponse(ATT_OP_READ_BY_GROUP_REQ, startHandle, ATT_ECODE_ATTR_NOT_FOUND);
    }
  } else {
    response = this.errorResponse(ATT_OP_READ_BY_GROUP_REQ, startHandle, ATT_ECODE_UNSUPP_GRP_TYPE);
  }

  return response;
};

Gatt.prototype.handleReadByTypeRequest = function(request) {
  var response = null;

  var startHandle = request.readUInt16LE(1);
  var endHandle = request.readUInt16LE(3);
  var uuid = uuidUtil.reverseByteOrder(request.slice(5).toString('hex'), '');
  var i;
  var handle;

  debug('read by type: startHandle = 0x' + startHandle.toString(16) + ', endHandle = 0x' + endHandle.toString(16) + ', uuid = 0x' + uuid.toString(16));

  if ('2803' === uuid) {
    var characteristics = [];

    for (i = startHandle; i <= endHandle; i++) {
      handle = this._handles[i];

      if (!handle) {
        break;
      }

      if (handle.type === 'characteristic') {
        characteristics.push(handle);
      }
    }

    if (characteristics.length) {
      var uuidSize = characteristics[0].uuid.length / 2;
      var numCharacteristics = 1;

      for (i = 1; i < characteristics.length; i++) {
        if (characteristics[0].uuid.length !== characteristics[i].uuid.length) {
          break;
        }
        numCharacteristics++;
      }

      var lengthPerCharacteristic = (uuidSize === 2) ? 7 : 21;
      var maxCharacteristics = Math.floor((this._mtu - 2) / lengthPerCharacteristic);
      numCharacteristics = Math.min(numCharacteristics, maxCharacteristics);

      response = new Buffer(2 + numCharacteristics * lengthPerCharacteristic);

      //response[0] = ATT_OP_READ_BY_TYPE_RESP;
      //response[1] = lengthPerCharacteristic;
      response.writeUInt8(ATT_OP_READ_BY_TYPE_RESP, 0);
      response.writeUInt8(lengthPerCharacteristic, 1);

      for (i = 0; i < numCharacteristics; i++) {
        var characteristic = characteristics[i];

        response.writeUInt16LE(characteristic.startHandle, 2 + i * lengthPerCharacteristic);
        response.writeUInt8(characteristic.properties, 2 + i * lengthPerCharacteristic + 2);
        response.writeUInt16LE(characteristic.valueHandle, 2 + i * lengthPerCharacteristic + 3);

        var characteristicUuid = new Buffer(uuidUtil.reverseByteOrder(characteristic.uuid, ''), 'hex');
        for (var j = 0; j < characteristicUuid.length; j++) {
          //response[2 + i * lengthPerCharacteristic + 5 + j] = characteristicUuid[j];
          response.writeUInt8(characteristicUuid.readUInt8(j), 2 + i * lengthPerCharacteristic + 5 + j);
        }
      }
    } else {
      response = this.errorResponse(ATT_OP_READ_BY_TYPE_REQ, startHandle, ATT_ECODE_ATTR_NOT_FOUND);
    }
  } else {
    var handleAttribute = null;
    var valueHandle = null;
    var secure = false;

    for (i = startHandle; i <= endHandle; i++) {
      handle = this._handles[i];

      if (!handle) {
        break;
      }

      if (handle.type === 'characteristic' && handle.uuid === uuid) {
        handleAttribute = handle.attribute;
        valueHandle = handle.valueHandle;
        secure = handle.secure & 0x02;
        break;
      } else if (handle.type === 'descriptor' && handle.uuid === uuid) {
        valueHandle = i;
        secure = handle.secure & 0x02;
        break;
      }
    }

    if (secure && !this._aclStream.encrypted) {
      response = this.errorResponse(ATT_OP_READ_BY_TYPE_REQ, startHandle, ATT_ECODE_AUTHENTICATION);
    } else if (valueHandle) {
      var callback = (function(valueHandle) {
        return function(result, data) {
          var callbackResponse = null;

          if (ATT_ECODE_SUCCESS === result) {
            var dataLength = Math.min(data.length, this._mtu - 4);
            callbackResponse = new Buffer(4 + dataLength);

            //callbackResponse[0] = ATT_OP_READ_BY_TYPE_RESP;
            //callbackResponse[1] = dataLength + 2;
            callbackResponse.writeUInt8(ATT_OP_READ_BY_TYPE_RESP, 0);
            callbackResponse.writeUInt8(dataLength + 2, 1);
            callbackResponse.writeUInt16LE(valueHandle, 2);
            for (i = 0; i < dataLength; i++) {
              //callbackResponse[4 + i] = data[i];
              callbackResponse.writeUInt8(data.readUInt8(i), 4 + i);
            }
          } else {
            callbackResponse = this.errorResponse(requestType, valueHandle, result);
          }

          debug('read by type response: ' + callbackResponse.toString('hex'));

          this.send(callbackResponse);
        }.bind(this);
      }.bind(this))(valueHandle);

      var data = this._handles[valueHandle].value;

      if (data) {
        callback(ATT_ECODE_SUCCESS, data);
      } else if (handleAttribute) {
        handleAttribute.emit('readRequest', 0, callback);
      } else {
        callback(ATT_ECODE_UNLIKELY);
      }
    } else {
      response = this.errorResponse(ATT_OP_READ_BY_TYPE_REQ, startHandle, ATT_ECODE_ATTR_NOT_FOUND);
    }
  }

  return response;
};

Gatt.prototype.handleReadOrReadBlobRequest = function(request) {
  var response = null;

  //var requestType = request[0];
  var requestType = request.readUInt8(0);
  var valueHandle = request.readUInt16LE(1);
  var offset = (requestType === ATT_OP_READ_BLOB_REQ) ? request.readUInt16LE(3) : 0;

  var handle = this._handles[valueHandle];

  if (handle) {
    var result = null;
    var data = null;
    var handleType = handle.type;

    var callback = (function(requestType, valueHandle) {
      return function(result, data) {
        var callbackResponse = null;

        if (ATT_ECODE_SUCCESS === result) {
          var dataLength = Math.min(data.length, this._mtu - 1);
          callbackResponse = new Buffer(1 + dataLength);

          //callbackResponse[0] = (requestType === ATT_OP_READ_BLOB_REQ) ? ATT_OP_READ_BLOB_RESP : ATT_OP_READ_RESP;
          callbackResponse.writeUInt8((requestType === ATT_OP_READ_BLOB_REQ) ? ATT_OP_READ_BLOB_RESP : ATT_OP_READ_RESP, 0);
          for (i = 0; i < dataLength; i++) {
            //callbackResponse[1 + i] = data[i];
            callbackResponse.writeUInt8(data.readUInt8(i), 1 + i);
          }
        } else {
          callbackResponse = this.errorResponse(requestType, valueHandle, result);
        }

        debug('read response: ' + callbackResponse.toString('hex'));

        this.send(callbackResponse);
      }.bind(this);
    }.bind(this))(requestType, valueHandle);

    if (handleType === 'service' || handleType === 'includedService') {
      result = ATT_ECODE_SUCCESS;
      data = new Buffer(uuidUtil.reverseByteOrder(handle.uuid, ''), 'hex');
    } else if (handleType === 'characteristic') {
      var uuid = new Buffer(uuidUtil.reverseByteOrder(handle.uuid, ''), 'hex');

      result = ATT_ECODE_SUCCESS;
      data = new Buffer(3 + uuid.length);
      data.writeUInt8(handle.properties, 0);
      data.writeUInt16LE(handle.valueHandle, 1);

      for (i = 0; i < uuid.length; i++) {
        //data[i + 3] = uuid[i];
        data.writeUInt8(uuid.readUInt8(i), i + 3);
      }
    } else if (handleType === 'characteristicValue' || handleType === 'descriptor') {
      var handleProperties = handle.properties;
      var handleSecure = handle.secure;
      var handleAttribute = handle.attribute;
      if (handleType === 'characteristicValue') {
        handleProperties = this._handles[valueHandle - 1].properties;
        handleSecure = this._handles[valueHandle - 1].secure;
        handleAttribute = this._handles[valueHandle - 1].attribute;
      }

      if (handleProperties & 0x02) {
        if (handleSecure & 0x02 && !this._aclStream.encrypted) {
          result = ATT_ECODE_AUTHENTICATION;
        } else {
          data = handle.value;

          if (data) {
            result = ATT_ECODE_SUCCESS;
          } else {
            handleAttribute.emit('readRequest', offset, callback);
          }
        }
      } else {
        result = ATT_ECODE_READ_NOT_PERM; // non-readable
      }
    }

    if (data && typeof data === 'string') {
      data = new Buffer(data);
    }

    if (result === ATT_ECODE_SUCCESS && data && offset) {
      if (data.length < offset) {
        errorCode = ATT_ECODE_INVALID_OFFSET;
        data = null;
      } else {
        data = data.slice(offset);
      }
    }

    if (result !== null) {
      callback(result, data);
    }
  } else {
    response = this.errorResponse(requestType, valueHandle, ATT_ECODE_INVALID_HANDLE);
  }

  return response;
};

Gatt.prototype.handleWriteRequestOrCommand = function(request) {
  var response = null;

  //var requestType = request[0];
  var requestType = request.readUInt8(0);
  var withoutResponse = (requestType === ATT_OP_WRITE_CMD);
  var valueHandle = request.readUInt16LE(1);
  var data = request.slice(3);
  var offset = 0;

  var handle = this._handles[valueHandle];

  if (handle) {
    if (handle.type === 'characteristicValue') {
      handle = this._handles[valueHandle - 1];
    }

    var handleProperties = handle.properties;
    var handleSecure = handle.secure;

    if (handleProperties && (withoutResponse ? (handleProperties & 0x04) : (handleProperties & 0x08))) {

      var callback = (function(requestType, valueHandle, withoutResponse) {
        return function(result) {
          if (!withoutResponse) {
            var callbackResponse = null;

            if (ATT_ECODE_SUCCESS === result) {
              callbackResponse = new Buffer([ATT_OP_WRITE_RESP]);
            } else {
              callbackResponse = this.errorResponse(requestType, valueHandle, result);
            }

            debug('write response: ' + callbackResponse.toString('hex'));

            this.send(callbackResponse);
          }
        }.bind(this);
      }.bind(this))(requestType, valueHandle, withoutResponse);

      if (handleSecure & (withoutResponse ? 0x04 : 0x08) && !this._aclStream.encrypted) {
        response = this.errorResponse(requestType, valueHandle, ATT_ECODE_AUTHENTICATION);
      } else if (handle.type === 'descriptor' || handle.uuid === '2902') {
        var result = null;

        if (data.length !== 2) {
          result = ATT_ECODE_INVAL_ATTR_VALUE_LEN;
        } else {
          var value = data.readUInt16LE(0);
          var handleAttribute = handle.attribute;

          handle.value = data;

          if (value & 0x0003) {
            var updateValueCallback = (function(valueHandle, attribute) {
              return function(data) {
                var dataLength = Math.min(data.length, this._mtu - 3);
                var useNotify = attribute.properties.indexOf('notify') !== -1;
                var useIndicate = attribute.properties.indexOf('indicate') !== -1;
                var i;

                if (useNotify) {
                  var notifyMessage = new Buffer(3 + dataLength);

                  notifyMessage.writeUInt8(ATT_OP_HANDLE_NOTIFY, 0);
                  notifyMessage.writeUInt16LE(valueHandle, 1);

                  for (i = 0; i < dataLength; i++) {
                    //notifyMessage[3 + i] = data[i];
                    notifyMessage.writeUInt8(data.readUInt8(i), 3 + i);
                  }

                  debug('notify message: ' + notifyMessage.toString('hex'));
                  this.send(notifyMessage);

                  attribute.emit('notify');
                } else if (useIndicate) {
                  var indicateMessage = new Buffer(3 + dataLength);

                  indicateMessage.writeUInt8(ATT_OP_HANDLE_IND, 0);
                  indicateMessage.writeUInt16LE(valueHandle, 1);

                  for (i = 0; i < dataLength; i++) {
                    //indicateMessage[3 + i] = data[i];
                    indicateMessage.writeUInt8(data.readUInt8(i), 3 + i);
                  }

                  this._lastIndicatedAttribute = attribute;

                  debug('indicate message: ' + indicateMessage.toString('hex'));
                  this.send(indicateMessage);
                }
              }.bind(this);
            }.bind(this))(valueHandle - 1, handleAttribute);

            if (handleAttribute.emit) {
              handleAttribute.emit('subscribe', this._mtu - 3, updateValueCallback);
            }
          } else {
            handleAttribute.emit('unsubscribe');
          }

          result = ATT_ECODE_SUCCESS;
        }

        callback(result);
      } else {
        handle.attribute.emit('writeRequest', data, offset, withoutResponse, callback);
      }
    } else {
      response = this.errorResponse(requestType, valueHandle, ATT_ECODE_WRITE_NOT_PERM);
    }
  } else {
    response = this.errorResponse(requestType, valueHandle, ATT_ECODE_INVALID_HANDLE);
  }

  return response;
};

Gatt.prototype.handlePrepareWriteRequest = function(request) {
  var response = null;

  //var requestType = request[0];
  var requestType = request.readUInt8(0);
  var valueHandle = request.readUInt16LE(1);
  var offset = request.readUInt16LE(3);
  var data = request.slice(5);

  var handle = this._handles[valueHandle];

  if (handle) {
    if (handle.type === 'characteristicValue') {
      handle = this._handles[valueHandle - 1];

      var handleProperties = handle.properties;
      var handleSecure = handle.secure;

      if (handleProperties && (handleProperties & 0x08)) {
        if ((handleSecure & 0x08) && !this._aclStream.encrypted) {
          response = this.errorResponse(requestType, valueHandle, ATT_ECODE_AUTHENTICATION);
        } else if (this._preparedWriteRequest) {
          if (this._preparedWriteRequest.handle !== handle) {
            response = this.errorResponse(requestType, valueHandle, ATT_ECODE_UNLIKELY);
          } else if (offset === (this._preparedWriteRequest.offset + this._preparedWriteRequest.data.length)) {
            this._preparedWriteRequest.data = Buffer.concat([
              this._preparedWriteRequest.data,
              data
            ]);

            response = new Buffer(request.length);
            request.copy(response);
            //response[0] = ATT_OP_PREP_WRITE_RESP;
            response.writeUInt8(ATT_OP_PREP_WRITE_RESP, 0);
          } else {
            response = this.errorResponse(requestType, valueHandle, ATT_ECODE_INVALID_OFFSET);
          }
        } else {
          this._preparedWriteRequest = {
            handle: handle,
            valueHandle: valueHandle,
            offset: offset,
            data: data
          };

          response = new Buffer(request.length);
          request.copy(response);
          // response[0] = ATT_OP_PREP_WRITE_RESP;
          response.writeUInt8(ATT_OP_PREP_WRITE_RESP, 0);
        }
      } else {
        response = this.errorResponse(requestType, valueHandle, ATT_ECODE_WRITE_NOT_PERM);
      }
    } else {
      response = this.errorResponse(requestType, valueHandle, ATT_ECODE_ATTR_NOT_LONG);
    }
  } else {
    response = this.errorResponse(requestType, valueHandle, ATT_ECODE_INVALID_HANDLE);
  }

  return response;
};

Gatt.prototype.handleExecuteWriteRequest = function(request) {
  var response = null;

  //var requestType = request[0];
  //var flag = request[1];
  var requestType = request.readUInt8(0);
  var flag = request.readUInt8(1);

  if (this._preparedWriteRequest) {
    var valueHandle = this._preparedWriteRequest.valueHandle;

    if (flag === 0x00) {
      response = new Buffer([ATT_OP_EXEC_WRITE_RESP]);
    } else if (flag === 0x01) {
      var callback = (function(requestType, valueHandle) {
        return function(result) {
          var callbackResponse = null;

          if (ATT_ECODE_SUCCESS === result) {
            callbackResponse = new Buffer([ATT_OP_EXEC_WRITE_RESP]);
          } else {
            callbackResponse = this.errorResponse(requestType, valueHandle, result);
          }

          debug('execute write response: ' + callbackResponse.toString('hex'));

          this.send(callbackResponse);
        }.bind(this);
      }.bind(this))(requestType, this._preparedWriteRequest.valueHandle);

      this._preparedWriteRequest.handle.attribute.emit('writeRequest', this._preparedWriteRequest.data, this._preparedWriteRequest.offset, false, callback);
    } else {
      response = this.errorResponse(requestType, 0x0000, ATT_ECODE_UNLIKELY);
    }

    this._preparedWriteRequest = null;
  } else {
    response = this.errorResponse(requestType, 0x0000, ATT_ECODE_UNLIKELY);
  }

  return response;
};

Gatt.prototype.handleConfirmation = function(request) {
  if (this._lastIndicatedAttribute) {
    if (this._lastIndicatedAttribute.emit) {
      this._lastIndicatedAttribute.emit('indicate');
    }

    this._lastIndicatedAttribute = null;
  }
};

module.exports = Gatt;
