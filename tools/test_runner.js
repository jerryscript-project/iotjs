/* Copyright 2016-present Samsung Electronics Co., Ltd. and other contributors
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
var util = require('util');
var testdriver = require('testdriver');
var console_wrapper = require('common_js/module/console');
var builtin_modules =
  Object.keys(process.native_sources).concat(Object.keys(process.binding));

function Runner(driver) {
  this.driver = driver;
  this.test = driver.currentTest();
  this.finished = false;
  if (driver.skipModule) {
    this.skipModule = driver.skipModule;
    this.skipModuleLength = this.skipModule.length;
  } else {
    this.skipModuleLength = 0;
  }
  if (driver.options.quiet == 'yes') {
    this.console = console;
    console = console_wrapper;
  }

  this.timeout = this.test['timeout'];
  if (!this.timeout) {
    this.timeout = driver.options['default-timeout'];
  }
  assert.assert(util.isNumber(this.timeout), "Timeout is not a number.");

  return this;
}

Runner.prototype.cleanup = function() {
  if (this.driver.options.quiet == 'yes') {
    console = this.console;
  }

  this.driver = null;
  this.test = null;
  if (this.timer != null) {
    clearTimeout(this.timer);
    this.timer = null;
  }
  if (this.timer_spin != null) {
    clearTimeout(this.timer_spin);
    this.timer_spin = null;
  }
};

Runner.prototype.spin = function() {
  var that = this;
  this.timer_spin = setTimeout(function() {
      var timerOnlyAlive = !testdriver.isAliveExceptFor(that.timer);
      if (timerOnlyAlive) {
        timerOnlyAlive = !process._onNextTick();
      }

      if (timerOnlyAlive) {
        process.exit(0);
      } else {
        if (!that.finished) {
          that.spin();
        }
      }
  }, 0);
};

Runner.prototype.checkSkipModule = function() {
  for (var i = 0; i < this.skipModuleLength; i++) {
    if (this.test['name'].indexOf(this.skipModule[i]) >= 0) {
      return true;
    }
  }

  return false;
};

Runner.prototype.checkSupportedModule = function() {
  // Cut off the '.js' ending
  var name = this.test['name'].slice(0, -3);

  // test_mod_smt -> [test, mod, smt]
  // test_modsmt -> [test, modsmt]
  var parts = name.split('_');
  if (parts[0] != 'test') {
    // test filename does not start with 'test_' so we'll just
    // assume we support it.
    return true;
  }

  // The second part of the array contains the module
  // which is under test
  var tested_module = parts[1];
  for (var i = 0; i < builtin_modules.length; i++) {
    if (tested_module == builtin_modules[i]) {
      return true;
    }
  }

  // In any other case we do not support this js file.
  return false;
}

Runner.prototype.run = function() {
  if (!this.checkSupportedModule()) {
    this.test.reason = 'unsupported module by iotjs build';
    this.finish('skip');
    return;
  }

  var skip = this.test['skip'];
  if (skip) {
    if ((skip.indexOf('all') >= 0) || (skip.indexOf(this.driver.os) >= 0)
      || (skip.indexOf(this.driver.stability) >= 0)) {
      this.finish('skip');
      return;
    }
  }

  if (this.skipModuleLength && this.checkSkipModule()) {
    this.test.reason = 'exclude module';
    this.finish('skip');
    return;
  }

  this.timer = null;

  var that = this;
  this.timer = setTimeout(function () {
    that.finish('timeout');
  }, this.timeout * 1000);

  try {
    var source = this.driver.test();
    eval(source);
  } catch(e) {
    if (this.test['expected-failure']) {
      this.finish('pass');
    } else if (this.test['uncaught']) {
      throw e;
    } else {
      console.error(e);
      this.finish('fail');
    }
  } finally {
    if (!this.finished) {
      this.spin();
    }
  }
};

Runner.prototype.finish = function(status) {
  if (this.finished)
    return;

  this.finished = true;
  process._exiting = false;

  this.driver.emitter.emit('nextTest', this.driver, status, this.test);
};

module.exports.Runner = Runner;
