var GPIO_MAP = {
  CTRL: {
    OUT: 0x10,
    IN:  0x20,
    FLOAT: 0,
  },
  PINS: {
    OUT1: { GPIO: 1 },
    INP1: { GPIO: 2 },
  },
};

module.exports = GPIO_MAP;
