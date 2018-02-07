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
 * This sample runs a simple REPL mode console on the device UART port
 *
 * Usage:
 *
 * To run this sample, connect a UART device (ex. FTDI USB-UART, RPI UART pins)
 * to RX/TX pin on the artik board (0 and 1 pin on CON709). Please note that
 * the serial device used in this example is different than normal USB port on
 * the Artik053 development board, so you need two connections, one to run
 * the code and second to connect to REPL console. After connecting please run:
 *
 * $ iotjs console.js
 *
 * You can now run simple JS code and the device will evaluate it and return
 * the results
 */

var uart = require('uart'),
  uartConfig = {
    device: '/dev/ttyS1',
    baudRate: 115200,
    dataBits: 8
  },
  buffer = [],
  serialDevice = null,
  log_enabled = true,
  MSG_INFO = 0,
  MSG_ERROR = 1,
  EVALUATE_CODE_CHR = 18, // CTRL+R <CR>
  uartResponseCodes = { // chars to send on specific input char codes
    8: '\b',
    13: '\r\n'
  },
  fakeConsole = { // fake console to allow sending console messages to user
    messages: []
  };

// on linux the device is different
if (process.platform === 'linux' ||
    (process.iotjs && process.iotjs.board === 'RP2')) {
  uartConfig.device = '/dev/serial0';
}

// tries to 'stringify' objects (and errors)
function obj2str(obj) {
  if (obj instanceof Error) {
    return obj.name + ': ' + obj.message;
  }

  return JSON.stringify(obj);
}

// stringify and array of data (ex. arguments of functions)
function arr2str(arr) {
  var strArr = [],
    i = 0,
    l = arr.length;
  for (; i < l; ++i) {
    switch (typeof arr[i]) {
      case 'object':
        strArr.push(obj2str(arr[i]));
        break;
      case 'function':
        strArr.push('function');
        break;
      default:
      case 'string':
      case 'number':
        strArr.push(arr[i]);
        break;
    }
  }
  return strArr.join('');
}

fakeConsole.log = function (/*...args*/) {
  var body = arr2str([].slice.call(arguments));
  log('LOG: ' + body);
  this.messages.push(body);
};

fakeConsole.error = function (/*...args*/) {
  var body = arr2str([].slice.call(arguments));
  log('ERROR: ' + body);
  this.messages.push(body);
};

fakeConsole.toString = function () {
  return this.messages.join('\r\n') + '\r\n';
};

fakeConsole.clear = function () {
  this.messages = [];
};

// logs only if log_enabled flag is set to true
function log(/*...args*/) {
  if (log_enabled) {
    console.log.apply(console, [].slice.call(arguments));
  }
}

// faleConsole needs to be available to 'eval'ed scripts
global.fakeConsole = fakeConsole;

// execude code with 'eval'
// this is done only for this sample, normally using eval is a no-no,
// please avoid if possible
function evalInContext(data) {
  data = data.replace(/console\.(log|error)/g, 'fakeConsole.$1');
  eval(data);
}

// handles code thats to be evaluated and sends response to uart device
function handleCode(code) {
  log('evaluating: >>>\r\n ' + code + ' \r\n>>>EOT');
  try {
    evalInContext(code);
  } catch (err) {
    fakeConsole.error(err);
  }

  serialDevice.write(fakeConsole.toString(), function (err) {
    if (err) {
      log('error while sending console data: ' + err);
    } else {
      fakeConsole.clear();
    }
  });
}

// handles data received from uart device
function handleData(data) {
  var arr = data.split(''),
    chrCode = 0,
    chr = '',
    i = 0,
    l = data.length,
    localBuff = buffer;

  for (;i < l; ++i) {
    chr = arr[i];
    chrCode = chr.charCodeAt(0);

    if (chrCode === 8) { // handle backspace
      localBuff.splice(localBuff.length - 1, 1);
      serialDevice.writeSync('\b \b'); // move back, erase by space, move back
    } else if ((chrCode > 31 && chrCode < 127) || chrCode === 13) {
      // and only visible chars and new lines
      localBuff.push(chr);
      serialDevice.writeSync(uartResponseCodes[chrCode] || chr);
    }

    if (chrCode === EVALUATE_CODE_CHR) { // evaluate code on EVALUATE_CODE_CHR
      handleCode(localBuff.join(''));
      localBuff = []; // clear buffer after code evaluation
    }
  }

  buffer = localBuff;
}

process.on('uncaughtException', function (err) {
  // code errors need to be cached and redirected to uart console
  log('uncaught exception: ' + err);
  fakeConsole.error(err);
});

serialDevice = uart.open(uartConfig, function (err) {
  if (err) {
    log('could not opend device: ' + uartConfig.device + ', reason: ' + err);
  } else {
    log('waiting for user input');
    serialDevice.on('data', handleData);
  }
});
