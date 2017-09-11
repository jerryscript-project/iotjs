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
var native_modules = Object.keys(process.native_sources)
var binding_modules = Object.keys(process.binding)
var builtins = native_modules.concat(binding_modules)

if (process.env.IOTJS_ENV.indexOf("experimental") > -1)
    stability = "experimental"
else
    stability = "stable"

result = {
    'builtins': builtins,
    'stability': stability
}

console.log(JSON.stringify(result))
