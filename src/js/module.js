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


var Builtin = require('builtin');
var fs = Builtin.require('fs');
var dynamicloader = Builtin.require('dynamicloader');

function normalizePathString(path) {
  // Assume all path separators are '/'
  var input = path.split('/');
  var output = [];
  while (input.length > 0) {
    if (input[0] === '.' || (input[0] === '' && input.length > 1)) {
      input.shift();
      continue;
    }
    if (input[0] === '..') {
      input.shift();
      if (output.length > 0 && output[output.length - 1] !== '..') {
        output.pop();
      } else {
        throw new Error('Requested path is below root: ' + path);
      }
      continue;
    }
    output.push(input.shift());
  }
  return output;
}

var path;
if (process.platform === 'windows') {
  /* In case of windows:
   * replace all '\' characters to '/' for ease of use for now.
   */
  path = {
    pathReplacer: new RegExp('\\\\', 'g'),
    pathSeparator: '\\',
    normalizeSeparators: function(pathString) {
      return pathString.replace(path.pathReplacer, '/');
    },
    isDeviceRoot: function(pathString) {
      if (pathString.charCodeAt(1) !== 0x3A /* ':' */) {
        return false;
      }
      var drive = pathString.charCodeAt(0);
      return (drive >= 0x61 /* a */ && drive <= 0x7A /* z */)
             || (drive >= 0x41 /* A */ && drive <= 0x5A /* Z */);
    },
    normalizePath: function(pathString) {
      pathString = path.normalizeSeparators(pathString);

      var deviceRoot = '';
      if (!path.isDeviceRoot(pathString)) {
        deviceRoot = path.cwd().substr(0, 2) + '/';
      }

      var pathElements = normalizePathString(pathString);
      return deviceRoot + pathElements.join('/');
    },
    cwd: function() {
      return path.normalizeSeparators(process.cwd());
    },
  };
} else {
  path = {
    isDeviceRoot: function(pathString) {
      return pathString.charCodeAt(0) === 0x2F; /* '/' */
    },
    normalizePath: function(path) {
      var beginning = '';
      if (path.indexOf('/') === 0) {
        beginning = '/';
      }

      var pathElements = normalizePathString(path);
      return beginning + pathElements.join('/');
    },
    cwd: process.cwd,
  };
}

function Module(id, parent) {
  this.id = id;
  this.exports = {};
  this.filename = null;
  this.parent = parent;
}

module.exports = Module;


Module.cache = {};
// Cache to store not yet compiled remote modules
Module.remoteCache = {};

var moduledirs = [''];

var cwd;
try {
  cwd = process.env.IOTJS_WORKING_DIR_PATH || path.cwd();
} catch (e) { }
if (cwd) {
  moduledirs.push(cwd + '/');
  moduledirs.push(cwd + '/iotjs_modules/');
}

if (process.env.HOME) {
  moduledirs.push(process.env.HOME + '/iotjs_modules/');
}

if (process.env.IOTJS_PATH) {
  moduledirs.push(process.env.IOTJS_PATH + '/iotjs_modules/');
}

if (process.env.IOTJS_EXTRA_MODULE_PATH) {
  var extra_paths = process.env.IOTJS_EXTRA_MODULE_PATH.split(':');
  extra_paths.forEach(function(path) {
    if (path.slice(-1) !== '/') {
      path += '/';
    }
    moduledirs.push(path);
  });
}

function tryPath(modulePath, ext) {
  return Module.tryPath(modulePath) ||
         Module.tryPath(modulePath + ext);
}

Module.resolveDirectories = function(id, parent) {
  var dirs = moduledirs;
  if (parent) {
    if (!parent.dirs) {
      parent.dirs = [];
    }
    dirs = parent.dirs.concat(dirs);
  }
  return dirs;
};


