/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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

var EventEmitter = require('events').EventEmitter;
var assert = require('assert');


var emitter = new EventEmitter();

var eventCnt1 = 0;
var eventCnt2 = 0;
var eventCnt3 = 0;
var onceCnt = 0;
var eventSequence = "";

emitter.once('once', function() {
  onceCnt += 1;
});


assert.equal(onceCnt, 0);
emitter.emit('once');
assert.equal(onceCnt, 1);
emitter.emit('once');
assert.equal(onceCnt, 1);


emitter.once('once2', function() {
  onceCnt += 1;
  assert.equal(arguments.length, 14);
  assert.equal(arguments[0], 0);
  assert.equal(arguments[1], 1);
  assert.equal(arguments[2], 2);
  assert.equal(arguments[3], 3);
  assert.equal(arguments[4], 4);
  assert.equal(arguments[5], 5);
  assert.equal(arguments[6], 6);
  assert.equal(arguments[7], 7);
  assert.equal(arguments[8], 8);
  assert.equal(arguments[9], 9);
  assert.equal(arguments[10], "a");
  assert.equal(arguments[11], "b");
  assert.equal(arguments[12], "c");
  assert.equal(arguments[13].a, 123);
});

assert.equal(onceCnt, 1);
emitter.emit('once2', 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, "a", "b", "c", { a: 123});
assert.equal(onceCnt, 2);
emitter.emit('once2', 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, "a", "b", "c", { a: 123});
assert.equal(onceCnt, 2);

emitter.addListener('event', function() {
  eventCnt1 += 1;
  eventSequence += "1";
});

assert.equal(eventCnt1, 0);
emitter.emit('event');
assert.equal(eventCnt1, 1);

emitter.addListener('event', function() {
  eventCnt2 += 1;
  eventSequence += "2";
});

assert.equal(eventCnt2, 0);
emitter.emit('event');
assert.equal(eventCnt1, 2);
assert.equal(eventCnt2, 1);

emitter.addListener('event', function() {
  eventCnt3 += 1;
  eventSequence += "3";
});

assert.equal(eventCnt3, 0);
emitter.emit('event');
assert.equal(eventCnt1, 3);
assert.equal(eventCnt2, 2);
assert.equal(eventCnt3, 1);
emitter.emit('event');
assert.equal(eventCnt1, 4);
assert.equal(eventCnt2, 3);
assert.equal(eventCnt3, 2);
emitter.emit('no receiver');
assert.equal(eventCnt1, 4);
assert.equal(eventCnt2, 3);
assert.equal(eventCnt3, 2);


emitter.addListener('args', function() {
  assert.equal(arguments.length, 14);
  assert.equal(arguments[0], 0);
  assert.equal(arguments[1], 1);
  assert.equal(arguments[2], 2);
  assert.equal(arguments[3], 3);
  assert.equal(arguments[4], 4);
  assert.equal(arguments[5], 5);
  assert.equal(arguments[6], 6);
  assert.equal(arguments[7], 7);
  assert.equal(arguments[8], 8);
  assert.equal(arguments[9], 9);
  assert.equal(arguments[10], "a");
  assert.equal(arguments[11], "b");
  assert.equal(arguments[12], "c");
  assert.equal(arguments[13].a, 123);
  eventSequence += "4";
});

emitter.addListener('args', function() {
  assert.equal(arguments.length, 14);
  eventSequence += "5";
})

emitter.emit('args', 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, "a", "b", "c", { a: 123});


var listener1 = function() {
  eventSequence += '6';
};

emitter.addListener('rmTest', listener1);
emitter.emit('rmTest');
emitter.removeListener('rmTest', function() {});
emitter.emit('rmTest');
emitter.removeListener('rmTest', listener1);
emitter.emit('rmTest');


var listener2 = function() {
  eventSequence += '7';
};

emitter.addListener('rmTest', listener2);
emitter.addListener('rmTest', listener2);
emitter.emit('rmTest');
eventSequence += "|"
emitter.removeListener('rmTest', listener2);
emitter.emit('rmTest');
eventSequence += "|"
emitter.removeListener('rmTest', listener2);
emitter.emit('rmTest');
eventSequence += "|"


var listener3 = function() {
  eventSequence += '8';
};

emitter.addListener('rmTest', listener3);
emitter.addListener('rmTest', listener3);
emitter.emit('rmTest');
eventSequence += "|";
emitter.removeAllListeners('rmTest');
emitter.emit('rmTest');
eventSequence += "|";


emitter.emit('event');
eventSequence += "|";
emitter.removeAllListeners();
emitter.emit('event');
eventSequence += "|";


assert.equal(eventSequence, "112123123456677|7||88||123||");
