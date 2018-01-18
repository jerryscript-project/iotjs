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
/**
 * Description:
 *
 * Turns light on/off with a fade effect
 *
 * Usage:
 *
 * To run this sample please connect a button to GND and pin 8 on CON708
 * header, the LED light cathode (-) to GND and anode (+) to pin 7 on CON703.
 * Next run:
 *
 * $ iotjs light-fade.js
 *
 * Pushing the button will turn on/off (toggle) the light with a fade effect.
 *
 */

var pwm = require('pwm'),
  gpio = require('gpio'),
  LOW = 0,
  HIGH = 1,
  FADE_TIME = 10000, // 10 seconds
  log_enabled = true,
  direction = 0, // 0 off 1 on
  buttonPin = 50,
  pwmPin = 0,
  step = 0.05,
  value = LOW,
  buttonDevice = null,
  pwmDevice = null;

// log only when log_enabled flag is set to true
function log(/*...args*/) {
  if (log_enabled) {
    console.log.apply(console, [].slice.call(arguments));
  }
}

// polling for gpio button changes
function buttonPoll() {
  buttonDevice.read(function (err, buttonValue) {
    var timeoutOffset = 0;
    if (buttonValue) {
      direction = direction ? 0 : 1; //reverse on button push
      log('switching to: ' + (direction ? 'ON': 'OFF'));
      timeoutOffset = 500; // offset the time for next check to prevent errors
                           // of mechanical buttons
    }
    setTimeout(buttonPoll, 100 + timeoutOffset);
  });
}

function mainLoop() {
  // handle fading
  if (direction === 0 && value > LOW) {
    value -= step;
  } else if (direction === 1 && value < HIGH) {
    value += step;
  }

  // clamping
  if (value > HIGH) {
    value = HIGH;
  } else if (value < LOW) {
    value = LOW;
  }

  pwmDevice.setDutyCycle(value, function (err) {
    if (err) {
      log('could not set device duty cycle');
    }
    setTimeout(mainLoop, FADE_TIME / (HIGH / step));
  });
}

log('setting up gpio');
buttonDevice = gpio.open({
  pin: buttonPin,
  direction: gpio.DIRECTION.IN,
  mode: gpio.MODE.NONE
}, function (err) {
  if (err) {
    log('error when opening gpio device: ' + err);
  } else {
    log('setting up pwm');
    pwmDevice = pwm.open({
      pin: pwmPin,
      period: 0.0001,
      dutyCycle: value
    }, function (err) {
      if (err) {
        log('error when opening pwm device: ' + err);
        buttonDevice.closeSync();
      } else {
        pwmDevice.setEnable(true, function (err) {
          if (err) {
            log('error when enabling pwm: ' + err);
            buttonDevice.closeSync();
            pwmDevice.close();
          } else {
            log('wating for user input');
            buttonPoll();
            mainLoop();
          }
        });
      }
    });
  }
});
