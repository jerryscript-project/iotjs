### Getting Started with Examples
As **IoT.js** is asynchronous and event-driven, programming style is pretty much different from traditional blocking synchronous style. This tutorial lets you know how to code with **IoT.js** mainly focused on asynchronous and event-driven style.

#### Hello World
Firstly, create a javascript file (e.g. `hello.js`) and open it. Then type as following.
```javascript
console.log('Hello, world!');
```

You must be familiar with the code above if you have ever worked with Javascript in web. This is exactly same way as in major web browsers.

You can run it with:
```
$ ./iotjs hello.js
```

Then it gives:
```
Hello, world!
```

Pretty simple. But where did `console` come from? `console` is not defined in Global Object according to ECMAScript spec.

The answer is `console` is a builtin module so it should have been `require`ed. However, `console` is a special case so we can use it directly without `require`. This is about Module System which we will cover later.

#### File Reader
To read a file, we need to import *File System* module.
When importing a module, we use function `require`.
*File System* module is abbreviated as `fs`. You can import it like:
```javascript
var fs = require('fs');
```

Then we can use APIs of `fs`. To read the whole file, use `readFile()`.
```javascript
fs.readFile("hello_iotjs.txt",   // path
            readFileCallback);   // callback
```
Let's say we want to read `hello_iotjs.txt`. Just pass it as first argument.
It may be enough for synchronous style but we must specify a callback function as last argument. `fs.readFile` does not wait for I/O to be done, it just goes on to next code. Sometime after file opening is completely done, callback will be called. This means that we must not implement `readFile` handling code on the next line, but in callback function.

Take a look at the callback function for `fs.readFile` below.
```javascript
function readFileCallback(err, data) {
  if (err) throw err;
  console.log(data.toString());
}
```
We can find two arguments in this function. We can think of them as the results of `fs.open`. In this case(`fs.readFile`), `err` and `data` are given. Of course, each API function has their own argument list.

`err` is an `Error` object. We just throw it if an error has occurred. Otherwise we have successfully read the file content and it is stored in `data`. In this example, it only prints the file content but you can do anything you want.

##### full code list
```javascript
var fs = require('fs');

fs.readFile("hello_iotjs.txt",   // path
            readFileCallback);   // callback

function readFileCallback(err, data) {
  if (err) throw err;
  console.log(data.toString());
}
```

#### TCP Echo Server
`net` module provides APIs for creating servers and clients. In this tutorial, we are going to create a server only. We can connect to the server and test it with external tools(e.g. `nc`).

Firstly, we need to require `net` module. 
```javascript
var net = require('net');
var port = 1235;  // custom port number
```
Then create a server with `net.createServer()`. It could have some arguments
```javascript
var server = net.createServer();
```
After creating a server, make the server listen for connections.
```javascript
server.listen(port);
```
By calling `listen`, object `server` starts listening with given port. Of course `listen` is processed asynchronously but we do not have to specify a callback. What we want to do next is not necessarily done in the callback because we are just going to add some event handlers.

**IoT.js** is event-driven. We can do a lot of stuff with event handlers.
Both `Server` and `Socket` object inherits from `EventsEmitter`, so we can add event listeners for them. For servers, we probably want to add a listener for `'connection'` event which is emitted when a new connection is made. Take a look at the following.
```javascript
server.on('connection', function(socket) {
  socket.on('data', function(data) {
    socket.write("echo: " + data);
  });
});
```
In `File Reader` example, we defined callbacks outside and referred them as arguments. In this time the function is embedded as a Function Expression.

When `'connection'` event is emitted, it creates a socket and we can get it from the first argument. In the same way we did for server, add a ``data`` listener for each socket which is emitted when data is received. As we are creating an echo server, what we want to do here is just send the `data` back to client. Note that to clarify this is an echoed data, `"echo: "` is prepended to it.

That is all. We just implemented an echo server less than 10 lines. Actually, the server will run forever because we did not add code for closing the server. As long as the server is listening, it does not terminate even if there is no more *javascript* code to run. As this is a simple tutorial, just kill the process manually like pressing `Ctrl+C`

##### full code list

```javascript
var net = require('net');
var port = 1235;

var server = net.createServer();
server.listen(port, 5);
server.on('connection', function(socket) {
  socket.on('data', function(data) {
    socket.write("echo: " + data);
  });
});
```
##### test the server
We have created a server but not a client. Instead of implementing a client, we are going to use a unix tool `nc`.

Run the server first:
```
$ ./iotjs echo_server.js &
```

Connect to the server with `nc`
```
$ nc localhost 1235 
```
Type whatever you want to send, and the message will be echoed back.
```
hello, echo server!
echo: hello, echo server!
```

### Module System
Javascript(ECMAScript 5.1 or under) itself does not support module system. In web browsers, even though a web page loads several Javascript files, they are evaluated in the same context. To overcome this language limit, **IoT.js** supports [CommonJS](http://www.commonjs.org/) modules.

We have used some of native modules through the examples above. When importing those modules, we use `require` function. Once `require`ed a module, we can use its APIs that are exported from the module. It will be covered in the section [Writing user modules](#writing-user-modules).

#### Writing user modules
When writing a module, APIs must be exposed by adding it in `exports` object. Otherwise it can be used only inside the module. Object `exports` will be returned when another module calls `require`.

Let's write a sample module. Save it as `mymodule.js`
```javascript
exports.hello = 'Hello, IoT.js!';   // string
exports.add = function(a, b) {      // function
  return a + b;
}
var local = 'local string';         // string (not exported)
```
Let's write another module that uses the module we just wrote. By calling `require`, we get its `exports` object. We will name it `mymodule_test.js`
```javascript
var mymodule = require('mymodule');
console.log(mymodule.hello);
console.log(mymodule.add(1, 2));
console.log(mymodule.local);
```
Save two files in the same directory so **IoT.js** can automatically find `mymodule.js`. Then we are ready to go. Execute the later script then you will see:
```
$ ./iotjs mymodule_test.js
Hello, IoT.js!
3
undefined
```
Note that `console.log(local)` prints `undefined`. It cannot be referred because it is not added in `exports`.