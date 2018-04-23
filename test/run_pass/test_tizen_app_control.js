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

var tizen = require('tizen');

// Need to set target APP_ID e.g) app.control.receiver
var app_id = '';
var data = {
  str: 'iotjs'
};

// test: tizen.on
tizen.on('appControl', function(msg) {
  console.log('appControl', msg);

  var extra_data = msg.extra_data;
  if(extra_data && extra_data.key === 'iotjs' ) {

    //test: tizen.launchAppControl
    try {
      var res = tizen.launchAppControl({
        app_id: app_id,
        extra_data: data,
      });
      console.log('Result', res);
    } catch(e) {
      console.log(e);
    }
  }
});
