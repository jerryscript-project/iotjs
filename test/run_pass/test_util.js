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

var util = require('util');
var assert = require('assert');

assert.equal(util.isNull(null), true);
assert.equal(util.isNull(0), false);
assert.equal(util.isNull('null'), false);


assert.equal(util.isUndefined(undefined), true);
assert.equal(util.isUndefined(0), false);
assert.equal(util.isUndefined('undefined'), false);


assert.equal(util.isNumber(0), true);
assert.equal(util.isNumber(5001), true);
assert.equal(util.isNumber(3.14), true);
assert.equal(util.isNumber(-5001), true);
assert.equal(util.isNumber('5000'), false);
assert.equal(util.isNumber(null), false);
assert.equal(util.isNumber([0,1,2,3,4,5]), false);


assert.equal(util.isFinite(5001), true);
assert.equal(util.isFinite(-5001), true);
assert.equal(util.isFinite(3.14), true);
assert.equal(util.isFinite(-3.14), true);
assert.equal(util.isFinite(0), true);
assert.equal(util.isFinite(Infinity), false);


assert.equal(util.isBoolean(true), true);
assert.equal(util.isBoolean(false), true);
assert.equal(util.isBoolean(Boolean(5001)), true);
assert.equal(util.isBoolean(Boolean(0)), true);
assert.equal(util.isBoolean(Boolean(-5001)), true);
assert.equal(util.isBoolean(Boolean('Hello IoT.js')), true);


assert.equal(util.isString('Hello IoT.js'), true);
assert.equal(util.isString(['Hello IoT.js']), false);
assert.equal(util.isString(-5001), false);
assert.equal(util.isString(5001), false);
assert.equal(util.isString(null), false);


var object = {
  value1: 0,
  value2: 5001,
  value3: -5001,
  value4: 3.14,
  value5: 'Hello IoT.js'
};
assert.equal(util.isObject(object), true);
assert.equal(util.isObject({obj: 5001}), true);
assert.equal(util.isObject({obj: object}), true);
assert.equal(util.isObject({}), true);
assert.equal(util.isObject([5001]), true);
assert.equal(util.isObject(['Hello IoT.js']), true);
assert.equal(util.isObject(null), false);
assert.equal(util.isObject(5001), false);
assert.equal(util.isObject('Hello IoT.js'), false);


var func1 = function() {};
assert.equal(util.isFunction(func1), true);
assert.equal(util.isFunction(function(arg) {/*do nothing*/}), true);
assert.equal(util.isFunction(null), false);
assert.equal(util.isFunction(5001), false);
assert.equal(util.isFunction([5001]), false);
assert.equal(util.isFunction([func1]), false);
assert.equal(util.isFunction({}), false);
assert.equal(util.isFunction('Hello IoT.js'), false);


var buff = new Buffer('Hello IoT.js');
assert.equal(util.isBuffer(buff), true);
assert.equal(util.isBuffer(new Buffer(5001)), true);
assert.equal(util.isBuffer(5001), false);
assert.equal(util.isBuffer({}), false);
assert.equal(util.isBuffer('5001'), false);
assert.equal(util.isBuffer([5001]), false);
assert.equal(util.isBuffer([buff]), false);
assert.equal(util.isBuffer({obj: buff}), false);


function Parent() {}
function Child() {}
util.inherits(Child, Parent);
var child = new Child();
assert.equal(child instanceof Parent, true);
assert.equal(child instanceof Buffer, false);


assert.equal(util.format(), '');
assert.equal(util.format(''), '');
assert.equal(util.format(null), 'null');
assert.equal(util.format(true), 'true');
assert.equal(util.format(false), 'false');
assert.equal(util.format('Hello IoT.js'), 'Hello IoT.js');
assert.equal(util.format(5001), '5001');

assert.equal(util.format('%d', 5001.5), '5001.5');
assert.equal(util.format('Hello IoT.js - %d', 5001), 'Hello IoT.js - 5001');
assert.equal(
  util.format('%s IoT.js - %d', 'Hello', 5001),
  'Hello IoT.js - 5001'
);
assert.equal(util.format('%d%%', 5001), '5001%');

var json = {
  "first":"1st",
  "second":"2nd"
};
assert.equal(
  util.format('%s: %j', 'Object', json),
  'Object: {"first":"1st","second":"2nd"}'
);
assert.equal(
  util.format('%d-%j-%s', 5001, json, 'IoT.js', 'end'),
  '5001-{"first":"1st","second":"2nd"}-IoT.js end'
);
json.json = json;
assert.equal(util.format('%j', json), '[Circular]');

assert.equal(util.format('%s', '5001'), '5001');
assert.equal(util.format('%j', '5001'), '"5001"');
assert.equal(util.format('%d%d', 5001), '5001%d');
assert.equal(util.format('%s%d%s%d', 'IoT.js ', 5001), 'IoT.js 5001%s%d');
assert.equal(util.format('%d%% %s', 100, 'IoT.js'), '100% IoT.js');
assert.equal(util.format(new Error('format')), 'Error: format');

var err1 = util.errnoException(3008, 'syscall', 'original message');
assert.equal(err1 instanceof Error, true);
assert.equal(err1, 'Error: syscall error original message');
assert.equal(err1.code, 'error');
assert.equal(err1.errno, 'error');
assert.equal(err1.syscall, 'syscall');

var err2 = util.errnoException(1, 'getSyscall');
assert.equal(err2 instanceof Error, true);
assert.equal(err2, 'Error: getSyscall error');
assert.equal(err2.code, 'error');
assert.equal(err2.errno, 'error');
assert.equal(err2.syscall, 'getSyscall');


var err3 = util.exceptionWithHostPort(1,
                                      'syscall',
                                      '127.0.0.1',
                                      5001,
                                      'additional info');
assert.equal(err3 instanceof Error, true);
assert.equal(
  err3,
  'Error: syscall error 127.0.0.1:5001 - Local (additional info)'
);
assert.equal(err3.code, 'error');
assert.equal(err3.errno, 'error');
assert.equal(err3.syscall, 'syscall');
assert.equal(err3.address, '127.0.0.1');
assert.equal(err3.port, 5001);

var err4 = util.exceptionWithHostPort(3008,
                                      'getSyscall',
                                      '127.0.0.1');
assert.equal(err4 instanceof Error, true);
assert.equal(
  err4,
  'Error: getSyscall error 127.0.0.1'
);
assert.equal(err4.code, 'error');
assert.equal(err4.errno, 'error');
assert.equal(err4.syscall, 'getSyscall');
assert.equal(err4.address, '127.0.0.1');
