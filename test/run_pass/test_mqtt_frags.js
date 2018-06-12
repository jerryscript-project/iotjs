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

var stream = require('stream');
var mqtt = require('mqtt');
var assert = require('assert');
var fs = require('fs');

var duplex = new stream.Duplex();

var connect_state = 0;
var recv_count = 0;
var fragment_index = 0;

// Message we send is always the same, but it is fragmented in different ways
var fragments = [
  // Basic message:
  '3014000d67656e6572616c2f746f70696368656c6c6f',

  // One message in three fragments
  '30',
  '14',
  '000d67656e6572616c2f746f70696368656c6c6f',

  // One message in four fragments
  '30',
  '14',
  '00',
  '0d67656e6572616c2f746f70696368656c6c6f',

  // One message in five fragments
  '30',
  '14',
  '00',
  '0d67656e6572616c2f746f70696368656c6c',
  '6f',

  // Two connected messages
  '3014000d67656e6572616c2f746f70696368656c6c6f' +
  '3014000d67656e6572616c2f746f70696368656c6c6f',

  // Two messages in three fragments
  '3014000d67656e6572616c2f74',
  '6f70696368656c6c6f3014000d67656e65',
  '72616c2f746f70696368656c6c6f',

  // Two messages in three fragments
  '3014000d67656e6572616c2f746f70696368656c6c6f30',
  '14',
  '000d67656e6572616c2f746f70696368656c6c6f',

  // A 132 byte long message
  '30',
  '87',
  '01',
  '000d67656e6572616c2f746f706963',
  '68656c6c6f68656c6c6f68656c6c6f68656c6c6f',
  '68656c6c6f68656c6c6f68656c6c6f68656c6c6f' +
  '68656c6c6f68656c6c6f68656c6c6f68656c6c6f' +
  '68656c6c6f68656c6c6f68656c6c6f68656c6c6f',
  '68656c6c6f68656c6c6f68656c6c6f68656c6c6f' +
  '68656c6c6f68656c6c6f68656c6c6f68656c6c6f'
];

function send_fragment() {
  duplex.push(new Buffer(fragments[fragment_index], 'hex'));

  if (++fragment_index < fragments.length) {
    process.nextTick(send_fragment);
  } else {
    duplex.end();
  }
}

duplex._write = function(chunk, callback, onwrite) {
  onwrite();

  switch (connect_state) {
  case 0:
    assert.equal(chunk.toString('hex'),
                 '100f00044d5154540402001e0003636c69');

    process.nextTick(function() {
      duplex.push(new Buffer('20020000', 'hex'));
    });
    break;

  case 1:
    assert.equal(chunk.toString('hex'),
                 '82120000000d67656e6572616c2f746f70696300');

    process.nextTick(function() {
      duplex.push(new Buffer('9003000002', 'hex'));
      process.nextTick(send_fragment);
    });
    break;

  default:
    throw new RangeError("Unknown connection state")
    break;
  }

  connect_state++;
};

duplex._readyToWrite();

var mqtt_client = mqtt.connect({
  clientId: 'cli',
  keepalive: 30,
  socket: duplex,
}, function() {
  /* Just subscribe a random topic. */
  mqtt_client.subscribe('general/topic');
});

mqtt_client.on('message', function(data) {
  if (recv_count < 10) {
    assert.equal(data.message.toString(), 'hello');
  } else {
    var str = '';
    for (var i = 0; i < 24; i++)
        str += 'hello';
    assert.equal(data.message.toString(), str);
  }

  recv_count++;
});

mqtt_client.on('finish', function(data) {
  assert.equal(recv_count, 11);
});
