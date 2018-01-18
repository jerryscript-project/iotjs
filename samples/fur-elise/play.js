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
 * Plays Bethovens Fur Elise using PWM
 *
 * Usage:
 *
 * To run this sample please connect a low-power speaker, like a buzzer
 * (piezoelectric speaker), negative feed (-) to GND and positive feed (+) to
 * pin 7 on Artik053 CON703, and run the code by executing
 *
 * $ iotjs play.js
 *
 */

var pwm = require('pwm'),
  // note indexes definition
  // please remember that D# is same as Bb here
  notes = {
    "C": 0,
    "C#": 1,
    "D": 2,
    "D#": 3,
    "E": 4,
    "F": 5,
    "F#": 6,
    "G": 7,
    "G#": 8,
    "A": 9,
    "Bb": 10,
    "B": 11
  },
  // note frequencies
  frequencies = [
    //C, C#, D, Eb, E, F, F#, G, G#, A, Bb, B in ocatves from 0 to 8
    [16.35, 17.32, 18.35, 19.45, 20.60, 21.83, 23.12, 24.50, 25.96, 27.50,
      29.14, 30.87],
    [32.70, 34.65, 36.71, 38.89, 41.20, 43.65, 46.25, 49.00, 51.91, 55.00,
      58.27, 61.74],
    [65.41, 69.30, 73.42, 77.78, 82.41, 87.31, 92.50, 98.00, 103.8, 110.0,
      116.5, 123.5],
    [130.8, 138.6, 146.8, 155.6, 164.8, 174.6, 185.0, 196.0, 207.7, 220.0,
      233.1, 246.9],
    [261.6, 277.2, 293.7, 311.1, 329.6, 349.2, 370.0, 392.0, 415.3, 440.0,
      466.2, 493.9],
    [523.3, 554.4, 587.3, 622.3, 659.3, 698.5, 740.0, 784.0, 830.6, 880.0,
      932.3, 987.8],
    [1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976],
    [2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951],
    [4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902]
  ],
  // fur elise notes
  song = [
    ["E6", 0.2], ["D#6", 0.2], ["E6", 0.2], ["D#6", 0.2], ["E6", 0.2],
    ["B5", 0.2], ["D6", 0.2], ["C6", 0.2], ["A5", 0.2], ["A3", 0.2],
    ["E4", 0.2], ["A4", 0.2], ["C5", 0.2], ["E5", 0.2], ["A5", 0.2],
    ["E3", 0.2], ["B5", 0.2], ["E4", 0.2], ["G#4", 0.2], ["E5", 0.2],
    ["G#5", 0.2], ["B5", 0.2], ["A3", 0.2], ["C6", 0.2], ["E4", 0.2],
    ["A4", 0.2], ["E5", 0.2], ["E6", 0.2], ["E6", 0.2], ["D#6", 0.2],
    ["D#6", 0.2], ["E6", 0.2], ["E6", 0.2], ["D#6", 0.2], ["D#6", 0.2],
    ["E6", 0.2], ["E6", 0.2], ["B5", 0.2], ["B5", 0.2], ["D6", 0.2],
    ["D6", 0.2], ["C6", 0.2], ["C6", 0.2], ["A3", 0.2], ["A5", 0.2],
    ["A5", 0.2], ["E4", 0.2], ["A4", 0.2], ["C5", 0.2], ["E5", 0.2],
    ["A5", 0.2], ["E3", 0.2], ["B5", 0.2], ["E4", 0.2], ["G#4", 0.2],
    ["E5", 0.2], ["C6", 0.2], ["B5", 0.2], ["A5", 0.2], ["A3", 0.2],
    ["E4", 0.2], ["A4", 0.2], ["B5", 0.2], ["C6", 0.2], ["D6", 0.2],
    ["C4", 0.2], ["E6", 0.2], ["G4", 0.2], ["C5", 0.2], ["G5", 0.2],
    ["F6", 0.2], ["E6", 0.2], ["G3", 0.2], ["D6", 0.2], ["G4", 0.2],
    ["B4", 0.2], ["F5", 0.2], ["E6", 0.2], ["D6", 0.2], ["A3", 0.2],
    ["C6", 0.2], ["E4", 0.2], ["A4", 0.2], ["E5", 0.2], ["D6", 0.2],
    ["C6", 0.2], ["E3", 0.2], ["B5", 0.2], ["E4", 0.4],
    ["E5", 0.2], ["E6", 0.2], ["E5", 0.4], ["E6", 0.2],
    ["E7", 0.2], ["D#6", 0.2], ["E6", 0.2], ["D#6", 0.2], ["E6", 0.2],
    ["D#6", 0.2], ["E6", 0.2], ["D#6", 0.2], ["E6", 0.2], ["D#6", 0.2],
    ["E6", 0.2], ["D#6", 0.2], ["E6", 0.2], ["B5", 0.2], ["D6", 0.2],
    ["C6", 0.2], ["A3", 0.2], ["A5", 0.2], ["E4", 0.2], ["A4", 0.2],
    ["C5", 0.2], ["E5", 0.2], ["A5", 0.2], ["E3", 0.2], ["B5", 0.2],
    ["E4", 0.2], ["G#4", 0.2], ["E5", 0.2], ["G#5", 0.2], ["B5", 0.2],
    ["A3", 0.2], ["C6", 0.2], ["E4", 0.2], ["A4", 0.2], ["E5", 0.2],
    ["E6", 0.2], ["D#6", 0.2], ["E6", 0.2], ["D#6", 0.2], ["E6", 0.2],
    ["B5", 0.2], ["D6", 0.2], ["C6", 0.2], ["A3", 0.2], ["A5", 0.2],
    ["E4", 0.2], ["A4", 0.2], ["C5", 0.2], ["E5", 0.2], ["A5", 0.2],
    ["E3", 0.2], ["B5", 0.2], ["E4", 0.2], ["G#4", 0.2], ["E5", 0.2],
    ["C6", 0.2], ["B5", 0.2], ["A3", 0.2], ["A5", 0.2], ["E4", 0.2],
    ["A4", 0.8]
  ],
  log_enable = true,
  device = null;

