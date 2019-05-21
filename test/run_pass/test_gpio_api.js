/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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

var assert = require('assert');
var gpio = require('gpio');

// ------ Test API existance
assert.assert(gpio.DIRECTION,
              'gpio module does not provide \'DIRECTION\' property');
assert.notEqual(gpio.DIRECTION.IN, undefined);
assert.notEqual(gpio.DIRECTION.OUT, undefined);

assert.assert(gpio.EDGE,
              'gpio module does not provide \'EDGE\' property');
assert.notEqual(gpio.EDGE.NONE, undefined);
assert.notEqual(gpio.EDGE.RISING, undefined);
assert.notEqual(gpio.EDGE.FALLING, undefined);
assert.notEqual(gpio.EDGE.BOTH, undefined);

assert.assert(gpio.MODE,
              'gpio module does not provide \'MODE\' property');
assert.notEqual(gpio.MODE.NONE, undefined);
if (process.platform === 'nuttx') {
  assert.notEqual(gpio.MODE.PULLUP, undefined);
  assert.notEqual(gpio.MODE.PULLDOWN, undefined);
  assert.notEqual(gpio.MODE.FLOAT, undefined);
  assert.notEqual(gpio.MODE.PUSHPULL, undefined);
  assert.notEqual(gpio.MODE.OPENDRAIN, undefined);
}

assert.equal(typeof gpio.open, 'function',
             'gpio does not provide \'open\' function');
assert.equal(typeof gpio.openSync, 'function',
             'gpio does not provide \'openSync\' function');


function check_gpiopin(gpiopin) {
  assert.equal(typeof pin.setDirectionSync, 'function',
               '\'gpiopin\' does not provide \'setDirectionSync\' function');
  assert.equal(typeof pin.write, 'function',
               '\'gpiopin\' does not provide \'write\' function');
  assert.equal(typeof pin.writeSync, 'function',
               '\'gpiopin\' does not provide \'writeSync\' function');
  assert.equal(typeof pin.read, 'function',
               '\'gpiopin\' does not provide \'read\' function');
  assert.equal(typeof pin.readSync, 'function',
               '\'gpiopin\' does not provide \'readSync\' function');
  assert.equal(typeof pin.close, 'function',
               '\'gpiopin\' does not provide \'close\' function');
  assert.equal(typeof pin.closeSync, 'function',
               '\'gpiopin\' does not provide \'closeSync\' function');
}

// ------ Test synchronous GPIO pin opening
assert.throws(
  function() {
    gpio.openSync({pin: 0, direction: 123});
  },
  TypeError
);

assert.throws(
  function() {
    gpio.openSync({pin: 0, direction: {}});
  },
  TypeError
);

assert.throws(
  function() {
    gpio.openSync({pin: 0, direction: 'out'});
  },
  TypeError
);

assert.throws(
  function() {
    gpio.openSync({pin: 0, edge: 123});
  },
  TypeError
);

assert.throws(
  function() {
    gpio.openSync({pin: 0, edge: {}});
  },
  TypeError
);

assert.throws(
  function() {
    gpio.openSync({pin: 0, edge: 'rising'});
  },
  TypeError
);

assert.throws(
  function() {
    gpio.openSync({pin: '12'});
  },
  TypeError
);

assert.throws(
  function() {
    gpio.openSync({pin: {}});
  },
  TypeError
);

assert.throws(
  function() {
    gpio.openSync({pin: -12});
  },
  TypeError
);

var pin = gpio.openSync({pin: 0, direction: gpio.DIRECTION.OUT});
check_gpiopin(pin);
pin.closeSync();

assert.throws( // close twice
  function() {
    pin.closeSync();
  },
  Error
);

pin = gpio.openSync({pin: 0, direction: gpio.DIRECTION.IN});
check_gpiopin(pin);
pin.closeSync();

pin = gpio.openSync({pin: 0});
check_gpiopin(pin);

assert.doesNotThrow(function() {
  pin.setDirectionSync(gpio.DIRECTION.OUT)
});

assert.throws(
  function() {
    pin.setDirectionSync(123);
  },
  TypeError
);

assert.throws(
  function() {
    pin.setDirectionSync('out');
  },
  Error
);

