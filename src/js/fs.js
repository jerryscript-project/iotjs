/* Copyright 2015 Samsung Electronics Co., Ltd.
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


var fs = exports;
var constants = require('constants');
var util = require('util');
var fsBuiltin = process.binding(process.binding.fs);


var O_APPEND = constants.O_APPEND;
var O_CREAT = constants.O_CREAT;
var O_EXCL = constants.O_EXCL;
var O_RDONLY = constants.O_RDONLY;
var O_RDWR = constants.O_RDWR;
var O_SYNC = constants.O_SYNC;
var O_TRUNC = constants.O_TRUNC;
var O_WRONLY = constants.O_WRONLY;


fs.statSync = function(path) {
  return fsBuiltin.stat(path);
};

fs.stat = function(path, callback) {
  fsBuiltin.stat(path, callback);
};


fs.Stats = function(dev,
                    mode,
                    nlink,
                    uid,
                    gid,
                    rdev,
                    blksize,
                    ino,
                    size,
                    blocks) {
  this.dev = dev;
  this.mode = mode;
  this.nlink = nlink;
  this.uid = uid;
  this.gid = gid;
  this.rdev = rdev;
  this.blksize = blksize;
  this.ino = ino;
  this.size = size;
  this.blocks = blocks;

};

fs.Stats.prototype.isDirectory = function() {
  return ((this.mode & constants.S_IFMT) === constants.S_IFDIR);
};



fsBuiltin.createStat = function(dev,
                                mode,
                                nlink,
                                uid,
                                gid,
                                rdev,
                                blksize,
                                ino,
                                size,
                                blocks) {
  var statobj = new fs.Stats(dev,
                             mode,
                             nlink,
                             uid,
                             gid,
                             rdev,
                             blksize,
                             ino,
                             size,
                             blocks);
  return statobj;
};


fs.close = function(fd, callback) {
  fsBuiltin.close(fd, checkArgFunction(callback, 'callback'));
};


fs.closeSync = function(fd) {
  fsBuiltin.close(fd);
};


fs.open = function(path, flags, mode, callback) {
  fsBuiltin.open(checkArgString(path, 'path'),
                 convertFlags(flags),
                 convertMode(mode, 438),
                 checkArgFunction(arguments[arguments.length - 1]), 'callback');
};


fs.openSync = function(path, flags, mode) {
  return fsBuiltin.open(checkArgString(path, 'path'),
                        convertFlags(flags),
                        convertMode(mode, 438));
};


fs.read = function(fd, buffer, offset, length, position, callback) {
  callback = checkArgFunction(callback, 'callback');

  var cb = function(err, bytesRead) {
    callback(err, bytesRead || 0, buffer);
  };

  return fsBuiltin.read(checkArgNumber(fd, 'fd'),
                        checkArgBuffer(buffer, 'buffer'),
                        checkArgNumber(offset, 'offset'),
                        checkArgNumber(length, 'length'),
                        checkArgNumber(position, 'position'),
                        cb);
};


fs.readSync = function(fd, buffer, offset, length, position) {
  return fsBuiltin.read(checkArgNumber(fd, 'fd'),
                        checkArgBuffer(buffer, 'buffer'),
                        checkArgNumber(offset, 'offset'),
                        checkArgNumber(length, 'length'),
                        checkArgNumber(position, 'position'));
};


fs.write = function(fd, buffer, offset, length, position, callback) {
  if (util.isFunction(position)) {
    callback = position;
    position = -1; // write at current position.
  }

  callback = checkArgFunction(callback, 'callback');

  var cb = function(err, written) {
    callback(err, written, buffer);
  };

  return fsBuiltin.write(checkArgNumber(fd, 'fd'),
                         checkArgBuffer(buffer, 'buffer'),
                         checkArgNumber(offset, 'offset'),
                         checkArgNumber(length, 'length'),
                         checkArgNumber(position, 'position'),
                         cb);
};


fs.writeSync = function(fd, buffer, offset, length, position) {
  if (util.isNullOrUndefined(position)) {
    position = -1; // write at current position.
  }

  return fsBuiltin.write(checkArgNumber(fd, 'fd'),
                         checkArgBuffer(buffer, 'buffer'),
                         checkArgNumber(offset, 'offset'),
                         checkArgNumber(length, 'length'),
                         checkArgNumber(position, 'position'));
};


function convertFlags(flag) {
  if (util.isString(flag)) {
    switch (flag) {
      case 'r': return O_RDONLY;
      case 'rs':
      case 'sr': return O_RDONLY | O_SYNC;

      case 'r+': return O_RDWR;
      case 'rs+':
      case 'sr+': return O_RDWR | O_SYNC;

      case 'w': return O_TRUNC | O_CREAT | O_WRONLY;
      case 'wx':
      case 'xw': return O_TRUNC | O_CREAT | O_WRONLY | O_EXCL;

      case 'w+': return O_TRUNC | O_CREAT | O_RDWR;
      case 'wx+':
      case 'xw+': return O_TRUNC | O_CREAT | O_RDWR | O_EXCL;

      case 'a': return O_APPEND | O_CREAT | O_WRONLY;
      case 'ax':
      case 'xa': return O_APPEND | O_CREAT | O_WRONLY | O_EXCL;

      case 'a+': return O_APPEND | O_CREAT | O_RDWR;
      case 'ax+':
      case 'xa+': return O_APPEND | O_CREAT | O_RDWR | O_EXCL;
    }
  }
  throw new TypeError('Bad argument: flags');
}


function convertMode(mode, def) {
  if (util.isNumber(mode)) {
    return mode;
  } else if (util.isString(mode)) {
    return parseInt(mode);
  } else if (def) {
    return convertMode(def);
  }
  return undefined;
}


function checkArgType(value, name, checkFunc) {
  if (checkFunc(value)) {
    return value;
  } else {
    throw new TypeError('Bad arguments: ' + name);
  }
}


function checkArgBuffer(value, name) {
  return checkArgType(value, name, util.isBuffer);
}


function checkArgNumber(value, name) {
  return checkArgType(value, name, util.isNumber);
}


function checkArgString(value, name) {
  return checkArgType(value, name, util.isString);
}


function checkArgFunction(value, name) {
  return checkArgType(value, name, util.isFunction);
}
