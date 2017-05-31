## Artik053 module

## `Pin`
To use system IO, such as PWM, you must know pin name.


### `PWM Pin Number` <a name="pwm-pin"></a>

For example,
``` javascript
var pwm = new require('pwm')();
var config = {
  pin: 0,
  period: 0.001,
  dutyCycle: 0.5
}
var pwm0 = pwm.open(config, function() {
  if (err) {
    throw err;
  }
});
```
The following is a list of PWM pin numbers.

| PWM Pin number | GPIO Name |
| :---: | :---: |
| 0 | XPWMTOUT_0 |
| 1 | XPWMTOUT_1 |
| 2 | XPWMTOUT_2 |
| 3 | XPWMTOUT_3 |
| 4 | XPWMTOUT_4 |
| 5 | XPWMTOUT_5 |
| 6 | XPWMTOUT_6 |
