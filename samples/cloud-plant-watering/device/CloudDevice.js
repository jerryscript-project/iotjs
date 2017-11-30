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

var https = require('https'),
  querystring = require('querystring');

var CloudDevice = function (options) {
    this.deviceId = options.deviceId;
    this.deviceToken = options.deviceToken;
    this.cloudHostName = options.cloudHostName;
  },
  proto = CloudDevice.prototype;

function request(options, data, callback) {
  var req = https.request(options, function (res) {
    var responseData = '';
    res.on('data', function (incomming) {
      responseData += incomming.toString();
    });

    res.on('end', function () {
      callback(null, responseData);
    });

    res.on('error', function (err) {
      callback(err);
    });
  });

  if (data) {
    req.end(new Buffer(data));
  } else {
    req.end();
  }
};

proto.getActions = function (options, callback) {
  var query = {
    startDate: options.startDate || 0,
    ddid: this.deviceId
  };

  if (options.endDate) {
    query.endDate = options.endDate;
  }

  if (options.order) {
    query.order = options.order;
  }

  if (options.count) {
    query.count = options.count;
  }

  request({
    hostname: this.cloudHostName,
    method: 'GET',
    path: '/v1.1/actions?' + querystring.encode(query),
    headers: {
      'Authorization': 'Bearer ' + this.deviceToken,
      'Content-Type': 'application/json',
      'Accept': 'application/json',
      'User-Agent': 'gardener/1.1'
    }
  },
  null,
  function (err, data) {
    var result = [];
    if (err) {
      callback(err);
    } else if (!data) {
      callback('no data');
    } else {
      var parsed = JSON.parse(data.toString());
      if (parsed.error) {
        callback(parsed.error.message);
      } else {
        if (data.size > 0) {
          result = data.data;
        }
        callback(null, result);
      }
    }
  });
};

proto.postMessage = function (data, callback) {
  var message = JSON.stringify({
    sdid: this.deviceId,
    data: data
  });

  request({
    hostname: this.cloudHostName,
    method: 'POST',
    path: '/v1.1/messages',
    headers: {
      'Content-Length': message.length,
      'Authorization': 'Bearer ' + this.deviceToken,
      'Content-Type': 'application/json',
      'Accept': 'application/json',
      'User-Agent': 'gardener/1.1'
    }
  },
  message,
  function (err, data) {
    if (err) {
      callback(err);
    } else if (!data) {
      callback('no data');
    } else {
      var parsed = JSON.parse(data.toString());
      if (parsed.error) {
        callback(parsed.error.message);
      } else {
        callback(null, parsed);
      }
    }
  });
};

module.exports = CloudDevice;
