## This is a sample API reference. Please use this as a guideline to write your module's API references.

- If you have any questions about this guide, please let us know as an issue.
- `Markdown Example` is added to help understanding, and you can ignore it when writing the actual document.
- The document contents should be in order of `"Module"`, `"Module Function"`, `"Class"`, `"Constructor"`, `"Properties"`, `"Static Function"`, `"Prototype Functions"`, and `"Events"`. If the content does not exist, it can be omitted.
- `Module Functions` are what you can directly invoke without an instance of a certain Class. E.g) net.connect.
- In case of `experimental` module, it's required to explicitly indicate that the features are experimental. Please put the caution below to the beginning of the document.
> :exclamation: This document describes an experimental feature and considerations. Please be aware that every experimental feature may change, be broken, or be removed in the future without any notice.

***

### Platform Support

The following shows `{Your_module_name}` module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| {class_name}.{functionName1} | O | O | O |
| {class_name}.{functionName2} | O | O | O |
| {class_name}.{functionName3} | O | O | O |

# {Your_module_name}

- Write a brief description of this module here.
- The first character of the title must start with an `Uppercase letter`.

#### Markdown Example

```
# Timer

The timer module exposes a global API for scheduling functions to be called at some future period of time.
Because the timer functions are globals, there is no need to call require('timers') to use the API.
```

## {your_module_name}.{your_module_function_name}([{argument_name}])
* `{argument_name}` {{Argument_type}} {more information} **Default:** `{defalut_value}`
* Returns: {{return_type}} {more information}

- Write a description of this function here.
- The first character of Module in the title must start with a `lowercase letter`.
- The other rules are the same as mentioned before.

**Example**
```
  Write a sample usage for this API if needed.
```

#### Markdown Example
```
### net.connect(port[, host][, connectListener])
* `port` {number} Port the client should connect to.
* `host` {string} Host the client should connect to. **Default:** `localhost`.
* `connectListener` {Function} Listener for the `'connect'` event.
* Returns {net.Socket}.

Creates a new `net.Socket` and automatically connects to the supplied `port` and `host`.
If host is omitted, `localhost` will be assumed.
The `connectListener` is automatically registered as a `'connect'` event listener.
```

## Class: {Your_class_name}

- Write a brief description of this class here.
- The first character of the title must start with an `Uppercase letter`.
- While you are writing this description, if you need to write down module / class / function / event name, arguments, or type which you already mentioned, then enclose the keyword in single-quotation. This rule applies to other items as well.

    E.g) The given `callback` is called every `delay` milliseconds. If it's not a function, a `TypeError` will be thrown.

### new {Your_class_name}([{argument_name}])
* `{argument_name}` {{Argument_type}} {more information} **Default:** `{defalut_value}`
* Returns: {{return_type}} {more information}

Notice that every argument name of API and defalut value are in a single-quote.

**Example**
```
  Write a sample usage for this API if needed.
```

#### Markdown Example

```
# Class: Buffer

Buffer class is a global type with various constructors and accessors.
IoT.js provides Buffer to manipulate binary data. Currently buffer has a pure
ES5 compatible implementation, but this might be reworked to use `UInt8Array` in the future.


### new Buffer(size)
* `size` {integer} Size of the new buffer.

Creates a new buffer of `size` bytes and initialize its data to zero.

```


### {Your_class_name}.{your_static_function_name}([{argument_name}])
* `{argument_name}` {{Argument_type}} {more information} **Default:** `{defalut_value}`
* Returns: {{return_type}} {more information}

- Write a description of this static function here.
- The first character of Class in the title must start with an `Uppercase letter`.
- The other rules are the same as mentioned before.

**Example**
```
  Write a sample usage for this API if needed.
```

#### Markdown Example

```
### Buffer.byteLength(str, encoding)

* `str` {string} Source string.
* `encoding` {string} String encoding.
* Returns: {integer} Byte length of source string.

Returns the byte length of a buffer representing the value
of the string argument encoded with encoding. The effect is
the same as:

```js
return new Buffer(str, encoding).length;
```


### {your_class_name}.{property_name}
* {{property_type}}

- Write a description of this property here.
- The first character of the title must start with a `lowercase letter`.

**Example**
```
  Write a sample usage for this API if needed.
```
#### Markdown Example
```
### buf.length
* {integer}

Returns the capacity of the buffer in bytes. Note: when
the buffer is converted to another type (e.g. `String`) the
length of the converted value might be different from
this value.

**Example**

```js
var Buffer = require('buffer');

var buffer = new Buffer([0xc8, 0x80])
console.log(buffer.length); // prints 2

var str = buffer.toString();
console.log(str.length); // prints 1
```


### {your_class_name}.{your_prototype_function_name}([{argument_name}])
* `{argument_name}` {{Argument_type}} {more information} **Default:** `{defalut_value}`
* Returns: {{return_type}} {more information}

- Write a description of this prototype function here.
- The first character of Class in the title must start with a `lowercase letter`.
- The other rules are the same as mentioned before.

**Example**
```
  Write a sample usage for this API if needed.
```

#### Markdown Example

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


### Event: '{your_events_name}'
* `{callback_name}` {{callback_function_argument}}
  * `{argument1}` {{argument2_type}} {more information}

- Write a description of this here.
- In case of Event, the name of Class that this event belongs to, is not prepended in the title.
- The other rules are the same as mentioned before.

#### Markdown Example

```
### Event: 'lookup'
* `callback` {Function}
  * `err` {Error}
  * `address` {string}
  * `family` {string|null}

Emitted after resolving hostname.
```

 - Notice that the `err {Error}` above is started with `2 spaces` indentation since it's given to `callback` as parameters, not `lookup` event.
