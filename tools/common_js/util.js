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

function absolutePath(path) {
  // FIXME: On NuttX side, when dealing with file, path should be absolute.
  // So workaround this problem, test driver converts relative path
  // to absolute one.
  return process.cwd() + '/' + path;
}

function join() {
  var path = Array.prototype.join.call(arguments, '/');
  return path;
}

module.exports.absolutePath = absolutePath;
module.exports.join = join;
