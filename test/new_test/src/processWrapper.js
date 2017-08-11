function guardAsync(funcDone) {
  process.removeAllListeners('uncaughtException');
  process.on('uncaughtException', function uncaughtExceptionHandler(e) {
    var ue = process._events['uncaughtException'];

    if ( ue && (typeof ue === 'function' || ue.length === 1)) {
      // Only this handler registered, so unexpected problem in test
      process.nextTick(funcDone.bind(null, e));
    }
  });
}

module.exports = {
  guardAsync: guardAsync
};
