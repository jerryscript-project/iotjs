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

// The number of the variable is the qos level (0-2)

var msg0 = 'hello iotjs 1';
var msg1 = 'hello iotjs 2';
var msg2 = 'hello iotjs 3';

var published0 = false;
var published1 = false;
var published2 = false;

var received0 = false;
var received1 = false;
var received2 = false;

var subClientOpts = {
  clientId: 'iotjs-mqtt-test-sub',
  host: 'test.mosquitto.org',
  port: 1883,
  keepalive: 30,
};

var pubClientOpts = {
  clientId: 'iotjs-mqtt-test-pub',
  port: 1883,
  keepalive: 30
};

var pubClient;

var subClient = mqtt.connect(subClientOpts, function() {
  connected = true;

  subClient.subscribe('iotjs-test-topic', { qos:2 }, function() {
    subscribed = true;

    pubClient = mqtt.connect('test.mosquitto.org', pubClientOpts, function() {
      pubClient.publish('iotjs-test-topic', msg0, { qos:0 }, function() {
        published0 = true;
      });

      pubClient.publish('iotjs-test-topic', msg1, { qos:1 }, function() {
        published1 = true;
      });

      pubClient.publish('iotjs-test-topic', msg2, { qos:2 }, function() {
        published2 = true;
      });
    });
  });

  subClient.on('message', function(data) {
    var str = data.message.toString();

    if (str == msg0) {
      received0 = true;
    }

    if (str == msg1) {
      received1 = true;
    }

    if (str == msg2) {
      received2 = true;
    }

    if (received0 && received1 && received2) {
      subClient.end();
      pubClient.end();
    }
  });
});

process.on('exit', function() {
  assert.equal(connected, true);
  assert.equal(subscribed, true);
  assert.equal(published0, true);
  assert.equal(published1, true);
  assert.equal(published2, true);
  assert.equal(received0, true);
  assert.equal(received1, true);
  assert.equal(received2, true);
});
