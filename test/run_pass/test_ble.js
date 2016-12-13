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

var ble = require('ble');

ble.on('stateChange', function(state){
  console.log('onStateChange: ' + state);

  if (state == 'poweredOn') {
    ble.startAdvertising('test', ['180F'], function(err) {
      if (err) console.log(err);
      else {
        console.log('advertising start success.');

        setTimeout(function() {
          ble.stopAdvertising(function(err) {
            if (err) console.log(err);
            else console.log('stop advertising.');
          });
        }, 5000);
      }
    });
  } else {
    ble.stopAdvertising(function(err) {
      if (err) console.log(err);
      else console.log('stop advertising.');
    });
  }
});
