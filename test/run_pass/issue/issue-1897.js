/* Copyright 2019-present Samsung Electronics Co., Ltd. and other contributors
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

var websocket = require('websocket');
var client = new websocket.Websocket();
var client2 = new websocket.Websocket();
var wss = new websocket.Server({port: 8081}, function () {});

client.connect('ws://localhost', 8081, '/', function() {
  client2.connect('ws://localhost', 8081, '/');
  client2.on('open', function() {
    wss.broadcast('a');
    // prevent blocking
    wss.close();
  });
});
