### Platform Support

The following shows Event module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| emitter.addListener | O | O | O |
| emitter.on | O | O | O |
| emitter.emit | O | O | O |
| emitter.once | O | O | O |
| emitter.addListener | O | O | O |
| emitter.removeListener | O | O | O |
| emitter.removeAllListener | O | O | O |


### Contents

- [Events](#events)
- [Class: EventEmitter](#class-eventemitter)
    - [Constructor](#constructor)
        - [`new EventEmitter()`](#new-eventemitter)
    - [Prototype Functions](#prototype-functions)
        - [`emitter.addListener(event, listener)`](#emitteraddlistenerevent-listener)
        - [`emitter.on(event, listener)`](#emitteronevent-listener)
        - [`emitter.emit(event[, args..])`](#emitteremitevent-args)
        - [`emitter.once(event, listener)`](#emitteronceevent-listener)
        - [`emitter.removeListener(event, listener)`](#emitterremovelistenerevent-listener)
        - [`emitter.removeAllListener([event])`](#emitterremovealllistenerevent)


# Events


# Class: EventEmitter

The application programming model of IoT.js is based on event-driven programming. Thus many objects in IoT.js emit events. `events.EventEmitter` plays a role as base class for such objects.

User application would not directly create an instance of `EventEmitter` since `EventEmitter` is an abstract trait which defines its behavior and grants to sub-classes.


## Constructor


### `new EventEmitter()`
Returns a new EventEmitter object.


## Prototype Functions


### `emitter.addListener(event, listener)`
### `emitter.on(event, listener)`
* `event <String>`
* `listener <Function([args..])>`
  * `...args <any>`
* Returns: `<EventEmitter>`

Adds `listener` to the end of list of event listeners for `event`.

**Example**

```js
var EventEmitter = require('events').EventEmitter;
var emitter = new EventEmitter();

emitter.addListener('event', function() {
  console.log('emit event');
});
```

### `emitter.emit(event[, args..])`
* `event <String>`
* `...args <any>`
* Returns: `<Boolean>`

Invokes each of listener with supplied arguments.

Returns `true` if there were listeners, `false` otherwise.

**Example**

```js
var EventEmitter = require('events').EventEmitter;
var emitter = new EventEmitter();

emitter.addListener('event', function() {
  console.log('emit event');
});

emitter.emit('event');
```

### `emitter.once(event, listener)`
* `event <String>`
* `listener <Function([args..])>`
  * `...args <any>`
* Returns: `<EventEmitter>`

Adds `listener` for one time listener for `event`.

The listener will be invoked at the next event and removed.

**Example**

``` js
var EventEmitter = require('events').EventEmitter;
var emitter = new EventEmitter();
var onceCnt = 0;

emitter.addListener('event', function() {
  console.log('emit event');
});
emitter('once', function() {
  onceCnt += 1;
});

emitter.emit('once');
assert.equal(onceCnt, 1);
emitter.emit('once');
assert.equal(onceCnt, 1);
```

### `emitter.removeListener(event, listener)`
* `event <String>`
* `listener <Function([args..])>`
  * `...args <any>`
* Returns: `<EventEmitter>`

Removes listener from the list of event listeners.

If you add the same listener multiple times, this removes only one instance of them.

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

### `emitter.removeAllListener([event])`
* `event <String>`
* Returns: `<EventEmitter>`

Removes all listeners.

If `event` was specified, it only removes listeners for that event.
