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

var GPIO = require('gpio'),
  ADC = require('adc'),
  gpio = new GPIO();

function Sensors(configuration) {
  this.soilHumidity = gpio.open({
    pin: configuration.soilHumidity.pin,
    mode: gpio.MODE.NONE,
    direction: gpio.DIRECTION.IN
  }, function (err) {
    if (err) {
      throw err;
    }
  });

  this.overflow = gpio.open({
    pin: configuration.overflow.pin,
    mode: gpio.MODE.NONE,
    direction: gpio.DIRECTION.IN
  }, function (err) {
    if (err) {
      throw err;
    }
  });

  this.light = gpio.open({
    pin: configuration.light.pin,
    mode: gpio.MODE.NONE,
    direction: gpio.DIRECTION.IN
  }, function (err) {
    if (err) {
      throw err;
    }
  });

  this.waterLevel = gpio.open({
    pin: configuration.waterLevel.pin,
    mode: gpio.MODE.NONE,
    direction: gpio.DIRECTION.IN
  }, function (err) {
    if (err) {
      throw err;
    }
  });
};

function runOnAll(asyncFuncList, callback) {
  var i = 0,
    errs = [],
    res = [],
    clbk = function (err, data) {
      ++i;
      errs.push(err);
      res.push(data);
      if (i === asyncFuncList.length) {
        callback(errs, res);
      } else {
        asyncFuncList[i](clbk);
      }
    };
  asyncFuncList[i](clbk);
}

Sensors.prototype.fetch = function (output, callback) {
  runOnAll([
    this.light.read.bind(this.light),
    this.overflow.read.bind(this.overflow),
    this.waterLevel.read.bind(this.waterLevel),
    this.soilHumidity.read.bind(this.soilHumidity)
  ], function (errors, results) {
    var value = 0;
    for (var i = 0, l = results.length; i < l; ++i) {
      if (errors[i]) {
        callback(errors[i]);
        break;
      } else {
        if (typeof results[i] === 'boolean') {
          value = results[i] ? 1 : 0;
        } else {
          value = results[i];
        }
        switch (i) {
          case 0:
            output.light = value;
            break;
          case 1:
            output.overflow = value;
            break;
          case 2:
            output.waterLevel = value;
            break;
          case 3:
            output.soilHumidity = value;
            break;
        }
      }
    }
    callback(null);
  });
};

module.exports = Sensors;
