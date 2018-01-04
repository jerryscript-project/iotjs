from gpiozero import LED
from time import sleep

gpio_pin = 5
print("*** Setting gpio pin to: {} ***".format(gpio_pin))

led = LED(5)
while True:
	led.on()
	print("LED on....")
	sleep(1)
	led.off()
	print("LED off....")
	sleep(1)
