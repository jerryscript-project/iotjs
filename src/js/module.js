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


var Native = require('native');
var fs = Native.require('fs');

function Module(id, parent) {
  this.id = id;
  this.exports = {};
  this.filename = null;
  this.parent = parent;
};

module.exports = Module;


Module.cache = {};
Module.wrapper = Native.wrapper;
Module.wrap = Native.wrap;


var moduledirs;
// In nuttx, we assume that modules are installed under
// /mnt/sdcard/node_modules/ directory
if(process.platform === 'nuttx'){
  moduledirs = [ "", process.cwd()+"/", "/mnt/sdcard/node_modules/" ];
}
else {
  moduledirs = [ "", process.cwd()+"/", process.cwd()+"/node_modules/",
                     process.env.HOME + "/node_modules/" ];
}

Module.concatdir = function(a, b){
  var rlist = [];
  for(var i = 0; i< a.length ; i++) {
    rlist.push(a[i]);
  }

  for(var i = 0; i< b.length ; i++) {
    rlist.push(b[i]);
  }

  return rlist;
};


Module.resolveDirectories = function(id, parent) {
  var dirs = moduledirs;
  if(parent) {
    if(!parent.dirs){
      parent.dirs = [];
    }
    dirs = Module.concatdir(parent.dirs, dirs);
  }
  return dirs;
};


Module.resolveFilepath = function(id, directories) {

  for(var i = 0; i<directories.length ; i++) {
    var dir = directories[i];
    // 1. 'id'
    var filepath = Module.tryPath(dir+id);

    if(filepath){
      return filepath;
    }

    // 2. 'id.js'
    filepath = Module.tryPath(dir+id+'.js');

    if(filepath){
      return filepath;
    }

    // 3. package path /node_modules/id
    var packagepath = dir + id;
    var jsonpath = packagepath + "/package.json";
    filepath = Module.tryPath(jsonpath);
    if(filepath){
      var pkgSrc = process.readSource(jsonpath);
      var pkgMainFile = process.JSONParse(pkgSrc).main;
      filepath = Module.tryPath(packagepath + "/" + pkgMainFile);
      if(filepath){
        return filepath;
      }
      // index.js
      filepath = Module.tryPath(packagepath + "/" + "index.js");
      if(filepath){
        return filepath;
      }
    }

  }

  return false;
};


Module.resolveModPath = function(id, parent) {

  // 0. resolve Directory for lookup
  var directories = Module.resolveDirectories(id, parent);

  var filepath = Module.resolveFilepath(id, directories);

  if(filepath){
    return filepath;
  }

  return false;
};


Module.tryPath = function(path) {
  var stats = Module.statPath(path);
  if(stats && !stats.isDirectory()) {
    return path;
  }
  else {
    return false;
  }
};


Module.statPath = function(path) {
  try {
    return fs.statSync(path);
  } catch (ex) {}
  return false;
};


Module.load = function(id, parent, isMain) {
  if(process.native_sources[id]){
    return Native.require(id);
  }
  var module = new Module(id, parent);

  var modPath = Module.resolveModPath(module.id, module.parent);

  var cachedModule = Module.cache[modPath];
  if (cachedModule) {
    return cachedModule.exports;
  }

  if (modPath) {
    module.filename = modPath;
    module.SetModuleDirs(modPath);
    module.compile();
  }
  else {
    throw new Error('No module found');
  }

  Module.cache[modPath] = module;

  return module.exports;
};


Module.prototype.compile = function() {
  var self = this;
  var requireForThis = function(path) {
      return self.require(path);
  };

  var source = process.readSource(self.filename);
  var fn = process.compile(source);
  fn.call(self, self.exports, requireForThis, self);
};


Module.runMain = function(){
  Module.load(process.argv[1], null, true);
  process._onNextTick();
};



Module.prototype.SetModuleDirs = function(filepath)
{
  // At next require, search module from parent's directory
  var dir = "";
  var i;
  for(i = filepath.length-1;i>=0 ; i--) {
    if(filepath[i] == '/'){
      break;
    }
  }

  // save filepath[0] to filepath[i]
  // e.g. /home/foo/main.js ->  /home/foo/
  for(;i>=0 ; i--) {
    dir = filepath[i] + dir;
  }
  this.dirs = [dir];
};


Module.prototype.require = function(id) {
  return Module.load(id, this);
};
