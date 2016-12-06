## Module: pin

## STM32F4-discovery
The pin name of STM32F4-discovery is `STM32F4DIS`.
``` javascript
var stm32f4Pin = require('pin').STM32F4DIS;
```


### GPIO
`GPIO.P<port><pin>`

For example,
``` javascript
var gpio = require('gpio');
var stm32f4Pin = require('pin').STM32F4DIS;

gpio.open(stm32f4Pin.GPIO.PD6);
```


### PWM
`PWM.TIM<timer>.CH<channel>_<number>`

For example,
``` javascript
var pwm = require('pwm');
var stm32f4Pin = require('pin').STM32F4DIS;

var pwm2 = new pwm(stm32f4Pin.PWM.TIM2_CH1_2);
```
The following is a list of PWM pin name.

| PWM Pin Name | GPIO Name | PWM Pin Name | GPIO Name|
| :---: | :---: | :---: | :---: | :---: |
| TIM1_CH1_1 | PA8 | TIM4_CH1_1| PB6 |
| TIM1_CH1_2 | PE9 | TIM4_CH1_2| PD12 |
| TIM1_CH2_1 | PA9 | TIM4_CH2_1| PB7 |
| TIM1_CH2_2 | PE11 | TIM4_CH2_2| PD13 |
| TIM1_CH3_1 | PA10 | TIM4_CH3_1| PB8 |
| TIM1_CH3_2 | PE13 | TIM4_CH3_2| PD14 |
| TIM1_CH4_1 | PA11 | TIM4_CH4_1| PB9 |
| TIM1_CH4_2 | PE14 | TIM4_CH4_2| PD15 |
| TIM2_CH1_1| PA0 | TIM5_CH1_1| PA0 |
| TIM2_CH1_2| PA15 | TIM5_CH2_1| PA1 |
| TIM2_CH1_3| PA5 | TIM5_CH3_1| PA2 |
| TIM2_CH2_1| PA1 | TIM5_CH4_1| PA3 |
| TIM2_CH2_2| PB3 | TIM8_CH1_1| PC6 |
| TIM2_CH3_1| PA2| TIM8_CH2_1| PC7|
| TIM2_CH3_2| PB10| TIM8_CH3_1| PC8|
| TIM2_CH4_1| PA3 | TIM8_CH4_1| PC9 |
| TIM2_CH4_2| PB11 | TIM9_CH1_1 | PA2 |
| TIM3_CH1_1 | PA6 | TIM9_CH1_2 | PE5 |
| TIM3_CH1_2 | PB4 | TIM9_CH2_1 | PA3 |
| TIM3_CH1_3 | PC6 | TIM9_CH2_2 | PE6 |
| TIM3_CH2_1 | PA7 | TIM10_CH1_1 | PB8 |
| TIM3_CH2_2 | PB5 | TIM11_CH1_1 | PB9 |
| TIM3_CH2_3 | PC7 | TIM12_CH1_2 | PB14 |
| TIM3_CH3_1 | PA11 | TIM12_CH2_1 | PB15 |
| TIM3_CH3_2 | PE14 | TIM13_CH1_1 | PA6 |
| TIM3_CH4_1 | PB1 | TIM14_CH1_1 | PA7 |
| TIM3_CH4_2 | PC9 | | |
