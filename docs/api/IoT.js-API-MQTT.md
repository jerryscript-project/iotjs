### Platform Support

The following chart shows the availability of each TLS module API function on each platform.

|  | Linux<br/>(Ubuntu) | Tizen<br/>(Raspberry Pi) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: | :---: |
| mqtt.publish  | O | X | X | X | X | X |
| mqtt.subscribe | O | X | X | X | X | X |
| mqtt.unsubscribe | X | X | X | X | X | X |
| mqtt.ping | O | X | X | X | X | X |
| mqtt.connect | O | X | X | X | X | X |

# MQTT

MQTT is a machine-to-machine (M2M)/"Internet of Things" connectivity protocol. It was designed as an extremely lightweight publish/subscribe messaging transport. It is useful for connections with remote locations where a small code footprint is required and/or network bandwidth is at a premium.

### QoS
The QoS level can be 0, 1 or 2.
- Level 0 means the packet arrives at most once.
- Level 1 means the packet arrives at least once (duplications might occur, it is the user's responsibility to take care of them).
- Level 2 means that the package is delivered exactly once.

### Topic seperating and wildcarding
Topics can be wildcarded and they also can be structured into multiple levels. These levels are divided by the `/` sign (eg. `iotjs/jerryscript/jerry-core`). There are multiple wildcarding possibilities:
 - `Multi-level wildcard` The `#` sign is a wildcard character that matches any number of levels within a topic. This character MUST be the last character in a topic name. Typing `iotjs/#` means the client subscribes to anything that is under the `iotjs` topic.
 - `Single-level wildcard` The `+` sign is a wildcard character that matches only one topic level. It can be used more at than one level in the topic name. It MUST be used so it occupies an entire level of the name. Typing `iotjs/+/jerry-core` subscribes you to `jerry-core` topic.
 - Topics that are beginning with `$` can not be matched with wildcard filters such as `#` or `+`. Subscriptions with wildcards to these topics means that they receive no data at all.

## Class: MQTTClient
The `MQTTClient` can subscribe or publish data to a broker. It sends data over a `net.socket`.

### mqtt.connect(options, callback)
- `options` {Object}
    - `clientId` {Buffer | string} Optional. The broker identifies each client by its `clientId`. If not specified, a randomly generated `clientId` is created.
    - `host` {Buffer | string} The address of the broker.
    - `port` {number} The port of the broker.
    - `socket` {net.Socket | TLSSocket} If a `TLSSocket` is given for secure communication it is the user's responsibility to connect it to establish the TLS connection first. Otherwise the client automatically connects the socket to the server.
    - `username` {Buffer | string} Optional. Use username when onnecting to a broker.
    - `password` {Buffer | string} Optional. Use password authentication when connecting to a broker.
    - `keepalive` {number} Keepalive time in seconds. If no data is sent on the connection in the given time window the broker disconnects the client.
    - `will` {boolean} Optional. If this flag is set to `true`, a `message` and a `topic` must follow with a QoS value between 0 and 2.
    - `qos` {number} If `will` is set to `true`, the message will be sent with the given QoS.
    - `topic` {Buffer | string} Only processed when `will` is set to `true`. The topic the `message` should be sent to.
    - `message` {Buffer | string} Only processed when `will` is set to `true`. The message to be sent to the broker when connecting.
- `callback` {function} The function will be executed when the client successfuly connected to the broker.

Returns with an MQTTClient object and starts connecting to a broker. Emits a `connect` event after the connection is completed.


**Example**
```js
var mqtt = require('mqtt');

var opts = {
    host: '127.0.0.1',
    port: 443,
    keepalive: 10,
    clientId: 'IoT.js Client',
}

var client = mqtt.getClient(opts);
client.connect(function () {
    client.disconnect();
});
```

### mqtt.disconnect()
Disconnects the client from the broker.

### mqtt.ping()
Sends a ping request to the server. If the server doesn't respond within 3 seconds, the client closes the connection. Emits a `pingresp` event if the server responded.

**Example**
```js
var mqtt = require('mqtt');

var opts = {
    host: '127.0.0.1',
    port: 443,
    keepalive: 10,
    clientId: 'IoT.js Client',
}

var client = mqtt.getClient(opts);
client.connect(function () {
    client.ping();
});

client.on('pingresp', function() {
  client.disconnect();
});
```

### mqtt.subscribe(options)
- `options` {Object}
    - `topic` {Buffer | string} The topic the client subscribes to.
    - `qos` {number} Optional. Defaults to 0.
    - `retain` {boolean} Optional. If retain is `true` the client receives the messages that were sent to the desired `topic` before it connected. Defaults to `false`.

The client subscribes to a given `topic`. If there are messages available on the `topic` the client emits a `data` event with the message received from the broker.

**Example**
```js
var mqtt = require('mqtt');

var opts = {
    host: '127.0.0.1',
    port: 443,
    keepalive: 10,
    clientId: 'IoT.js Client',
}

var subscribe_opts {
  topic: 'hello/#/iotjs',
  retain: false,
  qos: 2
}

var client = mqtt.getClient(opts);
client.connect(function () {
    client.subscribe(subscribe_opts);
});

client.on('data', function(data) {
  console.log('I received something: ' + data.toString());
});
```

### mqtt.unsubscribe(topic)
- `options` {Buffer | string} The topic to unsubscribe from.

Unsubscribes the client from a given topic. If QoS was turned on on the subscription the remaining packets will be sent by the server.


### mqtt.publish(options)
- `options` {Object}
    - `topic` {Buffer | string} The topic to send the `message` to.
    - `message` {Buffer | string} The message to be sent.
    - `qos` {number} Optional. Defaults to 0.
    - `retain` {boolean} Optional. If retain is `true` the broker stores the message for clients subscribing with retain `true` flag, therefore they can receive it later.

Publishes a `message` to the broker under the given `topic`.

**Example**
```js
var mqtt = require('mqtt');

var opts = {
    host: '127.0.0.1',
    port: 443,
    keepalive: 10,
    clientId: 'IoT.js Client',
}

var publish_opts {
  topic: 'hello/#/iotjs',
  message: 'MQTT now works!',
  retain: false,
  qos: 1
}

var client = mqtt.getClient(opts);
client.connect(function () {
    client.publish(publish_opts);
});
```

## Events
### `connect`
Emitted when the client successfully connects to a broker.

### `disconnect`
A `disconnect` event is emitted when the broker disconnects the client gracefully.

### `error`
If an error occured an `error` event is emitted with the error data.

### `message`
When data is received from the server a `message` event is emitted with a `data` object. It has the following properties:
   - `message`: The message the broker sent.
   - `topic`: The topic the message was sent from.
   - `qos`: The QoS level the message was sent with.
   - `packet_id`: The id of the packet if QoS was enabled.

### `pingresp`
Emitted when we receive a ping response from the server.

### `puback`
`puback` is emitted if the server has successfully received the QoS 1 packet sent with `publish`.

### `pubcomp`
If a QoS level 2 package has successfully arrived a `pubcomp` is emitted.

### `suback`
If a subscription was accepted by the broker to a topic, a `suback` event is emitted.
