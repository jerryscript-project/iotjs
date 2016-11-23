**The content on this page does not work. Currently, there is no plan to run ipm server. This document will leave for discussion in the future.**

IoT.js follows the practice of node.js npm to provide module development community with separate IoT.js own registry.

* To see what it actually is, please visit [www.npmjs.com](https://www.npmjs.com/), and also [docs.npmjs.com](https://docs.npmjs.com/).
* To avoid confusion with the original server, we'll call it "ipm", for IoT.js Package Manager.
* Current status of "ipm" is started and it's for developers so has no separate web page. We need to develop them.
* "npm" program is used for publish and download modules. We may fork when customization is needed.

### Installing "npm"
```
sudo apt-get install npm
```

### Setting registry

As ipm uses different server, you need to change registry information
```
npm config set registry="http://ipm.iotjs.net:5984/registry/_design/app/_rewrite"
```

### Adding your account

You may have to register your account if you plan to publish some packages, for example,
```
npm set init.author.name "Your Name"
npm set init.author.email "you@example.com"
npm set init.author.url "http://yourblog.com"
npm adduser
```
Please set to your real name and email address. url is optional.

### Publish a package

cd to your package folder you wish to publish and init
```
npm init
```

It'll ask information for your package, for an example;
```
Press ^C at any time to quit.
name: (t) echotest
version: (1.0.0) 0.0.1
description: simple echo server
entry point: (index.js)
test command:
git repository:
keywords:
license: (ISC)
About to write to /(your working folder)/package.json:
{
  "name": "echotest",
  "version": "0.0.1",
  "description": "simple echo server",
  "main": "index.js",
  "scripts": {
    "test": "echo \"Error: no test specified\" && exit 1"
  },
  "author": "(your name) <your email address> (your blog page)",
  "license": "ISC"
}

Is this ok? (yes) yes
```

and all things are good to go, publish.
```
npm publish ./
```

Please visit [npmjs.org](https://docs.npmjs.com/getting-started/publishing-npm-packages) for detailed explanations.


### About the license of each packages

IoT.js is released under Apache 2.0 license, [this page](https://github.com/Samsung/iotjs/wiki/License). We assume you also agree on this license when publishing to ipm registry.

### Downloading packages

"npm" provides local and global packages and you may be working on some powerful machines. In IoT devices this may be somewhat different. It may not have enough memory, power, network or even no console to give commands. So it can have several scenarios depending on the device you may be working on. This may be some of them;

1) Download to your machine with Linux, Mac or Windows.
  * Install packages to your powerful machine
  * Send it to the IoT device through copy to memory card
  * or send through serial line, BT, USB with old time kermit or z-modem

2) If it has a WiFi the download directly from the registry
  * But to make this work, we need to develop a small shell program with iotjs.
  * This can be done with built-in module downloader, we need to develop this.
  * Issue [#75](https://github.com/Samsung/iotjs/issues/75) to track

3) If your IoT is very small and even has no writable file system
  * Package modules should be built-in to IoT.js at compile time.
  * We should also develop this.

4) any more ideas?


As for case 1),
```
(cd to some working directory)
npm install <package name>
(copy node_modules folder to your device)
```

As mentioned above, npm has local and global packages. But for IoT.js lets stick on the local only. For some cases running on ROTS there may not be any globals place.

### Package license

You may use ipm packages freely under Apache 2.0 license, read [this page](https://github.com/Samsung/iotjs/wiki/License)

### Searching for packages

Searching is possible with search command. See [docs.npmjs.com/cli/search](https://docs.npmjs.com/cli/search).

```
npm search <keyword>
```

If you omit the keyword, it'll list all packages registered.
