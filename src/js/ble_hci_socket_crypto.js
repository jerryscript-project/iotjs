/* Copyright 2016-present Samsung Electronics Co., Ltd. and other contributors
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

/* Copyright (C) 2015 Sandeep Mistry sandeep.mistry@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

var crypto = require('ble_hci_socket_crypto');

function r() {
  return crypto.randomBytes(16);
}

function c1(k, r, pres, preq, iat, ia, rat, ra) {
  var p1 = Buffer.concat([
    iat,
    rat,
    preq,
    pres
  ]);

  var p2 = Buffer.concat([
    ra,
    ia,
    new Buffer('00000000', 'hex')
  ]);

  var res = xor(r, p1);
  res = e(k, res);
  res = xor(res, p2);
  res = e(k, res);

  return res;
}

function s1(k, r1, r2) {
  return e(k, Buffer.concat([
    r2.slice(0, 8),
    r1.slice(0, 8)
  ]));
}

function e(key, data) {
  key = swap(key);
  data = swap(data);

  var cipher = crypto.createCipheriv('aes-128-ecb', key, '');
  cipher.setAutoPadding(false);

  return swap(Buffer.concat([
    cipher.update(data),
    cipher.final()
  ]));
}

function xor(b1, b2) {
  var result = new Buffer(b1.length);

  for (var i = 0; i < b1.length; i++) {
    //result[i] = b1[i] ^ b2[i];
    result.writeUInt8(b1.readUInt8(i) ^ b2.readUInt8(i), i);
  }

  return result;
}

function swap(input) {
  var output = new Buffer(input.length);

  for (var i = 0; i < output.length; i++) {
    //output[i] = input[input.length - i - 1];
    output.writeUInt8(input.readUInt8(input.length - i - 1), i);
  }

  return output;
}

module.exports = {
  r: r,
  c1: c1,
  s1: s1,
  e: e
};
