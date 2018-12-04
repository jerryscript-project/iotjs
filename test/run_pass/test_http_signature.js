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

var fs = require('fs');
var http_sign = require('http_signature');
var assert = require('assert');

var key = fs.readFileSync(process.cwd() + '/resources/http_signature_key.key');

// This is an example request of a Samsung C2C demo
var sampleRequest = { "headers": {
              "Content-Type":"application/json",
              "Accept":"application/json",
              "Authorization":"Signature keyId=\"http_signature_key\"," +
              "signature=\"mJUG2ceYWCGww2tXrVJywQUqQvaWbkWpAx4xSfqZD9Gr" +
              "G12N8pVysa/nl18kEKe2Sbd00c50qyF/xH5hKtxFyyUYBxY5cOrdt+7W" +
              "1EmctaGGIDOnZA/qZcXcnTBZsp8k68XI/6HxwIxHVUntAd2vxJvqzibB" +
              "TZLHAhTRVCoAqHzjHe0kybv5oebbMASaNEhZTLslQYQUOYqVzE+4Ecen" +
              "Vxrlk2wpjjFjnBdxd/Ek34FTOcWMoPKjpj1ja+hfet2Em8YzF+aeHrBR" +
              "t7FTt7r/GkYfuwm9M0XYSY1JvnvCKxIU20YXKbZ+KINBaUXDwEKapUvm" +
              "bDFuLi3arJcDigWIOA==\",headers=\"(request-target) digest" +
              " date\",algorithm=\"rsa-sha256\"",
              "Date":"Tue, 28 Aug 2018 15:28:59 UTC",
              "Digest":"SHA-256=52eIrPP0TxhlUVwnChuVLj6qFmbl5dYdMIvUr+DlZ0A=",
              "X-ST-CORRELATION":"b7891162-2084-da6e-da84-401be50cd534",
              "X-B3-TraceId":"fb04fc10f3088f10",
              "X-B3-SpanId":"c6245b1a3c934a0d",
              "X-B3-ParentSpanId":"989731b1f545e7d1",
              "X-B3-Sampled":"1",
              "content-length":"344",
              "host":"example.host.com",
              "user-agent":"AHC/2.1",
              "X-Forwarded-Proto":"https",
              "X-Forwarded-For":"52.14.6.245" },
            "method": "POST",
            "url": "/",
            "httpVersion": "1.1",
          };


var parsedRequest = http_sign.parseRequest(sampleRequest);
assert.equal(http_sign.verifySignature(parsedRequest, key), true);