Module.resolveFilepath = function(id, directories) {
  for (var i = 0; i < directories.length; i++) {
    var dir = directories[i];
    var modulePath = dir + id;

    if (!path.isDeviceRoot(modulePath)) {
      modulePath = path.cwd() + '/' + modulePath;
    }

    if ((process.platform === 'tizenrt' || process.platform === 'nuttx') &&
        (modulePath.indexOf('../') != -1 || modulePath.indexOf('./') != -1)) {
      modulePath = path.normalizePath(modulePath);
    }

    var filepath,
        ext = '.js';

    // id[.ext]
    if ((filepath = tryPath(modulePath, ext))) {
      return filepath;
    }

    // 3. package path id/
    var jsonpath = modulePath + '/package.json';

    if (Module.tryPath(jsonpath)) {
      var pkgSrc = Builtin.readSource(jsonpath);
      var pkgMainFile = JSON.parse(pkgSrc).main;

      // pkgmain[.ext]
      if (pkgMainFile &&
          (filepath = tryPath(modulePath + '/' + pkgMainFile, ext))) {
        return filepath;
      }
    }

    // index[.ext] as default
    if ((filepath = tryPath(modulePath + '/index', ext))) {
      return filepath;
    }

    // id[.iotjs]
    if (dynamicloader && (filepath = tryPath(modulePath, '.iotjs'))) {
      return filepath;
    }
  }

  return false;
};


Module.resolveModPath = function(id, parent) {
  if (parent != null && id === parent.id) {
    return false;
  }

  // 0. resolve Directory for lookup
  var directories = Module.resolveDirectories(id, parent);

  var filepath = Module.resolveFilepath(id, directories);

  if (filepath) {
    return path.normalizePath(filepath);
  }

  return false;
};


Module.tryPath = function(path) {
  try {
    var stats = fs.statSync(path);
    if (stats && !stats.isDirectory()) {
      return path;
    }
  } catch (ex) { }

  return false;
};


Module.load = function(id, parent) {
  if (Builtin.builtin_modules[id]) {
    return Builtin.require(id);
  }
  if (Module.remoteCache[id]) {
    Module.compileRemoteSource(id, Module.remoteCache[id]);
    delete Module.remoteCache[id];
    return Module.cache[id].exports;
  }

  var module = new Module(id, parent);
  var modPath = Module.resolveModPath(module.id, module.parent);
  var cachedModule = Module.cache[modPath];

  if (cachedModule) {
    return cachedModule.exports;
  }

  if (!modPath) {
    throw new Error('Module not found: ' + id);
  }

  module.filename = modPath;
  module.dirs = [modPath.substring(0, modPath.lastIndexOf('/') + 1)];
  Module.cache[modPath] = module;

  var ext = modPath.substr(modPath.lastIndexOf('.') + 1);
  var source;

  if (ext === 'js') {
    source = Builtin.readSource(modPath);
    module.compile(modPath, source);
  } else if (ext === 'json') {
    source = Builtin.readSource(modPath);
    module.exports = JSON.parse(source);
  } else if (dynamicloader && ext === 'iotjs') {
    module.exports = dynamicloader(modPath);
  }

  Module.cache[modPath] = module;

  return module.exports;
};

Module.compileRemoteSource = function(filename, source) {
  var module = new Module(filename, null);
  var cachedModule = Module.cache[filename];

  if (cachedModule) {
    return cachedModule.exports;
  }

  module.filename = filename;
  module.compile(filename, source);
  Module.cache[filename] = module;

  return module.exports;
};


Module.prototype.compile = function(filename, source) {
    var fn = Builtin.compile(filename, source);
    fn.call(this.exports, this.exports, this.require.bind(this), this);
};


Module.runMain = function() {
  if (Builtin.debuggerWaitSource) {
    var sources = Builtin.debuggerGetSource();
    sources.forEach(function(rModule) {
      Module.remoteCache[rModule[0]] = rModule[1];
    });
    // Name of the first module
    var fModName = sources[sources.length - 1][0];
    Module.compileRemoteSource(fModName, Module.remoteCache[fModName]);
  } else {
    Module.load(process.argv[1], null);
  }
  while (process._onNextTick());
};

Module.prototype.require = function(id) {
  return Module.load(id, this);
};
