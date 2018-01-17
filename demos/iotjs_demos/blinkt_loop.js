var Blinkt = require('blinkt'),
	leds = new Blinkt(); 
//var leds = require('blinkt');

var r = 0;
    g = 0;
    b = 0;
    
 var numPixels = 8;

spacing = parseInt(360.0 / 16.0, 10);

function setValue(value) {
   value = value + spacing; 
   if (value > 255)
   {
     value = value -255;
   }
   return value;
}

leds.setup();
leds.clearAll();
    
for (var i = 0; i < numPixels; i++) {
  //r = setValue(r);
  //g = setValue(g);
  //b = setValue(b);
  leds.setPixel(i, r, g, b, 0.1);
}

leds.sendUpdate();
