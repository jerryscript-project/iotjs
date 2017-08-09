var fs = require('fs');
var pv = require('./processWrapper');
var suite = require('./suite');
var rep = require('./reporter');
var reporter = rep.getReporter();

var testFolder = './test/',
    specFiles = ['../test/test1.js'];

// TODO: Prepare environment
// var env = loadEnvFromFile(argv[...])
var env = {
  os: 'Linux',
  platform: 'desktop',
  modules: ['spi']
};

// TODO: Load tests from folder
// ...

// Load spec file with tests
function loadSpecFile(fileName) {
  var src = '"use strict";\n' + fs.readFileSync(fileName, 'utf8');
  try {
    var suiteFunc = new Function('describe', 'it', 'assert', src);
    return suiteFunc;
  } catch (e) {
    console.error('Failed to parse: ', fileName, '. Error:\n', e);
  }

  return null;
}


// Prepare test suites
specFiles.forEach(function (specFile) {
  reporter.startSpec(specFile);
  reporter.verbose('** Load spec file:', specFile);
  var specFunc = loadSpecFile(specFile);
  reporter.verbose('** Execute spec file:', specFile);
  var spec = suite.runSpec(specFile, specFunc); // Will register describes
  spec.env = env;
  reporter.verbose('** Execute suites', specFile);
  suite.runSuitesFrom(spec, reporter, function () { // Will execute tests (from'it')
    reporter.log(JSON.stringify(reporter.results, null, 2));
    reporter.verbose('** Done', specFile);
  });
});

