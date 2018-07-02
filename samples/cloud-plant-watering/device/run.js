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
  helpers = require(CWD + '/helpers.js'),
  configuration = helpers.loadJSON(CWD + '/configuration.json'),
  DEVICE_TYPE_ID = "dt20bd5c58972d4db5aac281dfea0b1648",
  CloudDevice = require(CWD + '/CloudDevice.js'),
  cdev1 = new CloudDevice({
    deviceId: configuration.device_id,
    deviceToken: configuration.device_token,
    cloudHostName: configuration.hostname
  }),
  Gardener = require(CWD + '/Gardener.js'),
  loopInterval = -1,
  g1 = null;

// hack
var n = Date.now;
Date.now = function () {
  return Math.floor(n());
};

process.on('exit', function () {
  if (loopInterval) {
    clearInterval(loopInterval);
  }
});

console.log('retreiving configuration');

var options = configuration.sensorDefaultTriggers;

g1 = new Gardener(configuration.device_name,
                  configuration.sensorPinConfiguration,
                  configuration.valvePinConfiguration);

var option = '';
var val = null;
for (option in options) {
  if (options.hasOwnProperty(option)) {
    if (options[option] === false) {
      g1.setTriggerOff(option);
    } else {
      g1.setTriggerOn(option);
      val = options[option];
      if (typeof val === 'object') {
        g1.setTriggerLevelRange(option, val.min, val.max);
      } else {
        g1.setTriggerLevelRange(option, val);
      }
    }
  }
}

function run(interval) {
  // instead of a loop the script should be waken up and with a specific
  // interval, this would benefit with batteries
  loopInterval = setTimeout(function () {
    cdev1.getActions({ // check for pending actions
      startDate: g1.lastStep,
      endDate: Date.now()
    }, function (error, data) {
      var watered = false;
      if (error) {
        console.error(error);
      } else if (data.length > 0) {
        var action = null;
        for (var i = 0, l = data.length; i < l; ++i) {
          if (data[i].data.actions) {
            for (var j = 0, m = data[i].data.actions.length; j < m; ++j) {
              action = data[i].data.actions[j];
              switch (action.name) {
                case 'setTriggerLevelRange':
                  g1.setTriggerLevelRange(
                    action.parameters.trigger,
                    helpers.typeCast(action.parameters.min),
                    helpers.typeCast(action.parameters.max)
                  );
                  break;
                case 'setOff':
                  g1.disable();
                  break;
                case 'setOn':
                  g1.enable();
                  break;
                case 'setTriggerOff':
                  g1.setTriggerOff(action.parameters.trigger);
                  break;
                case 'setTriggerON':
                  g1.setTriggerOff(action.parameters.trigger);
                  break;
                case 'startWatering':
                  g1.startWatering(helpers.typeCast(action.parameters.time),
                                   function (err) {
                    if (err) {
                      console.error(err);
                    }
                  });
                  watered = true;
                  break;
              }
            }
          }
        }
      }

      g1.step(Date.now(), watered, function () {
        console.log('Cloud sync...');
        cdev1.postMessage(g1.toObject(), function (error) {
          if (error) {
            console.error(error);
          } else {
            console.log('Done');
          }
          run(interval);
        });
      });
    });
  }, interval || 2000);
}

run(options.interval);
