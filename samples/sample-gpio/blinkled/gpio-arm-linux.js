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

// for Raspberry Pi B and 2
var GPIO_MAP = {
  CTRL: {
    // follows RPi2 GPIO control
    ENABLE:  0x00020000,
    DISABLE: 0x00000000,
    OUT:     0x00010000,
    IN:      0x00000000,
    FLOAT:   0x00000000
  },
  PINS: {
    LED1: { PIN: 11 },
    LED2: { PIN: 12 },
    LED3: { PIN: 13 },
    LED4: { PIN: 15 },

    LED5: { PIN: 16 },
    BTN1: { PIN: 18 }
  }
};

module.exports = GPIO_MAP;
