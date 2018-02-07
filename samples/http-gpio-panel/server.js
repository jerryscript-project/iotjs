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

/**
 * Description:
 *
 * This sample allows to control GPIO input/output pins through browser
 *
 * Usage:
 *
 * To run this example execute:
 *
 * $ iotjs server.js
 *
 * and navigate your browser to http://[your-ip-address]:8080
 *
 */
var http = require('http'),
  fs = require('fs'),
  Buffer = require('buffer'),
  gpio = require('gpio'),
  server = null,
  port = 8080,
  logs_enabled = true,
  pinConfiguration = {},
  activePins = {},
  GPIO_DIRECTION = {
    '0': gpio.DIRECTION.IN,
    '1': gpio.DIRECTION.OUT
  },
  GPIO_MODE = {
    '0': gpio.MODE.NONE,
    '1': gpio.MODE.PULLUP,
    '2': gpio.MODE.PULLDOWN,
    '3': gpio.MODE.FLOAT,
    '4': gpio.MODE.PUSHPULL,
    '5': gpio.MODE.OPENDRAIN
  };

// splits url by "/" deliminator and removes empty entries
function extractPath(url) {
  var urlParts = url.split('/'),
    i = 0,
    l = urlParts.length,
    result = [];
  for (; i < l; ++i) {
    if (urlParts[i].length > 0) {
      result.push(urlParts[i]);
    }
  }
  return result;
}

// processes request data stream and passes it to callback
function fetchRequestTextBody(req, callback) {
  var body = [];
  req.on('data', function (chunk) {
    body.push(chunk);
  });
  req.on('end', function () {
    callback(body.join(""));
  });
}

// sets 404 header and body as response
function notFound(res) {
  res.writeHead(404);
  res.end('404 not found');
}

// logs only when log_enabled is set to true
function log(/*...arg*/) {
  if (logs_enabled) {
    console.log.apply(console, [].slice.call(arguments));
  }
}

// reads file from specified path
function fetchFile(path) {
  var data = null;

  log('trying to open: ' + path);
  if (fs.existsSync(path)) {
    data = fs.readFileSync(path);
  }
  return data;
}

// synchronizes pin states with the data that was send with
// request from panel
function syncPins(pins) {
  var pin = '',
    updatedConf = {},
    updatedPins = {},
    value = 0;

  // update and add
  for (pin in pins) {
    if (pins.hasOwnProperty(pin)) {
      if (activePins[pin] === undefined) {
        // open pin if it does not exist
        log('opening new pin: ' + pin);
        activePins[pin] = gpio.open({
          pin: parseInt(pin, 10),
          direction: GPIO_DIRECTION[pins[pin].direction],
          mode: GPIO_MODE[pins[pin].mode]
        }, function (err) {
          if (err) {
            log('error while opening pin: ' + pin);
          } else {
            log('pin opened: ' + pin);
            if(parseInt(pins[pin].direction, 10) === 1) {
              activePins[pin].writeSync(parseInt(pins[pin].value, 10));
            }
          }
        });
      } else if(parseInt(pins[pin].direction, 10) === 1 &&
                pins[pin].value !== pinConfiguration[pin].value) {
        // update value if pin exists and is writable
        log('pin: ' + pin + ', new value: ' + parseInt(pins[pin].value, 10));
        activePins[pin].writeSync(parseInt(pins[pin].value, 10));
      }

      // save old value if pin exists
      if (pinConfiguration[pin] !== undefined) {
        value = pinConfiguration[pin].value;
      }

      // update
      pinConfiguration[pin] = pins[pin];

      // if pin is 'readable' then restore the value
      if(parseInt(pins[pin].direction, 10) === 0) {
        pinConfiguration[pin].value = value;
      }
    }
  }

  // handle pin removal
  for (pin in pinConfiguration) {
    if (pinConfiguration.hasOwnProperty(pin)) {
      if (pins[pin] !== undefined) {
        updatedConf[pin] = pinConfiguration[pin];
        updatedPins[pin] = activePins[pin];
      } else if (activePins[pin]) {
        log('closing pin: ' + pin);
        activePins[pin].closeSync();
      }
    }
  }

  // update internal data
  activePins = updatedPins;
  pinConfiguration = updatedConf;
}

function handleRequest(req, res) {
  var path = extractPath(req.url);
  switch (path[0]) {
    case 'update':
      fetchRequestTextBody(req, function (data) {
        syncPins(JSON.parse(data));
        res.writeHead(200);
        res.end(JSON.stringify(pinConfiguration));
      });
      break;
    case undefined:
      // front page
      path[0] = 'index.html';
    case 'favicon.ico':
      // serve favicon as most browsers always fetch this
      log('serving static: ' + path[0]);
      var fileData = fetchFile(process.cwd() + '/' + path[0]);
      if (fileData) {
        res.writeHead(200);
        res.end(fileData);
      } else {
        notFound(res);
      }
      break;
    default:
      // return 404 for all other requests
      notFound(res);
      break;
  }
}

// handles input pin state changes
setInterval(function () {
  var pin = '',
    value = null;
  for (pin in activePins) {
    if (activePins.hasOwnProperty(pin) &&
        parseInt(pinConfiguration[pin].direction, 10) === 0) { // check pin is
                                                               // if input pin
      value = activePins[pin].readSync() ? '1' : '0';
      if (pinConfiguration[pin].value !== value) { // check if value changed
        log('pin: ' + pin + ' value changed to: ' + value);
        pinConfiguration[pin].value = value;
      }
    }
  }
}, 500);

server = http.createServer(handleRequest);
server.listen(port, function (err) {
  if (err) {
    log('error while starting server: ' + err);
  } else {
    log('listening for connections on port: ' + port);
  }
});
