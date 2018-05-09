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

var mqtt = require('mqtt');
var assert = require('assert');

var connected = false;
var subscribed = false;
var pingresp = false;

var msg = 'hello iotjs';
var msg_received;

var subOpts = {
  topic: 'iotjs-test-topic',
};

var subClientOpts = {
  clientId: 'iotjs-mqtt-test-sub',
  host: 'test.mosquitto.org',
  port: 1883,
  keepalive: 30,
};

var subClient = mqtt.getClient(subClientOpts);
subClient.connect(function() {
  connected = true;

  subClient.on('pingresp', function() {
    pingresp = true;
    subClient.subscribe(subOpts);
  });

  subClient.on('suback', function() {
    subscribed = true;
    pubClient.publish(pubOpts);
  });

  subClient.on('message', function(data) {
    msg_received = data.message;
    subClient.disconnect();
    pubClient.disconnect();
  });

  subClient.ping();
});

var pubClientOpts = {
  clientId: 'iotjs-mqtt-test-pub',
  host: 'test.mosquitto.org',
  port: 1883,
  keepalive: 30,
};

var pubOpts = {
  topic: 'iotjs-test-topic',
  message: msg,
  qos: 1,
};

var pubClient = mqtt.getClient(pubClientOpts);
pubClient.connect();

process.on('exit', function() {
  assert.equal(connected, true);
  assert.equal(subscribed, true);
  assert.equal(pingresp, true);
  assert.equal(msg_received, msg);
});
