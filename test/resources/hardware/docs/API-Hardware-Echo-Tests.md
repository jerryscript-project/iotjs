## API Hardware Echo Tests

### Echo device

Echo device is just an Arduino Board (ATmega328P) which responds on specific actions. The code that needs to be uploaded to arduino board is located in tests/arduino/echo_service.ino

#### Implemented tests

#### GPIO

RPI3:
* Arduino IOref and Vin pins with test device Vout pins
* Arduino pin 7 to pin 7 on test device (gpio48)
* Arduino pin 8 to pin 8 on test device (gpio50)
* Arduino GND with test device GND

ARTIK053:
* Arduino IOref and Vin pins with test device Vout pins
* Arduino pin 7 to pin 18 on test device
* Arduino pin 8 to pin 16 on test device
* Arduino GND with test device GND

With this setup Arduino board is powered by test service so no signal converers/transistors/resistors are needed

`test/run_pass/test_gpio_echo.js`

This is the test file which checks starting state input pin, puts a HIGH signal on output pin and tests input pin again which should be set to HIGH by echo service (Arduino)

#### I2C

TODO

#### SPI

RPI3:
* Ardino 13 SCK Pin to test devce SPI Clock pin
* Arduno 12 MISO Pin to test device SPI MISO pin
* Arduno 11 MOSI Pin to test device SPI MOSI pin
* Arduno 10 Chip Select Pin to GND

ARTIK053:
* Ardino 13 SCK Pin to test devce SPI Clock pin
* Arduno 12 MISO Pin to test device SPI MISO pin
* Arduno 11 MOSI Pin to test device SPI MOSI pin
* Arduno 10 Chip Select Pin to test device SPI Chip Select

To run the test please run

`test/run_pass/test_spi_echo.js`

#### UART

RPI3:
* Arduino RX pin 0 to RPI3 TX pin 8
* Arduino TX pin 1 to RPI3 RX pin 10
* Arduino GND pin to Artik 053 GND pin
ARTIK053:
* Arduino RX pin 0 to Artik 053 TX pin 1 on CON709
* Arduino TX pin 1 to Artik 053 RX pin 0 on CON709
* Arduino GND pin to Artik 053 GND pin

To run the test please run

`test/run_pass/test_uart_echo.js`

The test sends string data to arduino echo service and waits for the same string to be sent back

#### ADC

ARTIK053:
* Artik A0 ADC pin on CON705 to low-pass filter Vout
* Artik 053 pin 23 on CON704 to pin 5 on Arduino
* Artik 053 pin 21 on CON704 to pin 6 on Arduino
* Arduino 9 PWM pin to low-pass filter Vin
* Arduino GND pin to low-pass filter GND
* Artik 053 GND pin to low-pass filter GND

To run the test please run

`test/run_pass/test_adc_echo.js`

At the test start the adc should read 0 values (or close to 0), then artik requests arduino for half duty cycle which creates ~1.6v on low-pass filter Vout, the adc value should be about ~700, then artik requests for full duty cycle which should generate about ~3.3v which should translate to adc value ~1500. If that was correct then the test is passed.

__Artik 053 note__ There is a bug in the ADC module, which gives first few read attemps at low values, this was fixed in the test code by using median values (this also filters signal imperfections)

Low-Pass filter schematic

![low pass filter](./low-pass-filter.png)


#### PWM

ARTIK053:
* Arduino A0 ADC pin low-pass filter Vout
* Arduino AREF pin to device Artik 053 3,3V
* Artik 053 pin 7 on CON703 to low-pass filter Vin
* Artik 053 GND pin to low-pass filter GND
* Arduino 4 pin to Artik 053 pin 13 on CON703
* Arduino 3 pin to Artik 053 pin 15 on CON703
* Arduino 2 pin to Artik 053 pin 17 on CON703

To run the test please run

`test/run_pass/test_pwm_echo.js`

The test starts with PWM half duty cycle to generate half of max voltage (1.6V), if Arduino reads that it responds with request for full voltage (3.3V) if that is ok then Arduino responds that test is passed.

Low-Pass filter schematic

![low pass filter](./low-pass-filter.png)


