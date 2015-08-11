/* Copyright 2015 Samsung Electronics Co., Ltd.
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

/*
  @TIMEOUT=10
*/

var assert = require('assert');
var gpio = require('gpio');

var gpioSequence = '';
var result;

result = gpio.initialize();
if (result >= 0)
  gpioSequence += "I";

gpio.setPin(1, "in");
gpioSequence += "A";

try {
  gpio.setPin(-1, "out");
}
catch(err) {
  if(err.code == gpio.ERR_INVALIDPARAM) {
    gpioSequence += '1';
  }
}

try {
  gpio.writePin(-1, true);
}
catch(err) {
  if(err.code == gpio.ERR_INVALIDPARAM) {
    gpioSequence += '2';
  }
}

try {
  gpio.readPin(-1);
}
catch(err) {
  if(err.code == gpio.ERR_INVALIDPARAM) {
    gpioSequence += '3';
  }
}

try {
  gpio.readPin(2);
}
catch(err) {
  if(err.code == gpio.ERR_INVALIDPARAM) {
    gpioSequence += '4';
  }
}


gpio.setPin(2, "out", function(err) {
  gpioSequence += 'B';
});

gpio.setPin(-1, "out", function(err) {
  if(err.code == gpio.ERR_INVALIDPARAM) {
    gpioSequence += 'C';
  }
});

gpio.setPin(3, "in", "float", function(err) {
  gpioSequence += 'D';
});


gpio.writePin(2, true, function(err) {
  gpioSequence += 'E';
});

gpio.readPin(3, function(err,value) {
  if (value == true)
    gpioSequence += 'F';
});

gpio.readPin(4, function(err,value) {
  if (value == false)
    gpioSequence += 'G';
});


process.on('exit', function(code) {
  gpio.release();
  assert.equal(gpioSequence, 'IA1234BCDEFG');
});
