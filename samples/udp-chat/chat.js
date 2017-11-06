/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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
/**
 * Description:
 *
 * Runs chat-bots which talk to each other using UDP protocol. The first
 * instance will create a server node and the other will connect to it
 * automatically
 *
 * Usage:
 *
 * This example requires multiple iotjs instances on different machines in the
 * same LAN/WIFI network. Each machine should run:
 *
 * $ iotjs chat.js
 *
 * This will handle everything and the "chat" activity will be printed on
 * the console
 */

var dgram = require('dgram'),
  log_enabled = true,
  socket = null,
  mode = 'client', // client / server
  messages = [ // some sentences to send over udp
    'Hello! How are you?',
    'The wether was great last weekend, what did you do?',
    'Last weekend I was trekking in the mountains, it was great.',
    'How\'s the family?',
    'My family is great.',
    'I have watched a great film yesterday.',
    'I have to go to the dentist.',
    'What\'s on your mind?',
    'It\'s getting late.',
    'You can do anything you want.',
    'I love camping in the woods',
    'Do you like rock music?',
    'I like pop music.',
    'I would really like to spend some time with you.',
    'My dad owns a radio store.',
    'I had great success with building my business'
  ],
  names = [ // few available nicknames
    'phil',
    'tom',
    'kate',
    'john',
    'george'
  ],
  nickname = '',
  remote = {},
  clients = [],
  MSG = {
    INQUIRE_SERVER: 'is_anyone_here',
    INQUIRE_SERVER_ADDR: 'i_am_here_dave',
    JOIN: 'join',
    CHAT: 'chat'
  },
  joined = false,
  PORT = 9292,
  bcastTimer = null,
  converseTimer = null,
  CONVERSE_INTERVAL = (1 + (Math.random() * 3)) * 1000, // between 1 and 3
  BCAST_TTL = 1000, // 1s wait for response
  BCAST_ADDR = '255.255.255.255';

// log only if log_enabled flag is set to true
function log(/*...args*/) {
  if (log_enabled) {
    console.log.apply(console, [].slice.call(arguments));
  }
}

// return a random sentence
function randomMessage() {
  return messages[(Math.random() * messages.length) | 0];
}

// return a random nickname with random "year" suffix
function randomNickname() {
  var name = names[(Math.random() * names.length) | 0];
  return name + '_' + (1980 + ((Math.random() * 30) | 0));
}

// wraps arguments to JSON string format
function wrapToString(/*...args*/) {
  return JSON.stringify([].slice.call(arguments));
}

// closes socket and releases timers
function cleanup() {
  if (socket) {
    socket.close(function (err) {
      if (err) {
        log('ERROR: could not close server: ' + err);
      } else {
        log('INFO: server closed');
      }
    });
  }

  if (converseTimer) {
    clearInterval(converseTimer);
  }

  if (bcastTimer) {
    clearTimeout(bcastTimer);
  }
}

// sends a random message to udp server/clients
function converse() {
  var message = randomMessage(),
    msg = new Buffer(wrapToString(MSG.CHAT, nickname, message));

  if (mode === 'server') {
    console.log(nickname + ': ' + message); // log my messages
    forwardMessageToClients(msg);
  } else {
    socket.send(
      msg,
      0,
      msg.length,
      remote.port,
      remote.address,
      function (err) {
        if (err) {
          log('ERROR: could not send message: ' + err);
        }
    });
  }
}

// set server mode if no udp inquire was received
function bcastTimeoutHandle() {
  mode = 'server';
  log('INFO: nobody replied, starting server mode');
}

// send join message to server and generate nickname
function join() {
  var message = new Buffer(wrapToString(MSG.JOIN));

  nickname = randomNickname();

  socket.send(
    message,
    0,
    message.length,
    remote.port,
    remote.address,
    function (err) {
    if (err) {
      log('ERROR: could not join: ' + err);
    } else {
      log('INFO: joined!');
      joined = true;
      converseTimer = setInterval(converse, CONVERSE_INTERVAL);
    }
  });
}

// sends supplied message to connected clients
function forwardMessageToClients(message) {
  var i = 0,
    l = clients.length;

  for (; i < l; ++i) {
    socket.send(
      message,
      0,
      message.length,
      clients[i].port,
      clients[i].address,
      function (err) {
      if (err) {
        log('ERROR: could not send data to client: ' + err);
      }
    });
  }
}

// handles incomming UDP data
function handleServerMessage(data, rinfo) {
  var message = JSON.parse(data.toString());

  switch (message[0]) {
    case MSG.INQUIRE_SERVER: // when somebody asks for the server addres
      if (mode === 'server') {
        log('INFO: host inquiry: ' + rinfo.address + ':' + rinfo.port);
        message = new Buffer(wrapToString(MSG.INQUIRE_SERVER_ADDR));
        socket.send(
          message,
          0,
          message.length,
          rinfo.port,
          rinfo.address,
          function (err) {
          if (err) {
            log('ERROR: could not respond to inquiry: ' + err);
          } else {
            log('INFO: responded');
          }
        });
      }
      break;
    case MSG.INQUIRE_SERVER_ADDR: // response with server address
      if (mode === 'client' && !joined) {
        remote.port = rinfo.port;
        remote.address = rinfo.address;
        clearTimeout(bcastTimer);
        join();
      }
      break;
    case MSG.JOIN: // when a host joins server chat
      log('INFO: host joining: ' + rinfo.address + ':' + rinfo.port);
      clients.push(rinfo);
      if (!converseTimer) {
        nickname = randomNickname();
        converseTimer = setInterval(converse, CONVERSE_INTERVAL);
      }
      break;
    case MSG.CHAT: // plain chat messages
      console.log(message[1] + ': ' + message[2]); // console here
                                                   // not log wrap
      if (mode === 'server') {
        forwardMessageToClients(data);
      }
      break;
    default: // everything other, should never run
      log('INFO: i do not understand: ' + data.toString());
      break;
  }
}


// check if anyone else is server
log('INFO: looking up server');
socket = dgram.createSocket({type: 'udp4'});
socket.on('message', handleServerMessage);
socket.bind(PORT, function (err) {
  var message = new Buffer(wrapToString(MSG.INQUIRE_SERVER));
  if (err) {
    log('ERROR: could not bind to server port: ' + err);
  } else {
    bcastTimer = setTimeout(bcastTimeoutHandle, BCAST_TTL);

    // first try to find out if any server is available in the network
    socket.setBroadcast(true);
    socket.send(
      message,
      0,
      message.length,
      PORT,
      BCAST_ADDR,
      function (err) {
      if (err) {
        log('ERROR: could not send broadcast message: ' + err);
      }
    });
  }
});

process.on('exit', cleanup);
