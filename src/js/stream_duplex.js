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


var util = require('util');
var Readable = require('stream_readable');
var Writable = require('stream_writable');


function Duplex(options) {
  if (!(this instanceof Duplex)) {
    return new Duplex(options);
  }

  Readable.call(this, options);
  Writable.call(this, options);
}

// Duplex inherits Readable.
util.inherits(Duplex, Readable);


// Duplex should also inherits Writable but there are no way for inheriting
// from multiple parents. Copy properties from Writable.
var keys = Object.keys(Writable.prototype);
for (var i = 0; i < keys.length; ++i) {
  var key = keys[i];
  if (!Duplex.prototype[key]) {
    Duplex.prototype[key] = Writable.prototype[key];
  }
}


module.exports = Duplex;
