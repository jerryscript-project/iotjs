### Platform Support

The following shows Event module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: |
| emitter.addListener | O | O | O | - |
| emitter.on | O | O | O | - |
| emitter.emit | O | O | O | - |
| emitter.once | O | O | O | - |
| emitter.removeListener | O | O | O | - |
| emitter.removeAllListeners | O | O | O | - |


# Events

IoT.js is based on event-driven programming where objects (called "emitters") periodically emit named events.

# Class: EventEmitter

The `events.EventEmitter` plays a role as base class for "emitters".
User application would not directly creates an instance of `EventEmitter` since `EventEmitter` is an abstract trait which defines its behavior and grants to sub-classes.

### new EventEmitter()
* Returns {events.EventEmitter}.

Returns with a new EventEmitter object.


**Example**

```js

var EventEmitter = require('events').EventEmitter;

var emitter = new EventEmitter();

```


### emitter.addListener(event, listener)
* `event` {string} The name of the event.
* `listener` {Function} The callback function.
  * `args` {any}.
* Returns `emitter` {events.EventEmitter}.

It is an alias for `emitter.on(eventName, listener)`.

Adds the `listener` callback function to the end of the listener's list for the given `event`. No checks are made to see if the `listener` has already been added.
In case of multiple calls the `listener` will be added and called multiple times.

**Example**

```js

var EventEmitter = require('events').EventEmitter;

var emitter = new EventEmitter();

var eventSequence = '';

var listener1 = function() {
  eventSequence += '2';
};

emitter.addListener('test', listener1);
emitter.addListener('test', listener1);
emitter.emit('test');

console.log(eventSequence); // prints '22'

```

### emitter.on(event, listener)
* `event` {string} The name of the event.
* `listener` {Function} The callback function.
  * `args` {any}.
* Returns `emitter` {events.EventEmitter}.

Adds the `listener` callback function to the end of the listener's list for the given `event`. No checks are made to see if the `listener` has already been added.
In case of multiple calls the `listener` will be added and called multiple times.

**Example**

```js

var EventEmitter = require('events').EventEmitter;
var emitter = new EventEmitter();

emitter.on('event', function() {
  console.log('emit event');
});

emitter.emit('event');

```

### emitter.emit(event[, args..])
* `event` {string} The name of the event.
  * `args` {any}.
* Returns {boolean}.

Synchronously calls each of the listeners registered for the `event`, in the order they were registered, passing the supplied arguments to each.

Returns true if the event had listeners, false otherwise.

**Example**

```js

var EventEmitter = require('events').EventEmitter;
var emitter = new EventEmitter();

emitter.addListener('event', function() {
  console.log('emit event');
});

emitter.emit('event'); // true

emitter.emit('not_an_event'); // false

```

### emitter.once(event, listener)
* `event` {string} The name of the event.
* `listener` {Function} The callback function.
  * `args` {any}.
* Returns `emitter` {events.EventEmitter}.

Adds the `listener` as a one time listener for the `event`.

Using this method, it is possible to register a listener that is called at most once for a particular `event`.
The listener will be invoked only once, when the first `event` is emitted.

**Example**

``` js

var EventEmitter = require('events').EventEmitter;
var assert = require('assert');

var emitter = new EventEmitter();

var onceCnt = 0;

emitter.once('once', function() {
  onceCnt += 1;
});

assert.equal(onceCnt, 0);
emitter.emit('once');
assert.equal(onceCnt, 1);
emitter.emit('once');
assert.equal(onceCnt, 1);

```

### emitter.removeListener(event, listener)
* `event` {string} The name of the event.
* `listener` {Function} The callback function.
  * `args` {any}.
* Returns `emitter` {events.EventEmitter}.

Removes `listener` from the list of event listeners.

If you add the same `listener` multiple times, this removes only one instance of them.

**Example**

```js

var EventEmitter = require('events').EventEmitter;
var emitter = new EventEmitter();

var listener = function() {
    console.log('listener');
};

emitter.addListener('event', listener);
emitter.removeListener('event', listener);

```

### emitter.removeAllListeners([event])
* `event` {string} The name of the event.
* Returns `emitter` {events.EventEmitter}.

Removes all listeners.

If `event` was specified, it only removes the listeners for that event.

**Example**

``` js

var EventEmitter = require('events').EventEmitter;

var emitter = new EventEmitter();

function removableListener() {
  console.log("listener called");
}

emitter.addListener('event1', removableListener);

emitter.addListener('event2', removableListener);
emitter.addListener('event2', removableListener);

emitter.addListener('event3', removableListener);


emitter.removeAllListeners('event2');

var res = emitter.emit('event2'); // res == false

res = emitter.emit('event1'); // res == true, prints "listener called"

emitter.removeAllListeners();

res = emitter.emit('event1'); // res == false
res = emitter.emit('event3'); // res == false

```
