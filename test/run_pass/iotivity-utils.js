/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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

function getRandomSeconds(min, max) {
  return (Math.random() * (max - min) + min) * 1000;
}
module.exports.getRandomSeconds = getRandomSeconds;

function payloadAssert(payload, object) {
  if (JSON.stringify(payload) != JSON.stringify(object)) {
    return true;
  }
  return false;
}
module.exports.payloadAssert = payloadAssert;

function findResource(response, uuid) {
  var index, resources,
    returnValue = false;

  if (response &&
    response.payload &&
    response.payload.resources &&
    response.payload.resources.length) {

    resources = response.payload.resources;

    for (index in resources) {
      if (resources[index].uri === "/a/" + uuid) {
        return true;
      }
    }
  }

  return returnValue;
}
module.exports.findResource = findResource;


function checkBitFields(enumName, value) {
  var index,
    bits = {};

  for (index in enumName) {
    if (value & enumName[index]) {
      bits[index] = true;
    }
  }
  return bits;
}
module.exports.checkBitFields = checkBitFields;


var filename_, fd_;
var fs = require('fs');
var persistent = {

  open: function (filename, mode) {
    var fd;

    mode = mode.replace(/b/g, "");
    try {
      if (filename !== filename_) {
        fd = fs.openSync(filename, "wx");
        var res = fs.closeSync(fd);
        filename_ = filename;
      }
    }
    catch (err) {
      if (err.message.substr(0, 6) !== "EEXIST") {
        throw err;
      }
    }
    fd = fs.openSync(filename, mode);
    fd_ = fd;
    fd = (fd === undefined ? -1 : fd);
    return fd;
  },
  close: function (fp) {
    fs.closeSync(fp);
    return 0;
  },
  read: function (buffer, totalSize, fp) {
    var result = fs.readSync(fp, buffer, 0, totalSize, null);
    return result;
  },
  write: function (buffer, totalSize, fp) {
    return fs.writeSync(fp, buffer, 0, totalSize, null);
  },
  unlink: function (path) {
    fs.unlinkSync(path);
    return 0;
  },
  clean: function() {
    fs.unlinkSync("oic_svr_db.dat");
  }
};
module.exports.persistent = persistent;
