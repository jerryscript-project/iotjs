## This is a sample API reference. Please use this as a guideline to write your module's API references.

- If you have any questions about this guide, please let us know as an issue.
- `Markdown Example` is added to help understanding, and you can ignore it when writing the actual document.
- In case of `experimental` module, it's required to explicitly indicate that the features are experimental. Please put the caution below to the begining of the document.
> :exclamation: This document describes an experimental feature and considerations. Please be aware that every experimental feature may change, be broken, or be removed in the future without any notice.

***

### Platform Support

The following shows `{Your_module_name}` module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| {functionName1} | O | O | O |
| {functionName2} | O | O | O |
| {functionName3} | O | O | O |

### Contents

- [{Your_module_name}](#your_module_name)
  - [Class: {Your_class_name}](#class-your_class_name)
      - [Constructor](#constructor)
          - [`new {Your_class_name}([{argument_name}])`](#new-your_class_nameargument_name)
      - [Properties](#properties)
          - [`{your_class_name}.{property_name}`](#your_class_nameproperty_name)
      - [Static Functions](#static-functions)
          - [`{Your_class_name}.{your_static_function_name}([{argument_name}])`](#your_class_nameyour_static_function_nameargument_name)
      - [Prototype Functions](#prototype-functions)
          - [`{your_class_name}.{your_prototype_function_name}([{argument_name}])`](#your_class_nameyour_prototype_function_nameargument_name)
      - [Events](#events)
          - [`{your_events_name}`](#your_events_name)
  - [Module Functions](#module-functions)
      - [`{your_module_name}.{your_module_function_name}([{argument_name}])`](#your_module_nameyour_module_function_nameargument_name)

# {Your_module_name}

- Write a brief description of this module here.
- The first character of the title must start with an `uppercase letter`.

#### Markdown Example

```
# Timer

The timer module exposes a global API for scheduling functions to be called at some future period of time.
Because the timer functions are globals, there is no need to call require('timers') to use the API.
```

# Class: {Your_class_name}

- Write a brief description of this class here.
- The first character of the title must start with an `uppercase letter`.
- The table of contents should be in order of `"Constructor"`, `"Properties"`, `"Protype Functions"`, and `"Events"`.
- While you are writing this description, if you need to write down module / class / function / event name, arguments, or type which you already mentioned, then enclose the keyword in single-quotation. This rule applies to other items as well.

    E.g) The given `callback` is called every `delay` milliseconds. If it's not a function, a `TypeError` will be thrown.

## Constructor

### `new {Your_class_name}([{argument_name}])`
* `{argument_name} <{Argument_type}> Default: {defalut_value}` - {more information}

Notice that every API name is in a single-quote.

#### Markdown Example

```
# Class: Buffer

Buffer class is a global type with various constructors and accessors.
IoT.js provides Buffer to manipulate binary data. Currently buffer has a pure
ES5 compatible implementation, but this might be reworked to use `UInt8Array` in the future.

## Constructor

### `new Buffer(size)`
* `size <Integer> Default: 0` - size of the new buffer

Creates a new buffer of `size` bytes and initialize its data to zero.

```

## Properties

### `{your_class_name}.{property_name}`

- Write a description of this property here.
- The title should be in a single quote.
- The first character of the title must start with a `lowercase letter`.

**Example**
```
  Write a sample usage for this API if needed.
```
#### Markdown Example
```
## Properties

### `buffer.length`
* `<Integer>` - length of the buffer

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

## Static Functions

Write a description of static functions that belongs to the current class.

### `{Your_class_name}.{your_static_function_name}([{argument_name}])`
* `{argument_name} <{Argument_type}> Default: {defalut_value}` - {more information}

- Write a description of this function here.
- The first character of Class in the title must start with an `Uppercase letter`.
- The other rules are the same as mentioned before.

**Example**
```
  Write a sample usage for this API if needed.
```


#### Markdown Example

```
## Static Functions

### `Buffer.byteLength(str[, encoding])`
* `str <String>` - source string
* `encoding <String>` - string encoding
* Returns: `<Integer>` - byte length of source string

Returns the byte length of a buffer representing the value
of the `string` argument encoded with `encoding`. The
effect is the same as:

```js
return new Buffer(str, encoding).length;
```

## Prototype Functions

Write a description of prototype functions that belongs to the current class.

### `{your_class_name}.{your_prototype_function_name}([{argument_name}])`
* `{argument_name} <{Argument_type}> Default: {defalut_value}` - {more information}

- Write a description of this function here.
- The first character of Class in the title must start with a `lowercase letter`.
- The other rules are the same as mentioned before.

**Example**
```
  Write a sample usage for this API if needed.
```

#### Markdown Example

```
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

## Events

### `{your_events_name}`
* `{callback_name} <{callback_function_argument}>`
  * `{argument1} <{argument2_type}>` - {more information}

- Write a description of this here.
- In case of Event, the name of Class that this event belongs to, is not prepended in the title.
- The other rules are the same as mentioned before.

#### Markdown Example

```
## Events

### `'lookup'`
* `callback <Function(err, address, family)>`
  * `err <Error> | Null` - Optionally, write a description for each argument.
  * `address <String>`
  * `family <String> | Null`
```

 - Notice that the `err <Error> | Null` above is started with `2 spaces` indentation since it's given to `callback` as parameters, not `lookup` event.

# Module Functions

- `Module functions` are what you can directly invoke without an instance of a certain Class. E.g) net.connect.
- Write a description of this here.

## `{your_module_name}.{your_module_function_name}([{argument_name}])`
* `{argument_name} <{Argument_type}> Default: {defalut_value}` - {more information}

- Write a description of this function here.
- The first character of Class in the title must start with a `lowercase letter`.
- The other rules are the same as mentioned before.

### Example <a name="example-2"></a>
```
  Write a sample usage for this API if needed.
```

#### Markdown Example
```
### `gpio.open(configuration[, callback])`
* `configuration <Object>`
  * `pin <Number>` - pin number to configure, mandatory configuration
  * `direction <GPIO.DIRECTION> Default: GPIO.DIRECTION.OUT` - direction of the pin
* `callback <Function(err: Error | null)>`
* Returns: `<GPIOPin>`
```
- Notice that the `pin <Number>` above is started with `2 spaces` indentation since it's a property inside `configuration`.
ss