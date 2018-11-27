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
var i2c = require('i2c');

// ------ Test API existance
assert.equal(typeof i2c.open, 'function',
             'i2c does not provide \'open\' function');
assert.equal(typeof i2c.openSync, 'function',
             'i2c does not provide \'openSync\' function');

function check_i2cbus(i2cbus) {
  assert.equal(typeof i2cbus.write, 'function',
               '\'i2cpin\' does not provide \'write\' function');
  assert.equal(typeof i2cbus.writeSync, 'function',
               '\'i2cpin\' does not provide \'writeSync\' function');
  assert.equal(typeof i2cbus.read, 'function',
               '\'i2cpin\' does not provide \'read\' function');
  assert.equal(typeof i2cbus.readSync, 'function',
               '\'i2cpin\' does not provide \'readSync\' function');
  assert.equal(typeof i2cbus.close, 'function',
               '\'i2cpin\' does not provide \'close\' function');
  assert.equal(typeof i2cbus.closeSync, 'function',
               '\'i2cpin\' does not provide \'closeSync\' function');
}

// ------ Test synchronous I2C Bus opening
assert.throws(
  function() {
    i2c.openSync({address: '0x0'});
  },
  TypeError
);

assert.throws(
  function() {
    i2c.openSync('0x0');
  },
  Error
);

assert.throws(
  function() {
    i2c.openSync({});
  },
  TypeError
);

var bus = i2c.openSync({address: 0x23});
check_i2cbus(bus);

assert.doesNotThrow(
  function() {
    bus.writeSync([0x10, 123, -12]);
    read_result = bus.readSync(5);
  }
);

assert.throws(
  function() {
    bus.writeSync(0x23);
  },
  Error
);

assert.throws(
  function() {
    bus.writeSync(null);
  },
  Error
);

assert.throws(
  function() {
    bus.readSync('5');
  },
  Error
);

assert.throws(
  function() {
    bus.readSync(null);
  },
  Error
);

assert.throws(
  function() {
    bus.readSync([5]);
  },
  Error
);

assert.throws(
  function() {
    bus.readSync({});
  },
  Error
);

assert.assert(Array.isArray(read_result));
assert.strictEqual(read_result.length, 5);

bus.closeSync();

// ------ Test asynchronous I2C Bus opening
i2c.open({address: 0x0}, function(open_err, async_bus) {
  assert.equal(open_err, null);
  open_cb1 = true;

  assert.throws(
    function() {
      async_bus.read(null);
    },
    Error
  );

  assert.throws(
    function() {
      async_bus.write(null);
    },
    Error
  );

  async_bus.write([0x10, 123, -12], function(write_err) {
    assert.equal(write_err, null);
    write_cb1 = true;

    async_bus.read(5, function(read_err, res) {
      assert.equal(read_err, null);
      read_cb1 = true;
      assert.assert(Array.isArray(res));
      assert.strictEqual(res.length, 5);

      async_bus.close(function(close_err) {
        assert.equal(close_err, null);
        close_cb1 = true;
      });
    });
  });
});

process.on('exit', function(code) {
  if (code === 0) {
    assert.assert(open_cb1, 'callback of \'i2c.open\' was not called');
    assert.assert(close_cb1, 'callback of \'i2c.close\' was not called');

    assert.assert(read_cb1, 'callback of \'i2cbus.read\' was not called');
    assert.assert(write_cb1, 'callback of \'i2cbus.write\' was not called');
  }
});

