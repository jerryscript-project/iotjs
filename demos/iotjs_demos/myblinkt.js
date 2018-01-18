
/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


"use strict";

var Gpio = require('gpio');
var gpio = new Gpio();

// Define the DATA pin and the CLK pin for the board. This targets the Pimoroni Blinkt Board
// Pinout can be found at https://pinout.xyz/pinout/blinkt
var	DAT = 23,
	CLK = 24,
	Blinkt;

var numPixels = 8;	// number of LED lights in our hardware strip
var numBitsAPA102 = 8;	// number of bits a data register is for the APA102
var pixels = [];	// an array used to store our byte array to  BITBANG our hardware

var gpio_dat,	// A reference (handle) to our gpio data pin
    gpio_clk;	// A reference (handle) to our gpioclock pin


function Blinkt() {
  if (!(this instanceof Blinkt)) {
    return new Blinkt();
  } 
}

/**
 * This Connects to our GPIO and sets the pin modes. Must be called
 * before any other commands. All pixels will start off white at
 * full brightness by default.
 */
Blinkt.prototype.setup = function setup () {
	console.log(" * Setup called *");
	gpio_dat = gpio.open({
	  pin: DAT,
	  direction: gpio.DIRECTION.OUT
	}, function(err) {
	  	if (!err) {
	  	}
		else {
			console.error(err);
		}
	});

	gpio_clk = gpio.open({
	  pin: CLK,
	  direction: gpio.DIRECTION.OUT
	}, function(err) {
	  if (!err) {
	  } else {
			console.error(err);
		}
	});


	// Init pixels array to all on and full brightness
	for (var i = 0; i < numPixels; i++) {
		this.setPixel(i, 255, 255, 255, 1.0);
	}
};

/**
 * Sets all pixels to the passed RGB and brightness values.
 * @param {Number} red The pixel red value between 0 and 255.
 * @param {Number} green The pixel green value between 0 and 255.
 * @param {Number} blue The pixel blue value between 0 and 255.
 * @param {Number} a The pixel brightness value between 0.0 and 1.0.
 */
Blinkt.prototype.setAllPixels = function setAllPixels (red, green, blue, a) {
	for (var i = 0; i < numPixels; i++) {
		this.setPixel(i, red, green, blue, a);
	}
};

/**
 * Sets the specified pixel to the passed rgb and brightness level.
 * The pixelNum is an integer between 0 and 7 to indicate the pixel
 * to change.
 * @param {Number} pixelNum The pixel to set RGB and brightness for.
 * An integer value between 0 and 7. Zero is the first pixel, 7 is
 * the last one.
 * @param {Number} red The pixel red value between 0 and 255.
 * @param {Number} green The pixel green value between 0 and 255.
 * @param {Number} blue The pixel blue value between 0 and 255.
 * @param {Number} a The pixel brightness value between 0.0 and 1.0.
 */

Blinkt.prototype.setPixel = function setPixel (pixelNum, red, green, blue, a) {

	if (a === undefined) {
		if (pixels[pixelNum]) {
			// Set a to current level or 1.0 if none exists
			a = pixels[pixelNum][3] !== undefined ? pixels[pixelNum][3] : 1.0;
		}
	} 
	else {
		a = parseInt((31.0 * a), 10) & 0x1F; //  see here how this bit string is defined for  brightness http://www.datasheetpdf.com/datasheet/APA102.html
	}

	pixels[pixelNum] = [
		parseInt(red, 10) & 255, //  convert to a bit string representation of value
		parseInt(green, 10) & 255,
		parseInt(blue, 10) & 255, 
		a
	];
};

/**
 * Sets the brightness of the pixel specified by pixelNum.
 * @param {Number} pixelNum The pixel to set RGB and brightness for.
 * An integer value between 0 and 7. Zero is the first pixel, 7 is
 * the last one.
 * @param {Number} brightness The pixel brightness value between 0.0
 * and 1.0.
 */
Blinkt.prototype.setBrightness = function setBrightness (pixelNum, brightness) {
	pixels[pixelNum][3] = parseInt((31.0 * brightness), 10) & 0x1F; 
};

/**
 * Clears the pixel buffer.
 * This is the same as setting all pixels to black.
 * You must also call sendUpdate() if you want to turn Blinkt! off.
 */
Blinkt.prototype.clearAll = function clearAll () {
	for (var i = 0; i < numPixels; i++) {
		this.setPixel(i, 0, 0, 0);
	}
};


/**
 * Takdes the current golbal 'pixels' array and  writes each byte to the
 * GPIO pins. Each pixel has 4 bytes describing it's state:
 * red, green, blue and brightness. 
 * i.e. pixels[0] = [255,255,255, 128]
 * Once this function is called the new state will be loaded onto the
 * blinkt hardware.
 */
Blinkt.prototype.sendUpdate = function sendUpdate () {

	var i, pixel;
	latch();	// send a 32 bit latch (on/off) sequence

	for (i = 0; i < numPixels; i++) {
		pixel = pixels[i];
		//console.log(" * PIXEL NUMBER : " +  i + " * ");
		// Brightness
		//console.log(" * writeByte * brightness: " + pixel[3]);
		writeByte(0xE0 | pixel[3]); // jshint ignore:line
		// Blue
		//console.log(" * writeByte * blue: " + pixel[2]);
		writeByte(pixel[2]);
		// Green
		//console.log(" * writeByte * green: " + pixel[1]);
		writeByte(pixel[1]);
		// Red
		//console.log(" * writeByte * red: " + pixel[0]);
		writeByte(pixel[0]);
	}

	latch();	// send a 32 bit latch (on/off) sequence
};

/**
 * Writes byte data to the GPIO pins.
 * @param {Number} byte The byte value to write.
 * @private
 */
function writeByte (byte) {
	var bit;

	for (var i = 0 ; i < numBitsAPA102; i++) {
		bit = ((byte & (1 << (7 - i))) > 0) === true ? 1 : 0; //  shift your byte value along and set your 'bit' true or false

    		gpio_dat.write(bit);		// physically set your pin high/low
		//console.log(" ***** writebit ***** DATA: " + bit);
		gpio_clk.write(true);		// set your clock high to load your data
		//console.log(" ***** writebit ***** CLK: " + true);
		gpio_clk.write(false);		// set your clock low to consume your data
		//console.log(" ***** writebit ***** CLK: " + false);
	}
}

/**
 * Emit exactly enough clock pulses to latch the small dark die APA102s which are weird.
 * @private
 */
function latch() {
	//console.log(" *** Latch DATA ***" + false);
	gpio_dat.write(false);
	for (var i = 0 ; i < 36; i++) {
		gpio_clk.write(true);
		//console.log(" *** Latch CLK ***" + true);
		gpio_clk.write(false);
		//console.log(" *** Latch CLK ***" + false);
	}
}

module.exports = Blinkt;
