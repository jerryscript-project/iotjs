/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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
 * To export an object/value use the 'module.exports' object.
 */
var demo_value = "Hello";

/* Export an object with two properties. */
module.exports = {
    /* the 'native' means the object returned by the C init method. */
    demo2: function() { return native.message; },
    add: native.add
}

/* Export a local variable. */
module.exports.demo_value = demo_value;
