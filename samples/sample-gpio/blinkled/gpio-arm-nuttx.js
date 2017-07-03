/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// for NuttX on STM32F4-discovery BB
var GMS = 18; // mode shift
var G_M_MASK  = (3 << GMS);
var G_INPUT   = (0 << GMS);
var G_OUTPUT  = (1 << GMS);

var GPS = (16); // pull up/dn shift
var G_P_MASK  = (3 << GPS);
var G_FLOAT   = (0 << GPS);
var G_PULLUP  = (1 << GPS);
var G_PULLDOWN= (2 << GPS);

var GTS = (4); // port shift
var G_T_MASK= (7 << GTS);
var G_PORTA = (0 << GTS);
var G_PORTB = (1 << GTS);
var G_PORTC = (2 << GTS);
var G_PORTD = (3 << GTS);

var GIS = (0);
var G_I_MASK=(15 << GIS);
var G_PIN0  = (0 << GIS);
var G_PIN1  = (1 << GIS);
var G_PIN2  = (2 << GIS);
var G_PIN3  = (3 << GIS);
var G_PIN4  = (4 << GIS);
var G_PIN5  = (5 << GIS);
var G_PIN6  = (6 << GIS);
var G_PIN7  = (7 << GIS);
var G_PIN8  = (8 << GIS);
var G_PIN9  = (9 << GIS);
var G_PIN10 =(10 << GIS);
var G_PIN11 =(11 << GIS);
var G_PIN12 =(12 << GIS);
var G_PIN13 =(13 << GIS);
var G_PIN14 =(14 << GIS);
var G_PIN15 =(15 << GIS);

var GPIO_MAP = {
  CTRL: {
    ENABLE : 0,
    DISABLE : 0,
    OUT: G_OUTPUT | G_PULLUP,
    IN: G_INPUT | G_PULLDOWN,
    FLOAT: G_OUTPUT | G_FLOAT
  },
  PINS: {
    LED1: { PIN: G_PIN8  | G_PORTA },
    LED2: { PIN: G_PIN10 | G_PORTA },
    LED3: { PIN: G_PIN15 | G_PORTA },
    LED4: { PIN: G_PIN11 | G_PORTD },

    LED5: { PIN: G_PIN3  | G_PORTA },
    BTN1: { PIN: G_PIN0  | G_PORTA }
  }
};

module.exports = GPIO_MAP;
