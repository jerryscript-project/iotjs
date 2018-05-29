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

var net = require('net');
var util = require('util');
var EventEmitter = require('events').EventEmitter;

var PacketTypeEnum = {
  PUBACK: 4,
  PUBREC: 5,
  PUBREL: 6,
  PUBCOMP: 7,
};

function MQTTHandle(client) {
  this.client = client;
  this.isConnected = false;
  this.package_id = 0;

  native.MqttInit(this);
}

MQTTHandle.prototype = {};

function MQTTClient(options, callback) {
  if (!(this instanceof MQTTClient)) {
    return new MQTTClient(options);
  }

  EventEmitter.call(this);

  if (util.isFunction(callback)) {
    this.on('connect', callback);
  }

  options = Object.create(options, {
    host: { value: options.host || '127.0.0.1'},
    port: { value: options.port || 8883 },
    qos: { value: options.qos || 0 },
    keepalive: { value: options.keepalive || 60 },
  });

  this._handle = new MQTTHandle(this);

  var socket;

  if (options.socket) {
    socket = options.socket;

    socket.write(native.connect(options));
  } else {
    socket = net.connect(options);

    var connectionMessage = native.connect(options);

    socket.on('connect', function() {
      this.write(connectionMessage);
    });
  }

  this._handle.socket = socket;
  socket._mqttSocket = this;

  socket.on('error', onerror);
  socket.on('data', ondata);
  socket.on('finish', onfinish);
}
util.inherits(MQTTClient, EventEmitter);

MQTTClient.prototype.end = function(error) {
  var handle = this._handle;

  handle.isConnected = false;

  if (error) {
    this.emit('error', error);
  }

  handle.socket.end(native.disconnect());
};

MQTTClient.prototype.publish = function(options) {
  if (!Buffer.isBuffer(options.message)) {
    options.message = new Buffer(options.message);
  }
  if (!Buffer.isBuffer(options.topic)) {
    options.topic = new Buffer(options.topic);
  }

  var handle = this._handle;

  if (util.isNumber(options.qos) && options.qos > 0) {
    options.packet_id = handle.package_id;
    handle.package_id++;

    var buffer = native.publish(options);
    handle.socket.write(buffer);

    var interval = setInterval(function() {
      handle.socket.write(buffer);
    }, 3000);

    this.on('puback', function() {
      clearInterval(interval);
    });
    this.on('pubrec', function() {
      clearInterval(interval);
    });

    return;
  }

  handle.socket.write(native.publish(options));
};

MQTTClient.prototype.subscribe = function(options) {
  if (!Buffer.isBuffer(options.topic)) {
    options.topic = new Buffer(options.topic);
  }

  this._handle.socket.write(native.subscribe(options));
};

MQTTClient.prototype.ping = function() {
  this._handle.socket.write(native.ping());
};

MQTTClient.prototype.unsubscribe = function(topic) {
  if (!Buffer.isBuffer(topic)) {
    topic = new Buffer(topic);
  }

  this._handle.socket.write(native.unsubscribe(topic));
};

function onerror(error) {
  this._mqttSocket.emit('error', error);
}

function ondata(data) {
  native.MqttReceive(this._mqttSocket._handle, data);
}

function onfinish() {
  this._mqttSocket._handle._isConnected = false;
  this._mqttSocket.emit('finish');
}

MQTTHandle.prototype.sendAck = function(type, packet_id) {
  this.socket.write(native.sendAck(type, packet_id));
};

MQTTHandle.prototype.onconnection = function() {
  this.client.emit('connect');
};

MQTTHandle.prototype.onEnd = function() {
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

MQTTHandle.prototype.onpingresp = function() {
  this.client.emit('pingresp');
};

MQTTHandle.prototype.onpuback = function(data) {
  /*
   * QoS level 1
   * Handle PUBACK package. If this package isn't arrived (properly),
   * we have to resend the last message.
   *
   * The 'data' contains the packet identifier.
   */
  this.client.emit('puback', data);
};

MQTTHandle.prototype.onpubcomp = function(data) {
  /*
   * Qos level 2
   * Handle PUBCOMP package. If this package is arrived, the sending process
   * is done.
   */
  this.client.emit('pubcomp', data);
};

MQTTHandle.prototype.onpubrec = function(data) {
  /*
   * Qos level 2
   * Handle PUBREC package. If this package is arrived, we have to send back
   * a PUBREL package to the server.
   */
  var jsref = this;

  this.sendAck(PacketTypeEnum.PUBREL, data);

  var interval = setInterval(function() {
    jsref.sendAck(PacketTypeEnum.PUBREL, data);
  }, 3000);

  jsref.on('pubcomp', function() {
    clearInterval(interval);
  });

  jsref.emit('pubrec', data);
};

MQTTHandle.prototype.onpubrel = function(data) {
  /*
   * Qos level 2
   * Handle PUBREL package. If this package is arrived, we have to send back
   * a PUBCOMP package to the server.
   */
  this.sendAck(PacketTypeEnum.PUBCOMP, data);
};

MQTTHandle.prototype.onsuback = function(data) {
  /*
   * Successful subscription, the client will get messages from the requested
   * topic. The granted QoS is given in data.
   */
  this.client.emit('suback', data);
};

MQTTHandle.prototype.onunsuback = function(data) {
  /*
   * Successful unsubscription, the client will not get messages from
   * the requested topic in the future
   */
  this.client.emit('unsuback', data);
};

/*
 * Returns an unique client ID based on current time.
 */
function defaultClientId() {
  return 'iotjs_mqtt_client_' + Date.now();
}

function connect(connectOptions, callback) {
  if (util.isUndefined(connectOptions.clientId)) {
    connectOptions.clientId = defaultClientId();
  }
  if (!Buffer.isBuffer(connectOptions.clientId)) {
    connectOptions.clientId =
        new Buffer(connectOptions.clientId.toString());
  }
  if (!util.isUndefined(connectOptions.username) &&
      !Buffer.isBuffer(connectOptions.username)) {
    connectOptions.username = new Buffer(connectOptions.username.toString());
  }
  if (!util.isUndefined(connectOptions.password) &&
      !Buffer.isBuffer(connectOptions.password)) {
    connectOptions.password = new Buffer(connectOptions.password.toString());
  }
  if (connectOptions.will) {
    if (util.isUndefined(connectOptions.topic) ||
        util.isUndefined(connectOptions.message) ||
        connectOptions.qos < 0 || connectOptions.qos > 2) {
      throw new Error('Wrong options given! Please refer to the documentation');
    }

    if (!util.isUndefined(connectOptions.topic) &&
        !Buffer.isBuffer(connectOptions.topic)) {
      connectOptions.topic = new Buffer(connectOptions.topic.toString());
    }
    if (!util.isUndefined(connectOptions.message) &&
        !Buffer.isBuffer(connectOptions.message)) {
      connectOptions.message = new Buffer(connectOptions.message.toString());
    }
  }

  return new MQTTClient(connectOptions, callback);
}

exports.connect = connect;
