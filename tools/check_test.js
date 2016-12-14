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

var fs = require('fs');
var Runner = require('test_runner').Runner;
var Logger = require('common_js/logger').Logger;
var OptionParser = require('common_js/option_parser').OptionParser;
var util = require('common_js/util');
// FIXME: After fs.readDirSync is implemented, this should be replaced.
var testsets = require('test/testsets');

var EventEmitter = require('events').EventEmitter;

var root = 'test';
var parent = '..';

function Driver() {
  this.results = {
    pass: 0,
    fail: 0,
    skip: 0,
    timeout: 0,
  };

  this.emitter = new EventEmitter();
  this.emitter.addListener('nextTest', function(driver, status, attr) {
    if (driver.runner) {
      driver.runner.cleanup();
    }
    var filename = driver.filename();

    if (status == 'pass') {
      driver.results.pass++;
      driver.logger.message('PASS : ' + filename, status);
    } else if (status == 'fail') {
      driver.results.fail++;
      driver.logger.message('FAIL : ' + filename, status);
    } else if (status == 'skip') {
      driver.results.skip++;
      driver.logger.message('SKIP : ' + filename +
                   '   (reason : ' + attr.reason + ")", status);
    } else if (status == 'timeout') {
      driver.results.timeout++;
      driver.logger.message('TIMEOUT : ' + filename, status);
    }
    driver.fIdx++;
    driver.runNextTest();
  });

  this.os = process.platform;
  this.board = process.iotjs.board;

  this.root = util.absolutePath(root);
  process.chdir(this.root);

  return this;
}

Driver.prototype.config = function() {
  var parser = new OptionParser();

  parser.addOption('start-from', "", "",
    "a test case file name where the driver starts.");
  parser.addOption('quiet', "yes|no", "yes",
    "a flag that indicates if the driver suppresses " +
    "console outputs of test case");
  parser.addOption('output-file', "", "",
    "a file name where the driver leaves output");
  parser.addOption('skip-module', "", "",
    "a module list to skip test of specific modules");

  var options = parser.parse();

  if (options == null) {
    parser.printHelp();
    return false;
  }

  var output = options['output-file'];
  if (output) {
    if (this.os == 'nuttx') {
      var path = util.join('/mnt/sdcard', output);
    } else {
      var path = util.join(this.root, '..', output);
    }
    fs.writeFileSync(path, new Buffer(''));
  }
  var skipModule = options['skip-module'];
  if (skipModule) {
    this.skipModule = skipModule.split(',');
  }

  this.logger = new Logger(path);

  this.options = options;

  this.tests = testsets();

  this.dIdx = 0;
  this.dLength = Object.keys(this.tests).length;

  var skipped = this.skipTestSet(options['start-from']);

  this.nextTestSet(skipped);
  return true;
};

Driver.prototype.getAttrs = function() {
  var content = fs.readFileSync(util.absolutePath('attrs.js')).toString();
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

Driver.prototype.skipTestSet = function(filename) {
  if (!filename)
    return false;

  var dLength = this.dLength;
  for (var dIdx = 0; dIdx < dLength; dIdx++) {
    var dirname = Object.keys(this.tests)[dIdx];
    var dir = this.tests[dirname];
    var fLength = dir.length;
    for (var fIdx = 0; fIdx < fLength; fIdx++) {
      if (dir[fIdx] == filename) {
        this.fIdx = fIdx;
        this.dIdx = dIdx;
        return true;
      }
    }
  }

  return false;
};

Driver.prototype.nextTestSet = function(skipped) {
  if (!skipped) {
    this.fIdx = 0;
  }

  var dirname = this.dirname();
  this.fLength = this.tests[dirname].length;
  process.chdir(util.absolutePath(dirname));
  this.attrs = this.getAttrs();
  this.logger.message("\n");
  this.logger.message(">>>> " + dirname, "summary");
};

Driver.prototype.dirname = function() {
  return Object.keys(this.tests)[this.dIdx]
};

Driver.prototype.filename = function() {
  var dirname = this.dirname();
  var filename = this.tests[dirname][this.fIdx];
  return filename;
};

Driver.prototype.test = function() {
  var filename = this.filename();
  var content =  fs.readFileSync(util.absolutePath(filename)).toString();
  return content;
};

Driver.prototype.finish = function() {
  this.logger.message('\n\nfinish all tests', this.logger.status.summary);

  this.logger.message('PASS : ' + this.results.pass, this.logger.status.pass);
  this.logger.message('FAIL : ' + this.results.fail, this.logger.status.fail);
  this.logger.message('TIMEOUT : ' +
    this.results.timeout, this.logger.status.timeout);
  this.logger.message('SKIP : ' + this.results.skip, this.logger.status.skip);

  if (this.results.fail > 0 || this.results.timeout > 0) {
    originalExit(1);
  }
};

var driver = new Driver();

var originalExit = process.exit;
process.exit = function(code) {
  // this function is called when the following happens.
  // 1. the test case is finished normally.
  // 2. assertion inside the callback function is failed.
  try {
    process.emitExit(code);
  } catch(e) {
    // when assertion inside the process.on('exit', function { ... }) is failed,
    // this procedure is executed.
    process.removeAllListeners('exit');

    if (driver.runner.attr.fail) {
      driver.runner.finish('pass');
    } else {
      console.error(e);
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
};

var conf = driver.config();
if (conf) {
  driver.runNextTest();
}
