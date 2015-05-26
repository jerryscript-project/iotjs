exports.add = function(a, b) {
  return a+b;
};

var x = require("lib/multi.js");
exports.multi = x.multi;
exports.add2 = x.add2;
