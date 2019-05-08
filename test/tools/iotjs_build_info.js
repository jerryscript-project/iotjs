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

/* Just for the testrunner to get runtime information about the build. */
var builtins = process.builtin_modules;

if (process.env.IOTJS_ENV.indexOf("experimental") > -1)
    stability = "experimental"
else
    stability = "stable"

/* Check if certain es2015 features are available */
function hasFeatures(object, features) {
  supported = true;

  for (feature in features) {
    supported = supported && object.hasOwnProperty(features[feature]);
  }

  return supported;
}

function hasArrowFunction() {
  try {
    eval("a => {}");
    return true;
  } catch(e) {}

  return false;
}

var features = {};

var typedArrayFeatures = [
  'Int8Array',
  'Uint8Array',
  'Uint8ClampedArray',
  'Int16Array',
  'Uint16Array',
  'Int32Array',
  'Uint32Array',
  'Float32Array',
  'Float64Array'
];

if (hasFeatures(global, ['Symbol']))
  features.Symbol = true;

if (hasFeatures(global, ['Promise']))
  features.Promise = true;

if (hasFeatures(global, ['ArrayBuffer']))
  features.ArrayBuffer = true;

if (hasFeatures(global, ['DataView']))
  features.DataView = true;

if (hasFeatures(global, typedArrayFeatures))
  features.TypedArray = true;

if (hasArrowFunction())
  features.ArrowFunction = true;

result = {
    'builtins': builtins,
    'features': features,
    'stability': stability,
    'debug': !!process.debug,
    'arch': process.arch
}

console.log(JSON.stringify(result))
