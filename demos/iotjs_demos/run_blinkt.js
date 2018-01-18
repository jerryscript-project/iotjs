var Blinkt = require('myblinkt'),
	leds = new Blinkt(); 
//var leds = require('blinkt');

//var myb = require('myblinkt2');

//console.log(myb.helloBlinkt);
//console.log(myb.addBlinkt(1, 2));
//console.log(myb.local);


var mymodule = require('mymodule');
console.log(mymodule.hello);
console.log(mymodule.add(1, 2));
console.log(mymodule.local);

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
    
for (var i = 0; i < numPixels; i++) {
  //r = setValue(r);
  //g = setValue(g);
  //b = setValue(b);
  leds.setPixel(i, 255, 0, 0, 0.1);
}

leds.sendUpdate();

