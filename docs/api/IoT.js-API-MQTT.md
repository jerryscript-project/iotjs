### Platform Support

The following chart shows the availability of each MQTT module API function on each platform.

|  | Linux<br/>(Ubuntu) | Tizen<br/>(Raspberry Pi) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: | :---: |
| mqtt.connect | O | O | O | X | O |
| mqtt.end | O | O | O | X | O |
| mqtt.publish  | O | O | O | X | O |
| mqtt.subscribe | O | O | O | X | O |
| mqtt.unsubscribe | O | O | O | X | O |

# MQTT

MQTT is a machine-to-machine (M2M)/"Internet of Things" connectivity protocol. It was designed as an extremely lightweight publish/subscribe messaging transport. It is useful for connections with remote locations where a small code footprint is required and/or network bandwidth is at a premium.

### QoS
The QoS level can be 0, 1 or 2.
- Level 0 means the packet arrives at most once.
- Level 1 means the packet arrives at least once (duplications might occur, it is the user's responsibility to take care of them).
- Level 2 means that the package is delivered exactly once.

### Topic separating and wildcarding
Topics can be wildcarded and they also can be structured into multiple levels. These levels are divided by the `/` sign (eg. `iotjs/jerryscript/jerry-core`). There are multiple wildcarding possibilities:
 - `Multi-level wildcard` The `#` sign is a wildcard character that matches any number of levels within a topic. This character MUST be the last character in a topic name. Typing `iotjs/#` means the client subscribes to anything that is under the `iotjs` topic.
 - `Single-level wildcard` The `+` sign is a wildcard character that matches only one topic level. It can be used more at than one level in the topic name. It MUST be used so it occupies an entire level of the name. Typing `iotjs/+/jerry-core` subscribes you to `jerry-core` topic.
 - Topics that are beginning with `$` can not be matched with wildcard filters such as `#` or `+`. Subscriptions with wildcards to these topics means that they receive no data at all.

## Class: MQTTClient
The `MQTTClient` can subscribe or publish data to a broker. It sends data over a `net.socket`.

### mqtt.connect([url], [options], [callback])
- `url` {string} host name optionally prefixed by `mqtt://` or `mqtts://`.
- `options` {Object}
    - `clientId` {Buffer | string} Optional. The broker identifies each client by its `clientId`. If not specified, a randomly generated `clientId` is created.
    - `host` {Buffer | string} The address of the broker.
    - `port` {number} The port of the broker.
    - `socket` {net.Socket | TLSSocket} If a `TLSSocket` is given for secure communication it is the user's responsibility to connect it to establish the TLS connection first. Otherwise the client automatically connects the socket to the server.
    - `username` {Buffer | string} Optional. Use username when connecting to a broker.
    - `password` {Buffer | string} Optional. Use password authentication when connecting to a broker.
    - `keepalive` {number} Keepalive time in seconds. If no data is sent on the connection in the given time window the broker disconnects the client.
    - `will` {boolean} Optional. If this flag is set to `true`, a `message` and a `topic` must follow with a QoS value between 0 and 2.
    - `qos` {number} If `will` is set to `true`, the message will be sent with the given QoS.
    - `topic` {Buffer | string} Only processed when `will` is set to `true`. The topic the `message` should be sent to.
    - `message` {Buffer | string} Only processed when `will` is set to `true`. The message to be sent to the broker when connecting.
- `callback` {function} the function which will be executed when the client successfully connected to the broker.

Returns with an MQTTClient object and starts connecting to a broker. Emits a `connect` event after the connection is completed.


**Example**
```js
var mqtt = require('mqtt');

var opts = {
  port: 443,
  keepalive: 10,
  clientId: 'IoT.js Client',
}

var client = mqtt.connect('mqtt://127.0.0.1', opts, function () {
  client.end();
});
```

### mqtt.end([force])
- `force` {boolean} force network connection abort

Disconnects the client from the broker.

### mqtt.subscribe(topic, [options], [callback])
- `topic` {Buffer | string} topic to subscribe to
- `options` {Object}
    - `qos` {number} Optional. Defaults to 0.
    - `retain` {boolean} Optional. If retain is `true` the client receives the messages that were sent to the desired `topic` before it connected. Defaults to `false`.
- `callback` {function} the function which will be executed when the subscribe is completed.


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

var subscribe_opts = {
  retain: false,
  qos: 2
}

var client = mqtt.connect(opts, function () {
  client.subscribe('hello/#/iotjs', subscribe_opts, function(error) {
    if (error) {
      console.log('Subscribe is failed');
    } else {
      console.log('Subscribe is successfully completed');
    }
  });
});

client.on('message', function(data) {
  console.log('I received something: ' + data.message.toString());
});
```

### mqtt.unsubscribe(topic, [callback])
- `topic` {Buffer | string} topic to unsubscribe from
- `callback` {function} the function which will be executed when the unsubscribe is completed.

Unsubscribes the client from a given topic. If QoS was turned on on the subscription the remaining packets will be sent by the server.


### mqtt.publish(topic, message, [options], [callback])
- `topic` {Buffer | string} topic to publish to
- `message` {Buffer | string} message to send
- `options` {Object}
    - `qos` {number} Optional. Defaults to 0.
    - `retain` {boolean} Optional. If retain is `true` the broker stores the message for clients subscribing with retain `true` flag, therefore they can receive it later.
- `callback` {function} the function which will be executed when the publish is completed


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

var client = mqtt.connect(opts, function () {
  client.publish('hello/#/iotjs', 'Hello MQTT clients!', { qos:1 }, function() {
    console.log('Message has been published');
  });
});
```

## Events
### `connect`
Emitted when the client successfully connects to a broker.

### `disconnect`
A `disconnect` event is emitted when the broker disconnects the client gracefully.

### `error`
If an error occurred an `error` event is emitted with the error data.

### `message`
When data is received from the server a `message` event is emitted with a `data` object. It has the following properties:
   - `message`: The message the broker sent.
   - `topic`: The topic the message was sent from.
   - `qos`: The QoS level the message was sent with.
   - `packet_id`: The id of the packet if QoS was enabled.
