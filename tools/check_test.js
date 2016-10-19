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

var fs = require('fs');
var Runner = require('test_runner').Runner;
var Logger = require('common_js/logger').Logger;
var OptionParser = require('common_js/option_parser').OptionParser;

var EventEmitter = require('events').EventEmitter;

var root = 'test';
var parent = '..';

function absolutePath(path) {
  // FIXME: On NuutX side, when dealing with file, path should be absolute.
  // So workaround this problem, test driver converts relative path
  // to absolute one.
  return process.cwd() + '/' + path;
}

function Driver() {
  this.emitter = new EventEmitter();
  this.emitter.addListener('nextTest', function(driver, status, attr) {
    if (driver.runner) {
      driver.runner.cleanup();
    }
    var filename = driver.filename();
    if (status == 'pass') {
      driver.logger.message('PASS : ' + filename, status);
    } else if (status == 'fail') {
      driver.logger.message('FAIL : ' + filename, status);
    } else if (status == 'skip') {
      driver.logger.message('SKIP : ' + filename +
                   '   (reason : ' + attr.reason + ")", status);
    } else if (status == 'timeout') {
      driver.logger.message('TIMEOUT : ' + filename, status);
    }
    driver.fIdx++;
    driver.runNextTest();
  });

  this.logger = new Logger();

  this.os = process.platform;
  this.board = process.iotjs.board;

  this.root = absolutePath(root);
  process.chdir(this.root);

  return this;
}

Driver.prototype.config = function() {
  if (this.os == 'linux') {
    var config = 'test/config-largemem'
  } else {
    var config = 'test/config-smallmem'
  }

  var testset = require(config);
  if (config == 'test/config-smallmem') {
    var parser = new OptionParser();

    parser.addOption('index', "an index to test, refer to *config-smallmem*");
    var options = parser.parse();
    if (options.index) {
      var idx = options.index;
    } else {
      parser.printHelp();
      return false;
    }
  }
  this.tests = testset(idx);

  this.dIdx = 0;
  this.dLength = Object.keys(this.tests).length;

  this.nextTestSet();
  return true;
}

Driver.prototype.getAttrs = function() {
  var content = fs.readFileSync(absolutePath('attrs.js')).toString();
  var attrs = eval(content);

  var dirname = this.dirname();
  if (dirname == 'run_fail') {
    var files = this.tests[dirname];
    for (var fIdx in files) {
      var file = files[fIdx];
      if (!attrs[file]) {
        attrs[file] = {};
      }
      attrs[file].fail = true;
    }
  }

  return attrs;
};

Driver.prototype.runNextTest = function() {
  if (this.dIdx == this.dLength) {
    this.finish();
  } else {
    if (this.fIdx == this.fLength) {
      process.chdir(this.root);
      this.dIdx++;
      if (this.dIdx == this.dLength) {
        this.finish();
      } else {
        this.nextTestSet();
        this.runNextTest();
      }
    } else {
      this.runner = new Runner(this);
      this.runner.run();
    }
  }
};

Driver.prototype.nextTestSet = function() {
  this.fIdx = 0;
  var dirname = this.dirname();
  this.fLength = this.tests[dirname].length;
  process.chdir(absolutePath(dirname));
  this.attrs = this.getAttrs();
  this.logger.message("\n");
  this.logger.message(">>>> " + dirname, "summary");
}

Driver.prototype.dirname = function() {
  return Object.keys(this.tests)[this.dIdx]
};

Driver.prototype.filename = function() {
  var dirname = this.dirname();
  var filename = this.tests[dirname][this.fIdx];
  return filename;
}

Driver.prototype.test = function() {
  var filename = this.filename();
  var content =  fs.readFileSync(absolutePath(filename)).toString();
  return content;
};

Driver.prototype.finish = function() {
  this.logger.message('finish all tests', this.logger.summary);
}

var driver = new Driver();

process.exit = function(code) {
  // this function is called when the following happens.
  // 1. the test case calls explicitly process.exit .
  // 2. when the test case has attribute 'exit', so the runner calls.
  // 3. assertion is failed.
  try {
    process.emitExit(code);
  } catch(e) {
    // run_fail/tests with exit attribute should be reached here.
    process.removeAllListeners('exit');

    if (driver.runner.attr.fail) {
      driver.runner.finish('pass');
    } else {
      driver.runner.finish('fail');
    }
  } finally {
    process.removeAllListeners('exit');

    if (code != 0) {
      driver.runner.finish('fail');
    } else {
      driver.runner.finish('pass');
    }
  }
}

var conf = driver.config();
if (conf) {
  driver.runNextTest();
}
