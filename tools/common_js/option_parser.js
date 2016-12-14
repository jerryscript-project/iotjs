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

function Option(arg, value, default_value, help) {
  this.arg = arg;
  this.value = value;
  this.default_value = default_value;
  this.help = help;

  return this;
}

Option.prototype.printHelp = function() {
  console.log("\t" + this.arg + "=[" + this.value + "](default: " +
              this.default_value + ") : " + this.help);
}

function OptionParser() {
  this.options = [];
  return this;
}

OptionParser.prototype.addOption = function(arg, value, default_value, help) {
  var option  = new Option(arg, value, default_value, help);
  this.options.push(option);
}

OptionParser.prototype.parse = function() {
  var options = {};

  for (var idx in this.options) {
    var option = this.options[idx];
    var default_value = option.default_value;
    if (default_value !== "") {
      options[option.arg] = default_value;
    }
  }

  for (var aIdx = 2; aIdx < process.argv.length; aIdx++) {
    var option = process.argv[aIdx];
    var arg_val = option.split("=");

    if (arg_val.length != 2 || !arg_val[0] || !arg_val[1]) {
      return null;
    }

    var arg = arg_val[0];
    var val = arg_val[1];
    var found = false;

    for (var oIdx in this.options) {
      if (arg == this.options[oIdx].arg) {
        options[arg] = val;
        found = true;
        break;
      }
    }

    if (!found)
      return null;
  }

  return options;
}

OptionParser.prototype.printHelp = function() {
  console.log(process.argv[1]);
  console.log("\noptional arguments");
  for (var idx in this.options) {
    this.options[idx].printHelp();
  }
}




 module.exports.OptionParser = OptionParser;
