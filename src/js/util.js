/* Copyright 2015 Samsung Electronics Co., Ltd.
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

function isNull(arg) {
  return arg === null;
};
exports.isNull = isNull;


function isUndefined(arg) {
  return arg === undefined;
};
exports.isUndefined = isUndefined;


function isNullOrUndefined(arg) {
  return isNull(arg) || isUndefined(arg);
};
exports.isNullOrUndefined = isNullOrUndefined;


function isNumber(arg) {
  return typeof arg === 'number';
};
exports.isNumber = isNumber;


function isString(arg) {
  return typeof arg === 'string';
};
exports.isString = isString;


function isObject(arg) {
  return typeof arg === 'object' && arg != null;
};
exports.isObject = isObject;


function isFunction(arg) {
  return typeof arg === 'function';
};
exports.isFunction = isFunction;


function isBuffer(arg) {
  return arg instanceof Buffer;
};
exports.isBuffer = isBuffer;


exports.isArray = Array.isArray;


function inherits(ctor, superCtor) {
  ctor.prototype = new superCtor();
};
exports.inherits = inherits;
