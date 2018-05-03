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
var tls = require('tls');

util.inherits(MQTTClient, EventEmitter);

var PacketTypeEnum = {
  PUBACK: 4,
  PUBREC: 5,
  PUBREL: 6,
  PUBCOMP: 7,
};

function MQTTClient(options) {
  if (!(this instanceof MQTTClient)) {
    return new MQTTClient(options);
  }

  EventEmitter.call(this);

  this._clientOptions = Object.create(options, {
    host: { value: options.host || '127.0.0.1'},
    port: { value: options.port || 8883 },
    qos: { value: options.qos || 0 },
    keepalive: { value: options.keepalive || 60 },
  });

  this._socket = options.socket || new net.Socket();
  this._socket.on('error', onerror);

  this._isConnected = false;
  this._reconnecting = false;
  this._package_id = 0;

  // Set the native callbacks
  this._onconnect = onconnect;
  this._ondisconnect = ondisconnect;
  this._onmessage = onmessage;
  this._onpingresp = onpingresp;
  this._onpuback = onpuback;
  this._onpubcomp = onpubcomp;
  this._onpubrec = onpubrec;
  this._onpubrel = onpubrel;
  this._onsuback = onsuback;

  native.MqttInit(this);
}

/*
 * Connect to an MQTT broker.
 */
function MqttConnect(socket, options) {
  var buff = native.connect(options);
  socket.write(buff);
}

MQTTClient.prototype.connect = function(callback) {
  this._clientOptions.cb = callback;
  var jsref = this;
  if (this._socket instanceof net.Socket) {
    this._socket = net.connect(this._clientOptions);
    this._socket.on('connect', function() {
      MqttConnect(this, jsref._clientOptions);
    });
  } else if ('TLSSocket' in tls && this._socket instanceof tls.TLSSocket) {
    MqttConnect(this._socket, jsref._clientOptions);
  }

  if (util.isFunction(callback)) {
    this.on('connect', callback);
  }

  this._socket.on('data', function(data) {
    ondata(jsref, data);
  });
  this._socket.on('error', function(e) {
    jsref.emit('error', e);
  });
  this._socket.on('end', function() {
    ondisconnect(jsref);
  });
};

MQTTClient.prototype.disconnect = function(error) {
  if (error) {
    this.emit('error', error);
  }

  this._isConnected = false;
  var buf = native.disconnect();
  this._socket.write(buf);
  this._socket.end();
};

MQTTClient.prototype.reconnect = function() {
  if (this._reconnecting) {
    return;
  }

  this.disconnect();
  setTimeout(this.connect, this._options.reconnectPeriod);
};

MQTTClient.prototype.publish = function(options) {
  if (!Buffer.isBuffer(options.message)) {
    options.message = new Buffer(options.message);
  }
  if (!Buffer.isBuffer(options.topic)) {
    options.topic = new Buffer(options.topic);
  }

  if (util.isNumber(options.qos) && options.qos > 0) {
    options.packet_id = this._package_id;
    this._package_id++;

    var buffer = native.publish(options);
    this._socket.write(buffer);

    var self = this;

    var interval = setInterval(function() {
      self._socket.write(buffer);
    }, 3000);

    this.on('puback', function() {
      clearInterval(interval);
    });
    this.on('pubrec', function() {
      clearInterval(interval);
    });

    return;
  }

  this._socket.write(native.publish(options));
};

MQTTClient.prototype.subscribe = function(options) {
  if (!Buffer.isBuffer(options.topic)) {
    options.topic = new Buffer(options.topic);
  }

  var buff = native.subscribe(options);
  this._socket.write(buff);
};

MQTTClient.prototype.ping = function() {
  var buff = native.ping();
  this._socket.write(buff);
};

MQTTClient.prototype.unsubscribe = function(topic) {
  if (!Buffer.isBuffer(topic)) {
    topic = new Buffer(topic);
  }

  var buf = native.unsubscribe(topic);
  this._socket.write(buf);
};

MQTTClient.prototype.sendAcknowledge = function(options) {
  var buff = native.sendAck(options);
  this._socket.write(buff);
};

function onpubcomp(jsref, data) {
  /*
   * Qos level 2
   * Handle PUBCOMP package. If this package is arrived, the sending process
   * is done.
   */
  jsref.emit('pubcomp', data);
}

function onpubrel(jsref, data) {
  /*
   * Qos level 2
   * Handle PUBREL package. If this package is arrived, we have to send back
   * a PUBCOMP package to the server.
   */
  var options = {
    type: PacketTypeEnum.PUBCOMP,
    packet_id: data,
  };

  jsref.sendAcknowledge(options);
}

function ondata(jsref, data) {
  native.MqttReceive(jsref, data);
}

function onconnect(jsref) {
  jsref.emit('connect');
}

function onpingresp(jsref) {
  jsref.emit('pingresp');
}

function onmessage(jsref, message, topic, qos, packet_id) {
  var data = {
    message: message,
    topic: topic,
    qos: qos,
    packet_id: packet_id,
  };

  if (qos == 1) {
    var opts = {
      type: PacketTypeEnum.PUBACK,
      packet_id: packet_id,
    };

    jsref.sendAcknowledge(opts);
  } else if (qos == 2) {
    var options = {
      type: PacketTypeEnum.PUBREC,
      packet_id: packet_id,
    };
    jsref.sendAcknowledge(options);
  }

  jsref.emit('message', data);
}

function ondisconnect(jsref, message) {
  jsref._isConnected = false;
  jsref.emit('disconnect', message);
}

function onpuback(jsref, data) {
  /*
   * QoS level 1
   * Handle PUBACK package. If this package isn't arrived (properly),
   * we have to resend the last message.
   *
   * The 'data' contains the packet identifier.
   */

  jsref.emit('puback', data);
}

function onpubrec(jsref, data) {
  /*
   * Qos level 2
   * Handle PUBREC package. If this package is arrived, we have to send back
   * a PUBREL package to the server.
   */
  var options = {
    type: PacketTypeEnum.PUBREL,
    packet_id: data,
  };

  jsref.sendAcknowledge(options);

  var interval = setInterval(function() {
    jsref.sendAcknowledge(options);
  }, 3000);

  jsref.on('pubcomp', function() {
    clearInterval(interval);
  });

  jsref.emit('pubrec', data);
}

function onsuback(jsref, data) {
  /*
   * Successful subscription, the client will get messages from the requested
   * topic. The granted QoS is given in data.
   */
   jsref.emit('suback', data);
}

function onerror(jsref, error) {
  jsref.emit('error', error);
}

/*
 * Returns an unique client ID based on current time.
 */
function defaultClientId() {
  return 'iotjs_mqtt_client_' + Date.now();
}

function getClient(connectOptions) {
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
  return new MQTTClient(connectOptions);
}

exports.getClient = getClient;
