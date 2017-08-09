
describe('First test set', {skip: {'nuttx': 'Not implemented'}}, function () {
  it('works with simple asserts, true === true', function () {
    assert.strictEqual(true, true, 'true is really === true');
  });
  it('works with simple asserts, false === false', function () {
    assert.strictEqual(false, false, 'false is really === false');
  });
  it('works with simple asserts, false !== true', function () {
    assert.notStrictEqual(false, true, 'false is really !== true');
  });
});

describe('first async test set', function () {
  it('works with simple asserts async way too', {async: true}, function (done) {
    setTimeout(function() {
      assert.strictEqual(true, true, 'true is really === true in async test');
      done();
    }, 2000);
  });
});

describe('skip check test set', {skip: {'linux': 'Not implemented'}}, function () {
  it('fail if not skipped', {async: true}, function (done) {
    setTimeout(function() {
      assert.strictEqual(true, false, 'true is not really === false in async test');
      done();
    }, 2000);
  });
});

describe('skip check test set 2', {
  skip: function(env) {return env.platform === 'desktop' ? 'Not for desktop' : null;}
}, function () {
  it('fail if not skipped', {async: true}, function (done) {
    setTimeout(function() {
      assert.strictEqual(true, false, 'true is not really === false in async test');
      done();
    }, 2000);
  });
});


// TODO: for this one proper 'process' wrapper needs to be done
describe('first uncaught exception', function () {
  var trace = '';
  it('checks order of exceptions', {async: true}, function (done) {

    process.on('uncaughtException', function () {
      trace += 'A';
    });

    process.on('uncaughtException', function () {
      trace += 'B';
    });

    process.on('exit', function() {
      trace += 'C';
      process.removeAllListeners('uncaughtException');
      assert.equal(trace, 'ABABC');
      done();
    });

    process.nextTick(function() {
      assert.fail('Fail 2');
    });

    assert.fail('Fail 1');
  });
});
