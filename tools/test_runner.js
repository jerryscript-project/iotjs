/* Copyright 2016 Samsung Electronics Co., Ltd.
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

var testdriver = require('testdriver');

function Runner(driver) {
  process._exiting = false;

  this.driver = driver;
  var filename = driver.filename();
  this.attr = driver.attrs[filename] || {};
  this.finished = false;

  return this;
};

Runner.prototype.cleanup = function() {
  this.driver = null;
  this.attr = null;
  if (this.timer != null) {
    clearTimeout(this.timer);
    this.timer = null;
  }
}

Runner.prototype.spin = function() {
  var that = this;
  process.nextTick(function() {
      var timerOnlyAlive = !testdriver.isAliveExceptFor(that.timer);
      if (timerOnlyAlive) {
        if (that.attr.exit) {
          process.exit(0);
        } else if (that.attr.fail) {
          that.finish('fail');
        } else {
          that.finish('pass');
        }
      } else {
        if (!that.finished) {
          that.spin();
        }
      }
  });
}

Runner.prototype.run = function() {
  if (this.attr.skip && (this.attr.skip.indexOf('all') >= 0 ||
      this.attr.skip.indexOf(this.driver.os) >= 0)) {
      this.finish('skip');
      return;
  }

  this.timer = null;
  if (this.attr.timeout) {
    var timeout = this.attr.timeout['all'] || this.attr.timeout[this.driver.os];
    if (timeout) {
      var that = this;
      this.timer = setTimeout(function () {
        that.finish('timeout');
      }, timeout * 1000);
    }
  }

  try {
    var source = this.driver.test();
    eval(source);
  } catch(e) {
    if (this.attr.fail) {
      this.finish('pass');
    } else if (this.attr.uncaught) {
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
}

Runner.prototype.finish = function(status) {
  if (this.finished)
    return;

  this.finished = true;

  this.driver.emitter.emit('nextTest', this.driver, status, this.attr);
}

module.exports.Runner = Runner;
