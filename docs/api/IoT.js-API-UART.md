### Platform Support

The following shows uart module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| uart.open | O | O | O |
| uartport.write | O | O | O |
| uartport.writeSync | O | O | O |
| uartport.close | O | O | O |
| uartport.closeSync | O | O | O |


## Contents
  * [UART](#uart)
    * [Constructor](#constructor)
      * [`new UART()`](#new-uart)
    * [Prototype Methods](#uart-prototype-methods)
      * [`uart.open(configurable[, callback])`](#uart-open)
  * [UARTPort](#uartport)
    * [Prototype Methods](#uartport-prototype-methods)
      * [`uartport.write(data[, callback])`](#uartport-write)
      * [`uartport.writeSync(data)`](#uartport-write-sync)
      * [`uartport.close([callback])`](#uartport-close)
      * [`uartport.closeSync()`](#uartport-close-sync)
    * [Event](#uartport-event)


## Class: UART <a name="uart"></a>


## Constructor <a name="uart-constructor"></a>


### `new UART()` <a name="uart-new"></a>

Returns a new UART object which can open UART port.


## Prototype methods <a name="uart-prototype-methods"></a>


### `uart.open(configuration[, callback])` <a name="uart-open"></a>
* `configuration <Object>`
  * `device <String>`, mandatory configuration
  * `baudRate: Number (Default value is 9600)`
  * `dataBits: Number (Default value is 8)`
* `callback <Function(err: Error | null)>`
* Returns: `<UARTPort>`

On Nuttx, you can set the properties of `configuration` only on Nuttx config. `Device Drivers -> Serial Driver Support -> U[S]ART(N) Configuration` will help. And if you want to get information about `device`, please see below list.
  * [STM32F4-discovery](../../targets/nuttx-stm32f4/Stm32f4dis.md#uart)

Opens UART port with the specified configuration. Following methods can be called with UARTPort object.


**Example**
```javascript
var Uart = require('uart');

var uart = new Uart();
var configuration = {
  device: '/dev/ttyUSB0'
  baudRate: 115200,
  dataBits: 8,
}

var serial = uart.open(configuration, function(err) {
  // Do something.
});
```


## Class: UARTPort <a name="uartport"></a>


## Prototype methods <a name="uartport-prototype-methods"></a>


### `uartport.write(data[, callback])` <a name="uartport-write"></a>
* `data: String`
* `callback: Function(err: Error | null)`

Writes data to UART device asynchronously.

**Example**
```javascript
serial.write("Hello?", function(err) {
  if (err) {
    // Do something.
  }
  serial.close();
});
```

### `uartport.writeSync(data)` <a name="uartport-write-sync"></a>
* `data: String`

Writes data to UART device synchronously.

**Example**
```javascript
serial.writeSync("Hello?");
serial.close();
```


### `uartport.close([callback])` <a name="uartport-close"></a>
* `callback: Function(err: Error | null)`

Closes UART device asynchronously.


### `uartport.closeSync()` <a name="uartport-close-sync"></a>

Closes UART device synchronously.


## Events <a name="uartport-event"></a>


### `'data'`
* `callback: Function(data)`

 `data` is a string from sender

For example,
```javascript
...
serial.on('data', function(data) {
  console.log('read result: ' + data.toString());
});

```