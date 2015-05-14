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


var buffer = process.binding(0);
var alloc = buffer.alloc;
var kMaxLength = buffer.kMaxLength;
var util = require('util');

function Buffer(subject, encoding) {
  if (!util.isBuffer(this)) {
    return new Buffer(subject, encoding);
  }

  if (util.isNumber(subject)) {
    this.length = subject > 0 ? subject >>> 0 : 0;
  } else if (util.isString(subject)) {
    this.length = Buffer.byteLength(subject);
  }

  alloc(this, this.length);

  if (util.isString(subject)) {
    this.write(subject, encoding);
  } else if (util.isBuffer(subject)) {
    subject.copy(this, 0, 0, this.length);
  }
}

buffer.setupBufferJs(Buffer);
