var fs = require('buffer');
var util = require('util');

var x = require("require_add");
print("require_add: " + x.add(1,4));
process.nextTick(function(){print("nextTick works!");});

var str = process.readSource("package.json");
var json = process.JSONParse(str);
print("package.json src:");
print(str);
print("package.json main: " + json.main);

