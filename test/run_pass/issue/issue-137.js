/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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


// FIX: issue #137.
console.log('hello world');
console.log(1);
console.log(9 + 10);
console.log(null);
console.log(true);
console.log(false);
console.log(undefined);
console.log(0.09);
console.log(NaN);
console.log(function () {});
console.log(function f() { return 0; });
console.log({});
console.log({a:1, b:2});
console.log([]);
console.log([1,2,3]);
console.log("%s=%d", "1+10", 1 + 10);
console.log("%s+%s=%d", 2, 10, 2 + 10);
console.log("%d+%d=%d", 3, 10, 3 + 10);
console.log("%s+%s=%s", 4, 10, 4 + 10);
console.log("%d+%d=%d", "5", "10", 5 + 10);
