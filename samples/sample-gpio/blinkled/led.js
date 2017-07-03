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

// LED blink sample for IoT.js and node.js
var events = require("events")
  , eventEmitter = new events.EventEmitter()
  , gpio = require("iotjs-gpio")
  , gpiocfg = require("gpiocfg.js")
  , gpio_pout = ["LED1", "LED2", "LED3", "LED4"]
  , gpio_ocnt = 4
  , intervalId
  , durationId;

function gpio_setpins() {
  var idx, portpin;
  var pin_ready_cnt = 0;
  for (idx=0; idx<gpio_ocnt; idx++) {
    portpin = gpiocfg.enableout(gpio_pout[idx]);

    gpio.pinmode(portpin, function(err) {
      pin_ready_cnt += 1;
      if (pin_ready_cnt >= gpio_ocnt) {
        eventEmitter.emit("pins_ready");
      }
    });
  }
}


function gpio_run() {
  var on = 1;
  var idx = 0;

  console.log("start blinking...");
  intervalId = setInterval(function() {
    var portpin = gpiocfg.map(gpio_pout[idx]);
    var err = gpio.write(portpin, on);
    idx = idx + 1;
    if (idx >= gpio_ocnt) {
      idx = 0;
      on = (on + 1) % 2;
    }
  }, 100);
}


function gpio_cleanup(timeout) {
  durationId = setTimeout(function() {
    clearInterval(intervalId);
    clearTimeout(durationId);
    console.log("blinking completed");

    var idx, portpin;
    for (idx=0; idx<gpio_ocnt; idx++) {
      portpin = gpiocfg.map(gpio_pout[idx]);
      gpio.write(portpin, 0);

      portpin = gpiocfg.disablefloat(gpio_pout[idx]);
      gpio.pinmode(portpin);
    }
    eventEmitter.emit("pins_done");
  }, timeout);
}


gpio.initialize(function(err) {
  gpio_setpins();
});


eventEmitter.on("pins_ready", function() {
  gpio_run();
  gpio_cleanup(10 * 1000); // run for 10 seconds
});


eventEmitter.on("pins_done", function() {
  gpio.release();
})