assert.throws(
  function() {
    pin.setDirectionSync({});
  },
  Error
);

assert.doesNotThrow(
  function() {
    pin.writeSync(true);
    pin.writeSync(false);
    pin.writeSync(0);
    pin.writeSync(1);
    pin.writeSync(123);
    pin.writeSync(-123);
  }
);

assert.doesNotThrow(
  function() {
    pin.write(true, function(err) {
      assert.assert(err === null, 'gpio.write failed: ' + err);
      write_cb1 = true;
    });
    pin.write(false, function(err) {
      assert.assert(err === null, 'gpio.write failed: ' + err);
      write_cb2 = true;
    });
    pin.write(0, function(err) {
      assert.assert(err === null, 'gpio.write failed: ' + err);
      write_cb3 = true;
    });
    pin.write(1, function(err) {
      assert.assert(err === null, 'gpio.write failed: ' + err);
      write_cb4 = true;
    });
    pin.write(123, function(err) {
      assert.assert(err === null, 'gpio.write failed: ' + err);
      write_cb5 = true;
    });
    pin.write(-123, function(err) {
      assert.assert(err === null, 'gpio.write failed: ' + err);
      write_cb6 = true;
    });
  }
);

assert.throws(
  function() {
    pin.writeSync('true');
  },
  Error
);

assert.throws(
  function() {
    pin.writeSync({});
  },
  Error
);

assert.throws(
  function() {
    gpio.write({});
  },
  Error
);

assert.throws(
  function() {
    gpio.write('true');
  },
  Error
);

pin.write('true', function(err) {
  assert.assert(err, 'gpio.write did not fail as expected');
});

pin.write({}, function(err) {
  assert.assert(err, 'gpio.write did not fail as expected');
});

// ------ Test asynchronous GPIO pin opening

var async_pin1 = gpio.open(
  {
    pin: 20,
    direction: gpio.DIRECTION.OUT
  },
  function(err, async_pin2) {
    open_cb1 = true;
    assert.assert(err === null, 'gpio.open failed: ' + err);
    assert.assert(async_pin1);
    assert.assert(async_pin2);
    assert.assert(async_pin1 === async_pin2,
                  'return value and callback parameters are not equal');
    check_gpiopin(async_pin2);
    async_pin2.close(function(err) {
      close_cb1 = true;
      assert.assert(err === null, 'gpio.close failed: ' + err);
    });
  }
);

gpio.open(
  {
    pin: 21,
    direction: gpio.DIRECTION.IN
  },
  function(err, async_pin) {
    open_cb2 = true;
    assert.assert(err === null, 'gpio.open failed: ' + err);
    check_gpiopin(async_pin);
    async_pin.close(function(err) {
      close_cb2 = true;
      assert.assert(err === null, 'gpio.close failed: ' + err);
    });
  }
);

gpio.open(
  { pin: 22 },
  function(err, async_pin) {
    open_cb3 = true;
    assert.assert(err === null, 'gpio.open failed: ' + err);
    check_gpiopin(async_pin);
    async_pin.close(function(err) {
      close_cb3 = true;
      assert.assert(err === null, 'gpio.close failed: ' + err);
    });
  }
);

process.on('exit', function(code) {
  if (code === 0) {
    assert.assert(open_cb1, 'callback of \'gpio.open\' was not called');
    assert.assert(close_cb1, 'callback of \'gpio.close\' was not called');
    assert.assert(open_cb2, 'callback of \'gpio.open\' was not called');
    assert.assert(close_cb2, 'callback of \'gpio.close\' was not called');
    assert.assert(open_cb3, 'callback of \'gpio.open\' was not called');
    assert.assert(close_cb3, 'callback of \'gpio.close\' was not called');

    assert.assert(write_cb1, 'callback of \'gpio.write\' was not called');
    assert.assert(write_cb2, 'callback of \'gpio.write\' was not called');
    assert.assert(write_cb3, 'callback of \'gpio.write\' was not called');
    assert.assert(write_cb4, 'callback of \'gpio.write\' was not called');
    assert.assert(write_cb5, 'callback of \'gpio.write\' was not called');
    assert.assert(write_cb6, 'callback of \'gpio.write\' was not called');
  }
  pin.closeSync();
});
