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

// for linux on x86
var GPIO_MAP = {
  CTRL: {
    // not used, maybe set to usb gpio later
    ENABLE:  0x1000,
    DISABLE: 0x2000,
    IN:      0x0000,
    OUT:     0x0100,
    FLOAT:   0x0200,
  },
  PINS: {
    LED1: { PIN: 1 },
    LED2: { PIN: 2 },
    LED3: { PIN: 3 },
    LED4: { PIN: 4 },

    LED5: { PIN: 11 },
    BTN1: { PIN: 12 },
  },
};

module.exports = GPIO_MAP;
