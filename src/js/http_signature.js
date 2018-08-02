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

var crypto = require('crypto');

function parseRequest(request) {
  var authHeader = false;
  var authType = false;
  if (request.headers['authorization']) {
    authType = 'authorization';
    authHeader = request.headers['authorization'];
  } else if (request.headers['Authorization']) {
    authType = 'Authorization';
    authHeader = request.headers['Authorization'];
  }

  if (!authHeader) {
    throw new Error('Authorization header is not present, invalid request.');
  }

  var requestHeaders = {
    'request-line': request.method + ' ' + request.url + ' HTTP/' +
                    request.httpVersion,
    '(request-target)': '(request-target): ' + request.method.toLowerCase() +
                        ' ' + request.url,
  };

  for (var key in request.headers) {
    if (key !== authType) {
      var keyData = key.toLowerCase();
      requestHeaders[keyData] = keyData + ': ' + request.headers[key];
    }
  }

  var authObject = {};

  var idx = 0;
  var types = ['keyId=', 'signature=', 'algorithm=', 'headers='];
  // TODO: We currently only accept data that's enclosed with double quotes.
  // The newest RFC doesn't state if the data in the authorization header needs
  // to be in double quotes, or just simply be after the equals sign.
  // reference: https://tools.ietf.org/html/draft-cavage-http-signatures-10
  for (var i = 0; i < types.length; i++) {
    if ((idx = authHeader.indexOf(types[i])) < 0) {
      throw new Error('Couldn\'t find header: ', types[i]);
    }

    idx += types[i].length + 1;
    var endIdx = authHeader.substring(idx).indexOf('"') + idx;
    authObject[types[i].slice(0, -1)] = authHeader.substring(idx, endIdx);
  }

  var parsedRequest = {
    requestObject: requestHeaders,
    authObject: authObject,
  };

  return parsedRequest;
}

function verifySignature(parsedRequest, pubKey) {
  // We only support RSA-SHAX signatures right now
  var algorithm = parsedRequest.authObject.algorithm.toLowerCase();
  if (algorithm.indexOf('rsa-sha') < 0) {
    throw new Error('Only rsa-shaX signatures are supported');
  }

  // We know it begins with rsa-sha, so give only the sha info to crypto
  var toVerify = crypto.createVerify(algorithm.split('-')[1]);
  var headersToHash = parsedRequest.authObject.headers.split(' ');

  for (var i = 0; i < headersToHash.length; i++) {
    toVerify.update(parsedRequest.requestObject[headersToHash[i]]);
    // 2.1.2.3 If value is not the last value then append an ASCII newline `\n`.
    // The string MUST NOT include a trailing ASCII newline.
    if (i + 1 != headersToHash.length) {
      toVerify.update('\n');
    }
  }

  return toVerify.verify(pubKey, parsedRequest.authObject.signature);
}

exports.verifySignature = verifySignature;
exports.parseRequest = parseRequest;
