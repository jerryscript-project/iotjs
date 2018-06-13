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

/*
  For this test, we created a certificate authority, and signed
  a certificate request with it. Here are the steps taken:

  1) created a secret rsa key for ourselves

    openssl genrsa -out my_key.key 2048

    Verify (dump): openssl rsa -in my_key.key -noout -text

  2) created a secret rsa key for our certificate authority (CA)

    openssl genrsa -out my_ca.key 2048

    Verify (dump): openssl rsa -in my_ca.key -noout -text

  3) created a certificate signing request (CSR),
     which is signed by the CA later

    openssl req -new -key my_key.pem -out my_csr.csr

    Country Name (2 letter code) [AU]:HU
    State or Province Name (full name) [Some-State]:
    Locality Name (eg, city) []:Szeged
    Organization Name (eg, company) [Internet Widgits Pty Ltd]:Trusted
    Organizational Unit Name (eg, section) []:
    Common Name (e.g. server FQDN or YOUR name) []:localhost
    Email Address []:trusted@localhost

    Please enter the following 'extra' attributes
    to be sent with your certificate request
    A challenge password []:ch+llenGe
    An optional company name []:

    Verify (dump): openssl req -in my_csr.csr -noout -text

  4) created a self-signed certificate for the CA

    openssl req -new -x509 -key my_ca.key -out my_ca.crt

    Country Name (2 letter code) [AU]:HU
    State or Province Name (full name) [Some-State]:
    Locality Name (eg, city) []:Szeged
    Organization Name (eg, company) [Internet Widgits Pty Ltd]:My CA
    Organizational Unit Name (eg, section) []:
    Common Name (e.g. server FQDN or YOUR name) []:myca.org
    Email Address []:myca@myca.org

    Verify (dump): openssl x509 -in my_ca.crt -noout -text

  5) sign our certificate signing request

    openssl x509 -req -in my_csr.csr -CA my_ca.crt -CAkey my_ca.key \
        -CAcreateserial -out my_crt.crt

    Note: A my_ca.srl file is also created to record the already
          issued serial numbers. This file is needed for future
          certificate signings.

    Verify (dump): openssl x509 -in my_crt.crt -noout -text
*/

var tls = require('tls');
var assert = require('assert');
var fs = require('fs');

var port = 8080;

var server_message = '';

var options = {
  key: fs.readFileSync(process.cwd() + '/resources/my_key.key').toString(),
  cert: fs.readFileSync(process.cwd() + '/resources/my_crt.crt')
};

var server = tls.createServer(options, function(socket) {
  socket.write('Server hello');
}).listen(port);

var sockOpts = {
  // The my_crt.crt certificate was issued for localhost as server FQDN
  // Anything else here (e.g. 127.0.0.1) makes the handshake failed
  host: 'localhost',
  port: port,
  rejectUnauthorized: true,
  ca: fs.readFileSync(process.cwd() + '/resources/my_ca.crt')
}

var socket = tls.connect(sockOpts, function() {
  socket.on('data', function(data) {
    server_message += data.toString();
    socket.end();
    server.close();
  });
});

process.on('exit', function() {
  assert.equal(server_message, 'Server hello');
});
