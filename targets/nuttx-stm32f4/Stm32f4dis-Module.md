## stm32f4dis Module

## pin

To use system IO, such as GPIO, PWM, ADC and so on, you must know pin name.
`stm32f4dis` module has pin object which is designed to find a pin name easier.

``` javascript
var pin = require('stm32f4dis').pin;
```

### GPIO
`P<port><pin>`

For example,
``` javascript
var gpio = require('gpio');
var pin = require('stm32f4dis').pin;

gpio.open(pin.PD6);
```


### PWM
`PWM<timer>.CH<channel>_<number>`

For example,
``` javascript
var pwm = require('pwm');
var pin = require('stm32f4dis').pin;

var pwm2 = new pwm(pin.PWM2.CH1_2);
```
The following is a list of PWM pin name.

| PWM Pin Name | GPIO Name | PWM Pin Name | GPIO Name|
| :---: | :---: | :---: | :---: | :---: |
| PWM1.CH1_1 | PA8 | PWM4.CH1_1| PB6 |
| PWM1.CH1_2 | PE9 | PWM4.CH1_2| PD12 |
| PWM1.CH2_1 | PA9 | PWM4.CH2_1| PB7 |
| PWM1.CH2_2 | PE11 | PWM4.CH2_2| PD13 |
| PWM1.CH3_1 | PA10 | PWM4.CH3_1| PB8 |
| PWM1.CH3_2 | PE13 | PWM4.CH3_2| PD14 |
| PWM1.CH4_1 | PA11 | PWM4.CH4_1| PB9 |
| PWM1.CH4_2 | PE14 | PWM4.CH4_2| PD15 |
| PWM2.CH1_1| PA0 | PWM5.CH1_1| PA0 |
| PWM2.CH1_2| PA15 | PWM5.CH2_1| PA1 |
| PWM2.CH1_3| PA5 | PWM5.CH3_1| PA2 |
| PWM2.CH2_1| PA1 | PWM5.CH4_1| PA3 |
| PWM2.CH2_2| PB3 | PWM8.CH1_1| PC6 |
| PWM2.CH3_1| PA2| PWM8.CH2_1| PC7|
| PWM2.CH3_2| PB10| PWM8.CH3_1| PC8|
| PWM2.CH4_1| PA3 | PWM8.CH4_1| PC9 |
| PWM2.CH4_2| PB11 | PWM9.CH1_1 | PA2 |
| PWM3.CH1_1 | PA6 | PWM9.CH1_2 | PE5 |
| PWM3.CH1_2 | PB4 | PWM9.CH2_1 | PA3 |
| PWM3.CH1_3 | PC6 | PWM9.CH2_2 | PE6 |
| PWM3.CH2_1 | PA7 | PWM10.CH1_1 | PB8 |
| PWM3.CH2_2 | PB5 | PWM11.CH1_1 | PB9 |
| PWM3.CH2_3 | PC7 | PWM12.CH1_2 | PB14 |
| PWM3.CH3_1 | PA11 | PWM12.CH2_1 | PB15 |
| PWM3.CH3_2 | PE14 | PWM13.CH1_1 | PA6 |
| PWM3.CH4_1 | PB1 | PWM14.CH1_1 | PA7 |
| PWM3.CH4_2 | PC9 | | |


### ADC
`ADC<number>_<timer>`

For example,
``` javascript
var adc = require('adc');
var pin = require('stm32f4dis').pin;

var adc1 = new adc(pin.ADC1_3);
```
The following is a list of ADC pin name.

| ADC Pin Name | GPIO Name |
| :--- | :---: |
| ADC1_0, ADC2_0, ADC3_0 | PA0 |
| ADC1_1, ADC2_1, ADC3_1 | PA1 |
| ADC1_2, ADC2_2, ADC3_2 | PA2 |
| ADC1_3, ADC2_3, ADC3_3 | PA3 |
| ADC1_4, ADC2_4 | PA4 |
| ADC1_5, ADC2_5 | PA5 |
| ADC1_6, ADC2_6 | PA6 |
| ADC1_7, ADC2_7 | PA7 |
| ADC1_8, ADC2_8 | PB0 |
| ADC1_9, ADC2_9 | PB1 |
| ADC1_10, ADC2_10, ADC3_10 | PC0 |
| ADC1_11, ADC2_11, ADC3_11 | PC1 |
| ADC1_12, ADC2_12, ADC3_12 | PC2 |
| ADC1_13, ADC2_13, ADC3_13 | PC3 |
| ADC1_14, ADC2_14 | PC4 |
| ADC1_15, ADC2_15 | PC5 |
