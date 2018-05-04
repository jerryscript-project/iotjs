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
var EventEmitter = require('events').EventEmitter;
var util = require('util');

var APP_CONTROL_EVENT_TYPE = 'appControl';

var BUNDLE_KEY_OPERATION = '__APP_SVC_OP_TYPE__';
var BUNDLE_KEY_URI = '__APP_SVC_URI__';
var BUNDLE_KEY_MIME = '__APP_SVC_MIME_TYPE__';
var BUNDLE_KEY_LAUNCH_MODE = '__APP_SVC_LAUNCH_MODE__';
var BUNDLE_KEY_CATEGORY = '__APP_SVC_CATEGORY__';
var BUNDLE_KEY_PACKAGE = '__APP_SVC_PKG_NAME__';

var bridge = new Bridge(native.MODULE_NAME);

var startsWith = function(searchString, position) {
  position = position || 0;
  return this.substr(position, searchString.length) === searchString;
};


function ApplicationControl(bundle) {
  if (!util.isObject(bundle)) {
    return this;
  }

  this.operation = bundle[BUNDLE_KEY_OPERATION];
  this.uri = bundle[BUNDLE_KEY_URI];
  this.mime = bundle[BUNDLE_KEY_MIME];
  this.launch_mode = bundle[BUNDLE_KEY_LAUNCH_MODE];
  this.category = bundle[BUNDLE_KEY_CATEGORY];
  this.app_id = bundle[BUNDLE_KEY_PACKAGE];

  var extra_data = {};
  for (var prop in bundle) {
    if (bundle.hasOwnProperty(prop) && !startsWith.call(prop, '__')) {
      extra_data[prop] = bundle[prop];
    }
  }
  this.extra_data = Object.keys(extra_data).length ? extra_data : undefined;

  return this;
}


function Bundle(appcontrol) {
  if (!util.isObject(appcontrol)) {
    return this;
  }

  var bundle = this;

  if (util.isString(appcontrol.operation)) {
    bundle[BUNDLE_KEY_OPERATION] = appcontrol.operation;
  }

  if (util.isString(appcontrol.uri)) {
    bundle[BUNDLE_KEY_URI] = appcontrol.uri;
  }

  if (util.isString(appcontrol.mime)) {
    bundle[BUNDLE_KEY_MIME] = appcontrol.mime;
  }

  if (util.isString(appcontrol.launch_mode)) {
    if (appcontrol.launch_mode === 'group' ||
        appcontrol.launch_mode === 'single') {
      bundle[BUNDLE_KEY_LAUNCH_MODE] = appcontrol.launch_mode;
    }
  }

  if (util.isString(appcontrol.category)) {
    bundle[BUNDLE_KEY_CATEGORY] = appcontrol.category;
  }

  if (util.isString(appcontrol.app_id)) {
    bundle[BUNDLE_KEY_PACKAGE] = appcontrol.app_id;
  }

  var extra_data = appcontrol.extra_data;
  if (util.isObject(extra_data)) {
    for (var prop in extra_data) {
      if (extra_data.hasOwnProperty(prop)) {
        // a bundle is a dictionary which consists of key and value pairs
        bundle[prop] = extra_data[prop].toString();
      }
    }
  }

  return this;
}


function on(type, listener) {
  var callback = listener;

  if (type === APP_CONTROL_EVENT_TYPE) {
    // replace callback for application control
    callback = function(msg) {
      if (util.isString(msg)) {
        try {
          var json = JSON.parse(msg);
          listener(new ApplicationControl(json));
        } catch (e) {
          return; // ignore msg. data should be a json string
        }
      }
    };
  }

  return EventEmitter.prototype.on.call(this, type, callback);
}


function launchAppControl(option) {
  var bundle = new Bundle(option);
  return 'OK' === bridge.sendSync('launchAppControl', JSON.stringify(bundle));
}


var getResPath = function() {
  return bridge.sendSync('getResPath', '');
};


var getDataPath = function() {
  return bridge.sendSync('getDataPath', '');
};


module.exports = util.mixin(native, EventEmitter.prototype, {
  launchAppControl: launchAppControl,
  getResPath: getResPath,
  getDataPath: getDataPath,
  on: on,
});
