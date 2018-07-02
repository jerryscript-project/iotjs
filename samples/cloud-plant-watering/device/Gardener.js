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

var CWD = process.cwd(),
  Sensors = require(CWD + '/Sensors.js'),
  helpers = require(CWD + '/helpers.js'),
  GPIO = require('gpio'),
  gpio = new GPIO();

var Gardener = function (id, sensorConfiguration, valvePinConfiguration) {
    var t = Date.now();
    var g = this;

    this.id = id || helpers.genID(10);

    console.log('[' + this.id + '/' + t + '] starting up, hello!');

    this.airHumidity = 0;
    this.soilHumidity = 0;
    this.temperature = 0;
    this.light = 0;
    this.lastWatered = 0;
    this.hasWater = 0;
    this.uptime = 0;
    this.lastStep = Date.now();
    this.triggers = {};
    this.enabled = true;
    this.enabledTriggers = [];
    this.valveInitalized = false;
    this.valve = gpio.open({
      pin: valvePinConfiguration.pin,
      direction: gpio.DIRECTION.OUT,
      mode: gpio.MODE.NONE
    }, function (err) {
      if (err) {
        console.error(err);
      } else {
        g.valveInitalized = true;
      }
    });

    this.sensorData = {};
    this.sensors = new Sensors(sensorConfiguration);
  },
  proto = {};

function shouldTrigger(triggerName, gardener) {
  var property = gardener.sensorData[triggerName];
  var trigger = gardener.triggers[triggerName];
  return property !== undefined && typeof property !== 'object'
      && trigger !== undefined && trigger.min <= property
      && trigger.max >= property;
}

proto.step = function (timestamp, bypassWatering, callback) {
  var s = this.sensors;
  var id = this.id;
  var g = this;
  this.uptime += timestamp - this.lastStep;
  this.lastStep = timestamp;

  console.log('[' + id + '/' + timestamp + '] fetching sensor data');

  this.sensors.fetch(this.sensorData, function (err) {
    if (err) {
      console.error('[' + id + '/' + Date.now() + '] ' + err);
    } else {
      if (!bypassWatering && g.enabled) {
        console.log('[' + id + '/' + timestamp + '] watering not bypassed');
        var results = 0;
        for (var i = 0, l = g.enabledTriggers.length; i < l; ++i) {
          if (shouldTrigger(g.enabledTriggers[i], g)) {
            ++results;
          }
        }

        if (results === g.enabledTriggers.length) {
          console.log('[' + id + '/' + timestamp + '] watering plant');
          g.startWatering();
        } else {
          console.log('[' + id + '/' + timestamp + '] no matching triggers');
        }
      }
      callback(this);
    }
  });
};

proto.setTriggerOn = function (trigger) {
  if (this.enabledTriggers.indexOf(trigger) < 0) {
    this.enabledTriggers.push(trigger);
  }
};

proto.setTriggerOff = function (trigger) {
  var idx = this.enabledTriggers.indexOf(trigger);
  if (idx > -1) {
    this.enabledTrigers.splice(idx, 1);
  }
};

proto.setTriggerLevelRange = function (trigger, min, max) {
  console.log('[' + this.id + '/' + this.uptime + '] setting trigger '
              + trigger +  ' level ' + min + '/' + max);
  this.triggers[trigger] = min === false ? false : {min: min, max: max};
};

proto.getTriggerLevelRange = function (trigger) {
  if (this.triggers[trigger]) {
    return this.triggers[trigger];
  }

  return false;
};

proto.startWatering = function (time, callback) {
  var g = this;
  if (!time) {
    time = 500;
  }
  if (this.valveInitalized && !this.watering) {
    this.watering = true;
    this.valve.write(true, function (err) {
      if (err) {
        callback(err);
      } else {
        console.log('[' + g.id + '/' + g.uptime + '] watering plant for '
                    + time + ' miliseconds');
        g.lastWatered = Date.now();
        g.watering = false;
        setTimeout(function () {
          g.valve.write(false, function (err) {
            callback(err);
          });
        }, time);
      }
    });
  }
};

proto.enable = function () {
  this.enabled = true;
};

proto.disable = function () {
  this.enabled = false;
};

proto.toObject = function () {
  return {
    id: this.id,
    soilHumidity: this.sensorData.soilHumidity,
    light: this.sensorData.light,
    lastWatered: this.lastWatered,
    hasWater: this.sensorData.waterLevel,
    overflow: this.sensorData.overflow,
    uptime: this.uptime
  };
};

proto.toString = function () {
  return JSON.stringify(this.toObject());
};

Gardener.prototype = proto;

module.exports = Gardener;
