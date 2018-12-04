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
var crypto = require('crypto');
var fs = require('fs');

var hash = crypto.createHash('sha256');
hash.update('Hello IoT.js');

assert.equal(hash.digest('hex'),
             '23a1b938002c83a74b887d0a447c990' +
             '5e83bab00459bcfd0bad548623b942e20');

/*
  This test requires a key file, which can be generated using the following
  steps:
  openssl genrsa -aes128 -passout pass:<passphrase> -out private.pem 2048
  openssl rsa -in private.pem -passin pass:<passphrase> -pubout -out public.pem

  Recreating this test with your own data requires you to have (a) key file(s)
  with a public and a private key in them. The private key is used to sign the
  data.
  The public key is used to verify that the signature was done with the
  appropriate private key.

  To generate the rsa-sha256 signature you need to execute these lines:
  Creating an sha256 signature
  openssl dgst -sha256 -sign <privatekeyfile> -out <signed.sha256> <inputfile>
  Creating a base64 encoded signature
  openssl base64 -in <signed.sha256> -out <signature.sha256>

  To verify the signature, you need:
    - The <signature.sha256> file,
    - The public key, most likely from the public.pem file,
    - The <inputfile> which contains the data, or you can just copy and paste,
      the data and give it to the `verify.update()` function.

  Having created the `verify` object, just call the `.verify(pubkey, signature)`
  on it, and you are ready to go.
*/

var pubKey = fs.readFileSync(process.cwd() + '/resources/crypto_public.pem');
var verify = crypto.createVerify('sha256');
verify.update('Hello IoT.js\n');
var res = verify.verify(pubKey, 'JkFnOrBQGXYlpmlcMuS5EwyJ44WY/kW5sFwb8DRgAoo7' +
                                'RmxYPKgyo/OrZ0YAcY5xpxbVZzS7ftxz7N4q+Ouufg6s' +
                                'NSzmIimBPLV+afX4Qb8jOV0edCmeBKZaHQrMWpisWXF/' +
                                'bZKS1yiMij2NGSJYXWhjRzreIeVNVv/b0phHHeK2r2tT' +
                                '9T+XA+rdgHlIOb+r/FT/VWopr+vd+8I0fjxpP/S8lZ5u' +
                                'HSF9jZ5TFdIEYMWchKit4Eyw7/VAWRlJNNKVxTmuM337' +
                                '+rP9oLKiFUeoM6jrE10LxGnIpelvyNV+MHfo11I1GAMK' +
                                'jsOuye9JZ8/hQPg+KLWH/l/xZlUD2fZNNg==');
assert.equal(res, true);
