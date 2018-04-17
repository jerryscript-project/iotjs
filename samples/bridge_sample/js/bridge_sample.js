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

var Bridge = require('bridge');

function bridge_sample(){
    this.bridge = new Bridge(native.MODULE_NAME);
}

bridge_sample.prototype.getResPath = function(){
    return this.bridge.sendSync('getResPath', '');
};

bridge_sample.prototype.getSystemInfo = function(callback){
    this.bridge.send('getSystemInfo', '', function(err, msg){
        callback(err, msg);
    });
};

bridge_sample.prototype.testThread = function(callback){
    this.bridge.send('testThread', '', function(err, msg){
        callback(err, msg);
    });
};

module.exports = new bridge_sample();
