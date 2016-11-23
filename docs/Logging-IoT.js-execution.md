### Logging Support

IoT.js supports logging on Debug version. It can output message strings to the stderr console by default or any file you give.

To add a message line, use one of three macros in the source, defined in iotjs_module_debug.h
```
DLOG()
DDLOG()
DDDLOG()
```
DLOG is level 1 which means it's an error so that should be displayed. DDLOG is for warning messages and is level 2. DDDLOG is information you need while IoT.js is running, which is level 3. Default is 1.

### Setting logging level

In linux, use `IOTJS_DEBUG_LEVEL` environment variable to change the level, for example, if you want to see error and warning messages;
```
export IOTJS_DEBUG_LEVEL=2
```
Numbers can be 0, 1, 2 or 3. If you give 0, it will be silence, no message.

### Logging to a file

To save to a file, use also the environment variable, for example;
```
export IOTJS_DEBUG_LOGFILE="/home/iotjsdev/iotjslog.txt"
```
You must have file creation rights to that directory. File will be overwritten on every start, so don't run the program before looking inside or backing it up.

To disable logging to a file,
```
unset IOTJS_DEBUG_LOGFILE
```
will do.
