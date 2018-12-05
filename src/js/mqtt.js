/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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

var util = require('util');
var EventEmitter = require('events').EventEmitter;
var net, tls;

var PacketTypeEnum = {
  PUBACK: 4,
  PUBREC: 5,
  PUBREL: 6,
  PUBCOMP: 7,
};

// In seconds (should be divisible by 8)
var MQTTTimeout = 64;

function MQTTHandle(client, keepalive) {
  this.client = client;
  this.isConnected = false;
  this.nextPacketId = 0;
  this.keepalive = keepalive;
  this.keepaliveCounter = 0;
  this.pingrespCounter = 0;
  this.storage = { };
  this.storageCount = 0;

  native.MqttInit(this);
}
MQTTHandle.prototype = {};

MQTTHandle.prototype.write = function(buf) {
  this.socket.write(buf);
  this.keepaliveCounter = 0;
};

MQTTHandle.prototype.sendAck = function(type, packet_id) {
  this.write(native.sendAck(type, packet_id));
};

MQTTHandle.prototype.onconnection = function() {
  this.isConnected = true;
  this.timer = setInterval(storageTimerHit.bind(this), 1000);

  this.client.emit('connect');
};

MQTTHandle.prototype.onEnd = function() {
  this.isConnected = false;

  // Abort outgoing messages.
  clearInterval(this.timer);
  this.storage = null;
  this.storageCount = 0;

  this.client.emit('end');
};

MQTTHandle.prototype.onmessage = function(message, topic, qos, packet_id) {
  var data = {
    message: message,
    topic: topic,
    qos: qos,
    packet_id: packet_id,
  };

  if (qos >= 1) {
    var type = (qos == 1) ? PacketTypeEnum.PUBACK : PacketTypeEnum.PUBREC;

    this.sendAck(type, packet_id);
  }

  this.client.emit('message', data);
};

MQTTHandle.prototype.getPacketId = function() {
  while (true) {
    var packet_id = this.nextPacketId;
    this.nextPacketId = (this.nextPacketId + 1) & 0xffff;

    if (!(packet_id in this.storage)) {
      this.storage[packet_id] = { remainingTime: MQTTTimeout };
      this.storageCount++;

      return packet_id;
    }
  }
};

MQTTHandle.prototype.releasePacket = function(packet_id, error) {
  // callback will be invalid after delete
  var callback = this.storage[packet_id].callback;

  delete this.storage[packet_id];
  this.storageCount--;

  // This function should never fail.
  try {
    if (typeof callback == 'function') {
      callback(error);
    } else if (error) {
      this.client.emit('error', error);
    }
  } catch (e) {
    // Do nothing.
  }
};

MQTTHandle.prototype.onpingresp = function() {
  this.pingrespCounter = 0;
};

MQTTHandle.prototype.onack = function(packet_id, error) {
  this.releasePacket(packet_id, error);
};

MQTTHandle.prototype.onpubrec = function(packet_id) {
  /*
   * Qos level 2
   * Handle PUBREC package. If this package is arrived, we have to send back
   * a PUBREL package to the server.
   */
  var buffer = native.sendAck(PacketTypeEnum.PUBREL, packet_id);
  this.write(buffer);

  // Upodate packet rather than create a new one
  var packet = this.storage[packet_id];
  packet.remainingTime = MQTTTimeout;
  packet.packet = buffer;
};

MQTTHandle.prototype.onpubrel = function(data) {
  /*
   * Qos level 2
   * Handle PUBREL package. If this package is arrived, we have to send back
   * a PUBCOMP package to the server.
   */
  this.sendAck(PacketTypeEnum.PUBCOMP, data);
};

function MQTTClient(url, options, callback) {
  if (!(this instanceof MQTTClient)) {
    return new MQTTClient(url, options, callback);
  }

  EventEmitter.call(this);

  var socket;

  if (typeof url == 'string') {
    if (typeof options == 'function') {
      callback = options;
      options = {};
    }
  } else {
    if (typeof url == 'function') {
      callback = url;
      options = {};
    } else if (typeof options == 'function') {
      callback = options;
      options = url;
    } else {
      options = url;
    }

    if (options.socket) {
      socket = options.socket;
    } else {
      url = options.host || '127.0.0.1';
    }
  }

  if (typeof callback == 'function') {
    this.on('connect', callback);
  }

  if (options.will) {
    if (typeof options.topic == 'undefined' ||
        typeof options.message == 'undefined' ||
        options.qos < 0 || options.qos > 2) {
      throw new Error('Incorrect mqtt will options');
    }
  }

  var host = '';
  var create_tls = false;

  if (!socket) {
    if (url.substring(0, 8) == 'mqtts://') {
      create_tls = true;
      host = url.substring(8);
    } else if (url.substring(0, 7) == 'mqtt://') {
      host = url.substring(7);
    } else {
      host = url;
    }
  }

  var keepalive = (options.keepalive || 60) | 0;

  if (keepalive < 30) {
    keepalive = 30;
  }

  if (keepalive > 65535) {
    keepalive = 65535;
  }

  options = Object.create(options, {
    clientId: { value: options.clientId || defaultClientId() },
    host: { value: host },
    port: { value: options.port || 8883 },
    qos: { value: options.qos || 0 },
    keepalive: { value: keepalive },
  });

  // Since network transmission takes time, the
  // actual keepalive message is sent a bit earlier
  this._handle = new MQTTHandle(this, keepalive - 5);

  var connectionMessage = native.connect(options);

  var onconnect = function() {
    // Currently the connect message is tried only once.
    // Multiple tries can be implemented later.
    this.write(connectionMessage);
  };

  if (socket) {
    onconnect.call(socket);
  } else {
    if (create_tls) {
      if (!tls) {
        tls = require('tls');
      }

      socket = tls.connect(options, onconnect);
    } else {
      if (!net) {
        net = require('net');
      }

      socket = net.connect(options, onconnect);
    }
  }

  this._handle.socket = socket;
  socket._mqttSocket = this;

  socket.on('error', onerror);
  socket.on('data', ondata);
  socket.on('finish', onfinish);
}
util.inherits(MQTTClient, EventEmitter);

