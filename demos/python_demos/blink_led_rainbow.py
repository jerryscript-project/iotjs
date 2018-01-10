from blinkt import set_pixel, set_brightness, show
from random import randint
import colorsys
import time

print("Program starting for blinkt rainbow...")

spacing = 360.0/16.0
hue = 0
set_brightness(0.1)

while True:
	hue = int(time.time() * 100) %360

	for pixel in range(8):
		offset_pixels = pixel * spacing			# How wide our colors will spread
		h = ((hue + offset_pixels) % 360) / 360.0
		
		r, g, b, = [int(c * 255) for c in colorsys.hsv_to_rgb(h, 1.0, 1.0)]
		set_pixel(pixel, r, g, b)
	show()
	time.sleep(0.001)
