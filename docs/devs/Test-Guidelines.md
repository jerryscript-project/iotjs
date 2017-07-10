### To write a test case

Depend on the purpose of the test case (whether it's a positive or negative one), place it under `test/run_pass` or `test/run_fail` directory. The required external resources should be placed into `test/resources`.

All test case files must be named in the following form `test_<module name>[_<functionallity].js` where `<module name>`
should match one of the JS modules name in the IoT.js. If there is a test case which can not tied to a
module (like some js features) then the `iotjs` name can be used as module name. It is important to
correctly specify the module name as the test executor relies on that information.

1. Write a test case and place it into the proper directory.
2. List up the test case in [test/testsets.json](https://github.com/Samsung/iotjs/blob/master/test/testsets.json), and set attributes (timeout, skip, ...) on the test case if it needs.


### How to Test

When you build ``iotjs`` binary successfully, you can run test driver with this binary.

```bash
/path/to/iotjs tools/check_test.js
```

#### Set test options

Some basic options are provided.

Existing test options are listed as follows;
```
start-from
quiet=yes|no (default is yes)
output-file
skip-module
output-coverage=yes|no (default is no)
experimental=yes|no (default is no)
```

To give options, please use two dashes '--' **once** before the option name as described in the following sections.

Options that may need explanations.
* start-from: a test case file name where the driver starts.
* quiet: a flag that indicates if the driver suppresses console outputs of test case.
* output-file: a file name where the driver leaves output.
* skip-module: a module list to skip test of specific modules.
* output-coverage: a flag that indicates wether coverage data should be written to disk
* experimental: a flag that indicates if tests for experimental are needed

#### Options example

```bash
build/x86_64-linux/debug/bin/iotjs tools/check_test.js -- start-from=test_console.js quiet=no
```