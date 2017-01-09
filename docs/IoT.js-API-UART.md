## Module: uart

### Platform Support

The following shows uart module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| uart.open | O | O | O |
| uart.close | O | O | O |
| uart.write | O | O | O |

### Constructor

#### new uart(path[, options][, callback]) for Ubuntu, Raspbian
#### new uart(path[, callback]) for Nuttx
* `path: String`
* `options: Object`
* `callback: Function(err)`

`options` is an object specifying following information:
* `baudRate: Number (Default value is 9600)`
* `dataBits: Number (Default value is 8)`
* `autoOpen: Boolean (Default value is true)`

* On Nuttx, you can set the properties of `options` only on Nuttx config. `Device Drivers -> Serial Driver Support -> U[S]ART(N) Configuration` will help. 

Create UART object. Following methods can be called with UART object.


For example,
```javascript
var uart = require('uart');
var options = {
  baudRate: 115200,
  dataBits: 8,
  autoOpen: true,
}

var serial = new uart('/dev/ttyUSB0', options, function(err) {
  // Do something.
});

serial.write("Hello?", function(err) {
  if (err) {
    // Do something.
  }
  serial.close();
});
```


### Events

#### `'data'`
* `callback: Function(data)`

 `data` is a string from sender

For example,
```javascript
...
serial.on('data', function(data) {
  console.log('read result: ' + data.toString());
});

```


### Methods

#### uart.open(callback)
* `callback: Function(err)`

Opens UART device.


#### uart.close()

Closes UART device.


#### uart.write(data, callback)
* `data: String`
* `callback: Function(err)`

Writes data to UART device.


## Particular information for Nuttx

To use UART on stm32f4-discovery board, you must use proper pins. Stm32f4-discovery board supports 4 UART ports, such as USART2, USART3, UART5, USART6.
But as our default config option sets SDIO to be on, it makes conflict with UART5 port because these two use the same pin for connection. So you must be careful when you enable UART5 port.

### How to enable UART port
 * [Nuttx-stm32f4](../targets/nuttx-stm32f4/README.md).

### Pin map for STM32F4-Discovery board

| U[S]ART Pin Name | GPIO Name |
| :--- | :---: |
| USART2_TX | PA2 |
| USART2_TX | PA3 |
| USART3_RX | PB10 |
| USART3_TX | PB11 |
| UART5_RX | PC12 |
| UART5_TX | PD2 |
| USART6_RX | PC6 |
| USART6_RX | PC7 |

* Different from other system IO such as GPIO, ADC, PWM, you can't find the name of the UART device easily by `stm32f4dis.pin` module. It's because the name of the uart device can be changed according to your Nuttx config option. You can find '/dev/ttyS[0-3]' according to your environment.

### Enable more ports using patch file

Current version of Nuttx doesn't support USART1 and UART4 as the ports for stm32f4-discovery board. But if you want to enable more ports other than above, you can modify Nuttx code by referring to a part of `targets/nuttx-stm32f4/nuttx/patch` file.

To apply whole patch,
```bash
~/workspace/nuttx$ patch -p1 < ../iotjs/targets/nuttx-stm32f4/nuttx/patch
```
Make sure it is your responsibility to enable the ports only when they do not make any conflicts.
