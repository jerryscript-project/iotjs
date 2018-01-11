# a dummy class to allow blinkt programs to work on non raspberry pi hardware and allow program output to be visible.
# so basically a development aid


BCM = "BCM Value"
OUT = "PIN setup as output (channel is an output)"


class GPIO():
    pass


def cleanup():
    print("* cleanup *")

def output(dat, value):

    print("* output * called with channels: {} , and mode: {}".format(dat, value))


def setmode(gpio_mode='PIN setup as output'):
    print("* setmode * called with mode: {}".format(gpio_mode))


def setwarnings(state=False):
    print("* setwarnings * called with state: {}".format(state))


def setup(pin_list = [0, 0], gpio_mode='PIN setup as output'):

    print("* setup * called with channels: {} , and mode: {}".format(pin_list, gpio_mode))



