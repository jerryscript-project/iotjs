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
var urlModule = require('url');
var Url = urlModule.Url;

var urls = [];
var urlObject = [];
var urlString = [];

urls.push(new Url());
urlString.push('https://www.example.com');
urls[0] = urlModule.parse(urlString[0], true);
urlObject.push(new Url());
urlObject[0].protocol = 'https';
urlObject[0].host = urlObject[0].hostname = 'www.example.com';

urls.push(new Url());
urlString.push('http://192.168.0.1:80');
urls[1] = urlModule.parse(urlString[1], true);
urlObject.push(new Url());
urlObject[1].protocol = 'http';
urlObject[1].host = '192.168.0.1:80';
urlObject[1].port = 80;
urlObject[1].hostname = '192.168.0.1';

urls.push(new Url());
urlString.push('https://user:password@server.com');
urls[2] = urlModule.parse(urlString[2], true);
urlObject.push(new Url());
urlObject[2].protocol = 'https';
urlObject[2].auth = 'user:password';
urlObject[2].host = urlObject[2].hostname = 'server.com';

var keys = Object.keys(urls[0]);
for (var i = 0; i < urls.length; ++i) {
  for (var j = 0; j < keys.length; ++j) {
    assert.equal(urls[i][keys[j]], urlObject[i][keys[j]],
        'test_url.js: urls[' + i + '].' + keys[j] + ' is not correct');
  }
}

for (i = 0; i < urlObject.length; ++i) {
  assert.equal(urlModule.format(urlObject[i]), urlString[i]);
}
