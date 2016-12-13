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
var assert = require('assert');

var path = '../resources/readdir'
var ans = 'DO_NOT_MODIFY_THIS_FOLDER\n'+
          'This_is_a_directory\n'+
          'This_is_another_directory\n'+
          'regular.txt\n';

var res;
var items;

res = "";
items = fs.readdirSync(path);
items.sort();
for (i = 0; i < items.length; i++)
  res += items[i] + '\n';
assert.equal(res, ans);

res = "";
fs.readdir(path, function(err, items) {
  assert.equal(err, null);
  items.sort();
  for (i = 0; i < items.length; i++)
    res += items[i] + '\n';
  assert.equal(res, ans);
});
