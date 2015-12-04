## process

`process` is unique global object can be accessed from anywhere.

### Methods

#### process.nextTick(callback)

* `callback: Funciton`

After current event loop finished, calls the `callback` function. The next tick callback will be called before any I/O events.