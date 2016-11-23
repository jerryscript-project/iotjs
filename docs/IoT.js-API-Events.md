The application programming model of IoT.js is based on event-driven programming. Thus many objects in IoT.js emit events. `events.EventEmitter` plays a role as base class for such objects.


## Class: EventEmitter
User application would not directly create an instance of `EventEmitter` since `EventEmitter` is an abstract trait which defines its behavior and grants to sub-classes.


### Methods


#### emitter.addListener(event, listener)
#### emitter.on(event, listener)
* `event: String`
* `listener: Function([args..])`
* Returns emitter

Adds `listener` to the end of list of event listeners for `event`.


#### emitter.emit(event[, arg1[, arg2[...]]])
* `event: String`
* Returns `Boolean`

Invokes each of listener with supplied arguments.

Returns `true` if there were listeners, `false` otherwise.


#### emitter.once(event, listener)
* `event: String`
* `listener: Function([args..])`
* Returns emitter

Adds `listener` for one time listener for `event`.

The listener will be invoked at the next event and removed.


#### emitter.removeListener(event, listener)
* `event: String`
* `listener: Function([args..])`
* Returns emitter

Removes listener from the list of event listeners.

If you add the same listener multiple times, this removes only one instance of them.


#### emitter.removeAllListener([event])
* `event: String`
* Returns emitter

Removes all listeners.

If `event` was specified, it only removes listeners for that event.
