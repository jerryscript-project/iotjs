var guardAsync = require('./processWrapper').guardAsync;
var assert = require('assert');

var defaultSuiteOptions = {};

function getDescribe(spec) {
  return function describe(name, options, suiteFunction) {
    var func = suiteFunction || options,
        opt = !suiteFunction ? defaultSuiteOptions : options;

    spec.suites.push({
      name: name,
      options: opt,
      exec: func,
      tests: []
    });
  };
}

var defaultTestOptions = {
  async: false
};

function getIt(spec) {
  return function it(name, options, testFunction) {
    var func = testFunction || options,
        opt = !testFunction ? defaultTestOptions : options;

    spec.currentSuite.tests.push({
      name: name,
      options: opt,
      exec: func
    });
  };
}

function runSpec(specName, specFunc, env) {
  var spec = {
    name: specName,
    currentSuite: null,
    suites: []
  };

  var describe = getDescribe(spec);
  var it = getIt(spec);

  try {
    specFunc(describe, it, assert, env);
  } catch (err) {
    console.error('Error running suiteFunc', err);
  }

  return spec;
}

function runSingleTest(test, doneCallback) {
  try {
    // Will guard errors in async functions and make sure test ends properly
    if (test.options.async) {
      guardAsync(doneCallback);
      test.exec(doneCallback);
    } else {
      test.exec(null);
      process.nextTick(doneCallback);
    }
  } catch (err) {
    console.error('[' + test.name + '] Error executing test:', err.message);
    doneCallback(err);
  }
}

function runSuiteTests(suite, reporter, suiteDone, idx) {
  if (typeof idx !== 'number') {
    idx = 0;
  }
  if (idx === suite.tests.length) {
    process.nextTick(suiteDone);
    return;
  }

  var test = suite.tests[idx];
  // TODO: add test timeout
  // TODO: add skipping single tests
  reporter.startTest(idx, suite.tests.length, test.name);
  runSingleTest(test, function (err) {
    if (test.complete === true) {
      // prevent from finishing test few times
      console.error('[' + test.name + '] Finished multiple times');
      return;
    }
    test.complete = true;

    reporter.endTest(err);
    if (err) {
      // Break whole suite
      process.nextTick(suiteDone.bind(null, err));
      return;
    }
    process.nextTick(runSuiteTests.bind(null, suite, reporter, suiteDone, idx + 1));
  });
}

function runSuitesFrom(spec, reporter, callback, suiteIdx) {
  if (typeof suiteIdx !== 'number') {
    suiteIdx = 0;
  }
  // No tests left, end suite without error
  if (suiteIdx === spec.suites.length) {
    process.nextTick(callback);
    return;
  }
  var suite = spec.suites[suiteIdx];

  // simple skip suite
  if (typeof suite.options.skip === 'function') {
    var skipReason =  suite.options.skip(spec.env);
    if (skipReason) {
      reporter.startSuite(suiteIdx, spec.suites.length, suite.name, skipReason);
      process.nextTick(runSuitesFrom.bind(null, spec, reporter, callback, suiteIdx + 1));
      return;
    }
  }

  reporter.startSuite(suiteIdx, spec.suites.length, suite.name);

  spec.currentSuite = suite;

  try {
    // Register 'it' tests from suite
    suite.exec();
  } catch (err) {
    reporter.error('[' + suite.name + '] Error executing suite:', err);
    reporter.endSuite(suiteIdx, err);
    // schedule next suite
    process.nextTick(runSuitesFrom.bind(null, spec, reporter, callback, suiteIdx + 1));
    return;
  }

  // Run suite
  runSuiteTests(suite, reporter, function(err) {
    reporter.endSuite(suiteIdx, err);
    // schedule next suite
    process.nextTick(runSuitesFrom.bind(null, spec, reporter, callback, suiteIdx + 1));
  });

}


module.exports = {
  runSpec: runSpec,
  runSuitesFrom: runSuitesFrom
};
