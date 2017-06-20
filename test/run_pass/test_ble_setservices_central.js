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

/* Copyright (C) 2015 Sandeep Mistry sandeep.mistry@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

var noble = require('../..');

var echoServiceUuid = 'ec00';
var echoCharacteristicUuid = 'ec0e';

noble.on('stateChange', function(state) {
  if (state === 'poweredOn') {
    //
    // Once the BLE radio has been powered on, it is possible
    // to begin scanning for services. Pass an empty array to
    // scan for all services (uses more time and power).
    //
    console.log('scanning...');
    noble.startScanning([echoServiceUuid], false);
  }
  else {
    noble.stopScanning();
  }
})

var echoService = null;
var echoCharacteristic = null;

noble.on('discover', function(peripheral) {
  // we found a peripheral, stop scanning
  noble.stopScanning();

  //
  // The advertisment data contains a name, power level (if available),
  // certain advertised service uuids, as well as manufacturer data,
  // which could be formatted as an iBeacon.
  //
  console.log('found peripheral:', peripheral.advertisement);
  //
  // Once the peripheral has been discovered, then connect to it.
  // It can also be constructed if the uuid is already known.
  ///
  peripheral.connect(function(err) {
    //
    // Once the peripheral has been connected, then discover the
    // services and characteristics of interest.
    //
    peripheral.discoverServices([echoServiceUuid], function(err, services) {
      services.forEach(function(service) {
        //
        // This must be the service we were looking for.
        //
        console.log('found service:', service.uuid);

        //
        // So, discover its characteristics.
        //
        service.discoverCharacteristics([], function(err, characteristics) {

          characteristics.forEach(function(characteristic) {
            //
            // Loop through each characteristic and match them to the
            // UUIDs that we know about.
            //
            console.log('found characteristic:', characteristic.uuid);

            if (echoCharacteristicUuid == characteristic.uuid) {
              echoCharacteristic = characteristic;
            }
          })

          //
          // Check to see if we found all of our characteristics.
          //
          if (echoCharacteristic) {
            bakePizza();
          }
          else {
            console.log('missing characteristics');
          }
        })
      })
    })
  })
})

function bakePizza() {
  var crust = new Buffer("Hello BLE Service.");
  echoCharacteristic.write(crust, false, function(err) {
    if (!err) {
      echoCharacteristic.read(function (err, buffer) {
        if (!err) {
          console.log('BLE says' + buffer.toString());
        }
      });
    }
    else {
      console.log('crust error');
    }
  })
}