// log only when log_enable flag is set to true
function log(/*...args*/) {
  if (log_enable) {
    console.log.apply(console, [].slice.call(arguments));
  }
}

// extracts frequency from freq array based on supplied note
function note2freq(noteStr) {
  var matches = noteStr.match(/([a-zA-Z\#]+)([0-9]+)/i),
    freq = 0;

  if (matches && matches.length === 3) {
    return frequencies[parseInt(matches[2], 10)][notes[matches[1]]];
  }

  return 0;
}

// sets pwm period and runs callback after specified length of time
function setPeriod(period, length, callback) {
  log('period: ' + period + ', length: ' + length + ' ms');
  device.setPeriod(period, function (err) {
    if (err) {
      callback(err);
    } else {
      setTimeout(callback, length);
    }
  });
}

// plays each note of song recursively and runs callback on end
function playSong(song, callback, currentNote) {
  var idx = currentNote === undefined ? 0 : currentNote,
    freq = 0;
  if (idx < song.length) {
    freq = note2freq(song[idx][0]);
    // period = 1 second / frequency
    setPeriod(freq !== 0 ? 1 / freq : 0.5, 1000 * song[idx][1],
             playSong.bind(null, song, callback, ++idx));
  } else {
    callback();
  }
}

device = pwm.open({
  pin: 0,
  dutyCycle: 0.5,
  period: 1 / 10
}, function (err) {
  if (err) {
    log('could not open pwm device: ' + err);
  } else {
    device.setEnableSync(true);
    log('playing song');
    playSong(song, function () {
      device.close(function (err) {
        if (err) {
          log('error while closing device: ' + err);
        } else {
          log('done');
        }
      });
    });
  }
});
