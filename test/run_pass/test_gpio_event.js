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

var Gpio = require('gpio');
var gpio = new Gpio();

var testGpioInfo = [
  {
    pin: 13,
    edge: gpio.EDGE.RISING
  },
  {
    pin: 19,
    edge: gpio.EDGE.FALLING
  },
  {
    pin: 26,
    edge: gpio.EDGE.BOTH
  }
];

testGpioInfo.forEach(function(info) {
  var switchGpio = gpio.open({
    pin: info.pin,
    edge: info.edge,
    direction: gpio.DIRECTION.IN
  }, function() {
    switchGpio.on('change', function() {
      console.log('pin:', info.pin, ', current value:', this.readSync());
    });
  });
});

setTimeout(function(){
  console.log('finish test');
}, 10000);
