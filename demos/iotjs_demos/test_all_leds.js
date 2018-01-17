var Blinkt = require('myblinkt'),
	leds = new Blinkt(); 
//var leds = require('blinkt');


var numPixels = 8;

try
{
leds.setup();
}
catch (e)
{
  console.error(e);
}

console.log("Starting LED light program.");
leds.clearAll();

// ***********************************
// Testing maximum r g b values at 50%
// ***********************************
console.log("* Testing all LED's are RED at 50% intensity *");    
for (var i = 0; i < numPixels; i++) {
  leds.setPixel(i, 255, 0, 0, 0.5);
}

leds.sendUpdate();

console.log("* Testing all LED's are GREEN at 50% intensity *"); 
for (var i = 0; i < numPixels; i++) {
  leds.setPixel(i, 0, 255, 0, 0.5);
}

leds.sendUpdate();

console.log("* Testing all LED's are BLUE at 50% intensity *"); 
for (var i = 0; i < numPixels; i++) {
  leds.setPixel(i, 0, 0, 255, 0.5);
}

leds.sendUpdate();


// ************************************
// Testing maximum r g b values at 100%
// ************************************

console.log("* Testing all LED's are RED at 100% intensity *");    
for (var i = 0; i < numPixels; i++) {
  leds.setPixel(i, 255, 0, 0, 1);
}

leds.sendUpdate();

console.log("* Testing all LED's are GREEN at 100% intensity *"); 
for (var i = 0; i < numPixels; i++) {
  leds.setPixel(i, 0, 255, 0, 1);
}

leds.sendUpdate();

console.log("* Testing all LED's are BLUE at 100% intensity *"); 
for (var i = 0; i < numPixels; i++) {
  leds.setPixel(i, 0, 0, 255, 1);
}

leds.sendUpdate();


// *************************************************
// Testing maximum white values at 10%, 50% and 100%
// *************************************************


console.log("* Testing all LED's are WHITE at 10% intensity *");    
for (var i = 0; i < numPixels; i++) {
  leds.setPixel(i, 255, 255, 255, 0.1);
}

leds.sendUpdate();

console.log("* Testing all LED's are WHITE at 50% intensity *"); 
for (var i = 0; i < numPixels; i++) {
  leds.setPixel(i, 255, 255, 255, 0.5);
}

leds.sendUpdate();

console.log("* Testing all LED's are WHITE at 100% intensity *"); 
for (var i = 0; i < numPixels; i++) {
  leds.setPixel(i, 255, 255, 255, 1);
}

leds.sendUpdate();


// Reset all LED's to switch them off.
var timeout = setTimeout(function() {
	leds.clearAll();
	leds.sendUpdate();
	console.log("******** All LED's are cleared. *******");
}, 1000);



