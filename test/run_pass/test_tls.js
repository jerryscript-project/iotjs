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

var assert = require('assert');
var tls = require('tls');
var util = require('util');

assert.assert(util.isFunction(tls.TLSSocket),
              'tls does not provide \'TLSSocket\' function');

assert.assert(util.isFunction(tls.connect),
             'tls does not provide \'connect\' function');

assert.assert(util.isFunction(tls.createSecureContext),
             'tls does not provide \'createSecureContext\' function');

assert.assert(util.isFunction(tls.checkServerIdentity),
             'tls does not provide \'checkServerIdentity\' function');

assert.assert(util.isFunction(tls.createServer),
             'tls does not provide \'createServer\' function');
