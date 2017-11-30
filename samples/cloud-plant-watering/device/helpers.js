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

var fs = require('fs');

module.exports.loadJSON = function (path) {
  var result = null,
    fileData = null;

  try {
    fileData = fs.readFileSync(path);
    result = JSON.parse(fileData);
  } catch (err) {
    // supress exception, just log
    console.error(err + ' ' + path);
  }

  return result;
};

module.exports.typeCast = function (param) {
  if (param === 'true' || param === 'false') {
    return param === 'true';
  }

  if (!isNaN(parseFloat(param))) {
    return parseFloat(param);
  }

  return param;
};

module.exports.genID = function(num) {
  var a = 'abcdefghijklmnoprstuwxyz',
    l = a.length,
    ret = '';

  while (--num >= 0) {
    ret += a[(Math.random() * l) | 0];
  }
  return ret;
};
