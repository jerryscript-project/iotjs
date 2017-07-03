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

// reads GPIO configuration file for iot.js or node.js
// provides simple method for pin open/close/map

var cfgfile = "gpio-" + process.arch + "-" + process.platform + ".js";
var gpiomap = require(cfgfile);

function GpioCfg() {

}


GpioCfg.map = function(portname) {
  return gpiomap.PINS[portname].PIN;
}


GpioCfg.enableout = function(portname) {
  return gpiomap.PINS[portname].PIN |
         gpiomap.CTRL.ENABLE |
         gpiomap.CTRL.OUT;
}


GpioCfg.enablein = function(portname) {
  return gpiomap.PINS[portname].PIN |
         gpiomap.CTRL.ENABLE |
         gpiomap.CTRL.IN;
}


GpioCfg.disablefloat = function(portname) {
  return gpiomap.PINS[portname].PIN |
         gpiomap.CTRL.DISABLE |
         gpiomap.CTRL.FLOAT;
}


module.exports = GpioCfg;
