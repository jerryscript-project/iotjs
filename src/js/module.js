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


var Native = require('native');
var fs = Native.require('fs');

function IotjsModule(id, parent) {
  this.id = id;
  this.exports = {};
  this.filename = null;
  this.parent = parent;
}

module.exports = IotjsModule;


IotjsModule.cache = {};
IotjsModule.wrapper = Native.wrapper;
IotjsModule.wrap = Native.wrap;


var cwd;
try { cwd = process.cwd(); } catch (e) { cwd = null; }

var moduledirs = [''];
if (cwd){
  moduledirs.push(cwd + '/');
  moduledirs.push(cwd + '/node_modules/');
}
if (process.env.HOME){
  moduledirs.push(process.env.HOME + '/node_modules/');
}
if (process.env.NODE_PATH){
  moduledirs.push(process.env.NODE_PATH + '/node_modules/');
}

IotjsModule.concatdir = function(a, b){
  var rlist = [];
  var i;
  for (i = 0; i< a.length ; i++) {
    rlist.push(a[i]);
  }

  for (i = 0; i< b.length ; i++) {
    rlist.push(b[i]);
  }

  return rlist;
};


IotjsModule.resolveDirectories = function(id, parent) {
  var dirs = moduledirs;
  if (parent) {
    if (!parent.dirs){
      parent.dirs = [];
    }
    dirs = IotjsModule.concatdir(parent.dirs, dirs);
  }

  return dirs;
};


IotjsModule.resolveFilepath = function(id, directories) {

  for (var i = 0; i<directories.length ; i++) {
    var dir = directories[i];
    // 1. 'id'
    var filepath = IotjsModule.tryPath(dir+id);

    if (filepath){
      return filepath;
    }

    // 2. 'id.js'
    filepath = IotjsModule.tryPath(dir+id+'.js');

    if (filepath){
      return filepath;
    }

    // 3. package path /node_modules/id
    var packagepath = dir + id;
    var jsonpath = packagepath + '/package.json';
    filepath = IotjsModule.tryPath(jsonpath);
    if (filepath){
      var pkgSrc = process.readSource(jsonpath);
      var pkgMainFile = JSON.parse(pkgSrc).main;
      filepath = IotjsModule.tryPath(packagepath + '/' + pkgMainFile);
      if (filepath){
        return filepath;
      }
      // index.js
      filepath = IotjsModule.tryPath(packagepath + '/' + 'index.js');
      if (filepath){
        return filepath;
      }
    }

  }

  return false;
};


IotjsModule.resolveModPath = function(id, parent) {

  // 0. resolve Directory for lookup
  var directories = IotjsModule.resolveDirectories(id, parent);

  var filepath = IotjsModule.resolveFilepath(id, directories);

  if (filepath){
    return filepath;
  }

  return false;
};


IotjsModule.tryPath = function(path) {
  var stats = IotjsModule.statPath(path);
  if (stats && !stats.isDirectory()) {
    return path;
  }  else {
    return false;
  }
};


IotjsModule.statPath = function(path) {
  try {
    return fs.statSync(path);
  } catch (ex) {
    return false;
  }
};


IotjsModule.load = function(id, parent /*, isMain */) {
  if (process.native_sources[id]){
    return Native.require(id);
  }
  var module = new IotjsModule(id, parent);

  var modPath = IotjsModule.resolveModPath(module.id, module.parent);

  var cachedModule = IotjsModule.cache[modPath];
  if (cachedModule) {
    return cachedModule.exports;
  }

  if (modPath) {
    module.filename = modPath;
    module.SetModuleDirs(modPath);
    module.compile();
  }  else {
    throw new Error('No module found');
  }

  IotjsModule.cache[modPath] = module;

  return module.exports;
};


IotjsModule.prototype.compile = function() {
  var self = this;
  var requireForThis = function(path) {
    return self.require(path);
  };

  var source = process.readSource(self.filename);
  var fn = process.compile(source);
  fn.call(self.exports, self.exports, requireForThis, self);
};


IotjsModule.runMain = function(){
  IotjsModule.load(process.argv[1], null, true);
  process._onNextTick();
};


IotjsModule.prototype.SetModuleDirs = function(filepath){
  // At next require, search module from parent's directory
  var dir = '';
  var i;
  for (i = filepath.length-1;i>=0 ; i--) {
    if (filepath[i] == '/'){
      break;
    }
  }

  // save filepath[0] to filepath[i]
  // e.g. /home/foo/main.js ->  /home/foo/
  for (;i>=0 ; i--) {
    dir = filepath[i] + dir;
  }
  this.dirs = [dir];
};


IotjsModule.prototype.require = function(id) {
  return IotjsModule.load(id, this);
};
