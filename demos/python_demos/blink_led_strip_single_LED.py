from blinkt import set_pixel, show
from random import randint
from time import sleep

print("Program starting for blinkt - this should switch on a single LED...")

while True:

    print("Setting single LED strip sequence")
    r = randint(0, 255)
    g = randint(0, 255)
    b = randint(0, 255)
    set_pixel(0, r, g, b)
    show()
    sleep(5)

    print("Switching off single LED strip sequence")
    r = randint(0, 255)
    g = randint(0, 255)
    b = randint(0, 255)
    set_pixel(pixel, 0, 0, 0)
    show()
    sleep(5)


        

