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

var assert = require('assert');
var common = require('common.js');

var prop_module = require('./build/' + common.buildTypePath +
                          '/test_napi_properties.node');

var obj = {
  array: [
    1, 94, 'str', 12.321, { test: 'obj in arr' }
  ],
  num: 123,
  subObj: {
    test: 'obj in obj'
  },
  str: 'hello'
};

var names = prop_module.GetNames(obj);
var keys = Object.keys(obj);
assert.strictEqual(names.length, keys.length);
for (var i = 0; i < keys.length; i++) {
  assert(prop_module.HasProperty(obj, keys[i]));
  assert.strictEqual(names[i], keys[i]);
}

assert.strictEqual(prop_module.GetProperty(obj, 'unkown'), undefined);
assert(!prop_module.HasProperty(obj, 'unkown'));
assert.strictEqual(prop_module.GetProperty(obj, 'num'), 123);
assert(prop_module.SetProperty(obj, 'num', 321));
assert.strictEqual(prop_module.GetProperty(obj, 'num'), 321);
assert.strictEqual(prop_module.GetProperty(obj, 'str'), 'hello');

assert(!prop_module.HasProperty(obj, 'newProp'));
assert(prop_module.SetProperty(obj, 'newProp', 'newValue'));
assert(prop_module.HasProperty(obj, 'newProp'));
assert.strictEqual(prop_module.GetProperty(obj, 'newProp'), 'newValue');

assert(prop_module.DeleteProperty(obj, 'newProp'));
assert(!prop_module.HasProperty(obj, 'newProp'));

/* Test prototype chain */
function Person(first, last, age, eyecolor) {
  this.firstName = first;
  this.lastName = last;
}

Person.prototype.name = function() {
  return this.firstName + " " + this.lastName;
};

var person = new Person('John', 'Doe', 99, 'blue');

assert(prop_module.HasProperty(person, 'name'));
assert(prop_module.HasProperty(person, 'firstName'));
assert(prop_module.HasProperty(person, 'lastName'));
assert(!prop_module.HasOwnProperty(person, 'name'));
assert(prop_module.HasOwnProperty(person, 'firstName'));
assert(prop_module.HasOwnProperty(person, 'lastName'));

assert(prop_module.DeleteProperty(Person.prototype, 'name'));
assert(!prop_module.HasProperty(person, 'name'));
assert(prop_module.HasProperty(person, 'firstName'));
assert(prop_module.HasProperty(person, 'lastName'));
