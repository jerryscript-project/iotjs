var ERROR = 0;
var NORMAL = 1;
var VERBOSE = 2;

// Polyfill for lack of proces.stdout
if (process && !process.stdout) {
  process.stdout = {
    write: function(str) {console.log(str)}
  }
}

function getReporter(options) {
  var opt = options || {};
  var toConsole = opt.toConsole || true;
  var lvl = opt.logLevel;
  lvl = (lvl === ERROR || lvl === NORMAL || lvl === NORMAL) ? lvl : NORMAL;

  var specName = '';
  var suiteName = '';
  var results = {
    bin: {
      text: 0,
      total: 0,
      data: 0,
      bss: 0,
      rodata: 0
    },
    date: new Date().toISOString(),
    tests: [],
    submodules: []
  };

  function log() {
    if (toConsole && lvl <= NORMAL)
      console.log.apply(console, arguments);
  }

  function error() {
    if (toConsole && lvl <= ERROR)
      console.error.apply(console, arguments);
  }

  function verbose() {
    if (toConsole && lvl <= VERBOSE)
      console.warn.apply(console, arguments);
  }

  function startSpec(name) {
    specName = name;
    log('Testing', name);
  }

  function endSpec() {
    log('');
  }

  function startSuite(no, suiteCnt, name, skipReason) {
    suiteName = name;

    if (skipReason !== undefined) {
      results.tests.push({
        name: specName + ' / ' + suiteName,
        result: 'skip',
        output: skipReason
      });
      log('  [' + specName + '] Suite ' + (no + 1) + '/' + suiteCnt + ' - ', name, '... Skipped ('+skipReason+')');
    } else {
      log('  [' + specName + '] Suite ' + (no + 1) + '/' + suiteCnt + ' - ', name);
    }
  }

  function endSuite(no, err) {
    var res = {
      name: specName + ' / ' + suiteName,
      result: 'pass'
    };
    if (err instanceof Error) {
      res.result = 'fail';
      res.output = '' + err;
    }
    results.tests.push(res);
  }

  function startTest(no, suiteTests, name) {
    process.stdout.write('      [' + (no + 1) + '/' + suiteTests + '] ' + name + '... ');
  }

  function endTest(err) {
    if (err === undefined) {
      log('\r    ✔');
    } else {
      // todo: display AssertError nice way, other as exceptions
      process.stdout.write('' + err);
      log('\r    ✘');
    }
  }
  return {
    startSpec: startSpec,
    startSuite: startSuite,
    startTest: startTest,
    endSuite: endSuite,
    endTest: endTest,
    error: error,
    log: log,
    verbose: verbose,
    results: results
  };
}

module.exports = {
  getReporter: getReporter,
  ERROR: ERROR,
  NORMAL: NORMAL,
  VERBOSE: VERBOSE
};
