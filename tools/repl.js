/* Copyright 2019-present Samsung Electronics Co., Ltd. and other contributors
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
 * Minial REPL implementation.
 *
 * Notes:
 *  * Currently there is no support for multi-line code input.
 *  * No input history.
 */
var fs = require('fs');
var EventEmitter = require('events').EventEmitter;
var utils = require('util');

function stdin() {
  EventEmitter.call(this);
}

utils.inherits(stdin, EventEmitter);

stdin.prototype.start = function() {
  var self = this;
  var buffer = new Buffer(10);

  var read = function(buffer) {
    /* Read from stdin(0) */
    fs.read(0, buffer, 0, buffer.length, -1, after_read);
  };

  var after_read = function(err, bytes_read, buffer) {
    if (err) {
      throw err;
    };

    if (bytes_read === 0) {
      self.emit('end');
    } else {
      self.emit('data', buffer.slice(0, bytes_read));
      if (!process._exiting) {
        read(buffer);
      }
    }
  };

  read(buffer);
}

stdin.prototype.readline = function(callback) {
  var line_parts = [];

  this.on('data', function(data) {
    line_parts.push(data);

    /* Check if the last character is a '\n' */
    var has_line_end = (data.readInt8(data.length - 1) == 0x0A);
    if (has_line_end) {
      callback(Buffer.concat(line_parts).toString());
      line_parts = [];
    }
  });
};


function REPL() {
  this.input = new stdin();
  this._prompt_msg = new Buffer('> ');
};

REPL.prototype.print_prompt = function() {
  /* Write to stdout(1) */
  fs.writeSync(1, this._prompt_msg, 0, this._prompt_msg.length);
};

REPL.prototype.run_code = function(line) {
  var result;
  try {
    /* Doing indirect eval to force everything into the global object. */
    result = eval.call(undefined, line);
    console.log(result);
  } catch (ex) {
    console.error(ex);
  }
};

REPL.prototype.process_line = function(line) {
  this.run_code(line);
  this.print_prompt();
};

REPL.prototype.start = function() {
  /* Expose the "require" method for the global object.
   * This way the "eval" call can access it correctly.
   */
  global.require = require;

  this.print_prompt();
  this.input.start();
  this.input.readline(this.process_line.bind(this));
};

var repl = new REPL();
console.log('IoT.js (%s) Minimal REPL', process.version);
repl.start();