MQTTClient.prototype.end = function(force) {
  var handle = this._handle;

  handle.isConnected = false;

  if (force || handle.storageCount == 0) {
    handle.socket.end(native.disconnect());

    // Abort ongoing messages.
    clearInterval(this.timer);
    this.storage = null;
    this.storageCount = 0;
  }
};

MQTTClient.prototype.checkConnection = function() {
  if (!this._handle.isConnected) {
    throw new Error('MQTT client is not connected');
  }
};

MQTTClient.prototype.publish = function(topic, message, options, callback) {
  this.checkConnection();

  var handle = this._handle;

  // header bits: | 16 bit packet id | 4 bit PUBLISH header |
  var header = 0;
  var qos = 0;

  if (options) {
    if (options.retain) {
      header = 0x1;
    }

    qos = options.qos;

    if (qos !== 1 && qos !== 2) {
      qos = 0;
    }

    header |= (qos << 1);
  }

  if (qos > 0) {
    var packet_id = handle.getPacketId();
    header |= (packet_id << 4);

    var buffer = native.publish(topic, message, header);
    handle.write(buffer);

    // Set dup flag.
    buffer.writeUInt8(buffer.readUInt8(0) | 0x08, 0);

    var packet = handle.storage[packet_id];

    packet.packet = buffer;
    packet.callback = callback;
    return;
  }

  handle.write(native.publish(topic, message, header));

  if (typeof callback == 'function') {
    process.nextTick(callback);
  }
};

MQTTClient.prototype.subscribe = function(topic, options, callback) {
  this.checkConnection();

  var handle = this._handle;

  var packet_id = handle.getPacketId();

  // header bits: | 2 bit qos | 16 bit packet id |
  var header = packet_id;

  var qos = 0;

  if (options) {
    qos = options.qos;

    if (qos !== 1 || qos !== 2) {
      qos = 0;
    }

    header |= (qos << 16);
  }

  var buffer = native.subscribe(topic, header);

  handle.write(buffer);

  var packet = handle.storage[packet_id];

  packet.packet = buffer;
  packet.callback = callback;
};

MQTTClient.prototype.unsubscribe = function(topic, callback) {
  this.checkConnection();

  var handle = this._handle;

  var packet_id = handle.getPacketId();

  // header bits: | 16 bit packet id |
  var header = packet_id;

  var buffer = native.unsubscribe(topic, header);

  handle.write(buffer);

  var packet = handle.storage[packet_id];

  packet.packet = buffer;
  packet.callback = callback;
};

function onerror(error) {
  this._mqttSocket.emit('error', error);
}

function ondata(data) {
  native.MqttReceive(this._mqttSocket._handle, data);
}

function onfinish() {
  this._mqttSocket._handle.onEnd();
}

function storageTimerHit() {
  // this: MQTTHandle

  // eslint-disable-next-line guard-for-in
  for (var packet_id in this.storage) {
    var packet = this.storage[packet_id];

    packet.remainingTime--;

    if (packet.remainingTime <= 0) {
      this.releasePacket(packet_id, new Error('Undelivered message'));
      continue;
    }

    // Every 8 seconds, the message is retransmitted.
    if (!(packet.remainingTime & 0x7)) {
      this.write(packet.packet);
    }
  }

  if (this.storageCount == 0 && !this.isConnected) {
    // Graceful disconnect after all messages transmitted.
    this.socket.end(native.disconnect());

    clearInterval(this.timer);
    this.storage = null;
    return;
  }

  if (this.pingrespCounter > 0) {
    this.pingrespCounter--;

    if (this.pingrespCounter <= 0) {
      this.onEnd();
    }
  }

  this.keepaliveCounter++;

  if (this.keepaliveCounter >= this.keepalive) {
    this.write(native.ping());

    if (this.pingrespCounter == 0) {
      this.pingrespCounter = (this.keepalive + 5) * 3 >> 1;
    }
  }
}

/*
 * Returns an unique client ID based on current time.
 */
function defaultClientId() {
  return 'iotjs_mqtt_client_' + Date.now();
}

function connect(url, options, callback) {
  return new MQTTClient(url, options, callback);
}

exports.connect = connect;
