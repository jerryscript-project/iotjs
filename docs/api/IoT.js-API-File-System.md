### Platform Support

The following shows fs module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| fs.close | O | O | O |
| fs.closeSync | O | O | O |
| fs.open | O | O | O |
| fs.openSync | O | O | O |
| fs.read | O | O | O |
| fs.readSync | O | O | O |
| fs.readDir | O | O | X |
| fs.readDirSync | O | O | X |
| fs.readFile | O | O | O |
| fs.readFileSync | O | O | O |
| fs.rename | O | O | O |
| fs.renameSync | O | O | O |
| fs.stat | O | O | O |
| fs.statSync | O | O | O |
| fs.fstat | O | O | O |
| fs.fstatSync | O | O | O |
| fs.write | O | O | O |
| fs.writeSync | O | O | O |
| fs.writeFile | O | O | O |
| fs.writeFileSync | O | O | O |
| fs.unlink | O | O | O |
| fs.unlinkSync | O | O | O |

※ On `nuttx` path should be passed with a form of **absolute path**.


### Contents
- [File System](#file-system)
    - [Module Functions](#module-functions)
        - [`fs.open(path, flags[, mode], callback)`](#fsopenpath-flags-mode-callback)
        - [`fs.openSync(path, flags[, mode])`](#fsopensyncpath-flags-mode)
        - [`fs.read(fd, buffer, offset, length, position, callback)`](#fsreadfd-buffer-offset-length-position-callback)
        - [`fs.readSync(fd, buffer, offset, length, position)`](#fsreadsyncfd-buffer-offset-length-position)
        - [`fs.readFile(path[, options], callback)`](#fsreadfilepath-options-callback)
        - [`fs.readFileSync(path[, options])`](#fsreadfilesyncpath-options)
        - [`fs.rename(oldPath, newPath, callback)`](#fsrenameoldpath-newpath-callback)
        - [`fs.renameSync(oldPath, newPath)`](#fsrenamesyncoldpath-newpath)
        - [`fs.stat(path, callback)`](#fsstatpath-callback)
        - [`fs.statSync(path)`](#fsstatsyncpath)
        - [`fs.fstat(fd, callback)`](#fsfstatfd-callback)
        - [`fs.fstatSync(fd)`](#fsfstatsyncfd)
        - [`fs.write(fd, buffer, offset, length, position, callback)`](#fswritefd-buffer-offset-length-position-callback)
        - [`fs.writeSync(fd, buffer, offset, length, position)`](#fswritesyncfd-buffer-offset-length-position)
        - [`fs.writeFile(path, data, [, options], callback)`](#fswritefilepath-data--options-callback)
        - [`fs.writeFileSync(path, data, [, options])`](#fswritefilesyncpath-data--options)
        - [`fs.unlink(path, callback)`](#fsunlinkpath-callback)
        - [`fs.unlinkSync(path)`](#fsunlinksyncpath)
        - [`fs.close(fd, callback)`](#fsclosefd-callback)
        - [`fs.closeSync(fd)`](#fsclosesyncfd)
    - [Class: fs.Stats](#class-fsstats)
        - [Prototype Functions](#prototype-functions)
            - [`stats.isDirectory()`](#statsisdirectory)
            - [`stats.isFile()`](#statsisfile)


# File System


## Module Functions

### `fs.open(path, flags[, mode], callback)`
* `path <String>` - file path to be opened.
* `flags <String>` - open flags.
* `mode <Number>` Default: `0666` - permission mode.
* `callback <Function(err: Error | null, fd: Number)>`

Opens file asynchronously.

**Example**

```js
var fs = require('fs');
fs.open('test.txt', 'r', 755, function(err, fd) {
  if (err) {
    throw err;
  }
});
```

### `fs.openSync(path, flags[, mode])`
* `path <String>` - file path to be opened.
* `flags <String>` - open flags.
* `mode <Number>` Default: `0666` - permission mode.

Opens file synchronously.


### `fs.read(fd, buffer, offset, length, position, callback)`
* `fd <Integer>` - file descriptor.
* `buffer <Buffer>` - buffer that the data will be written to.
* `offset <Number>` - offset of the buffer where to start writing.
* `length <Number>` - number of bytes to read.
* `position <Number>` - specifying where to start read data from the file, if `null`, read from current position.
* `callback <Function(err: null | Error, bytesRead: Number, buffer: Buffer)>`

Reads data from the file specified by fd asynchronously.

**Example**

```js
var fs = require('fs');
fs.open('test.txt', 'r', 755, function(err, fd) {
  if (err) {
    throw err;
  }
  var buffer = new Buffer(64);
  fs.read(fd, buffer, 0, buffer.length, 0, function(err, bytesRead, buffer) {
      if (err) {
        throw err;
      }
    });
});
```


### `fs.readSync(fd, buffer, offset, length, position)`
* `fd <Integer>` - file descriptor.
* `buffer <Buffer>` - buffer that the data will be written to.
* `offset <Number>` - offset of the buffer where to start writing.
* `length <Number>` - number of bytes to read.
* `position <Number>` - specifying where to start read data from the file, if `null`, read from current position.

Reads data from the file specified by fd synchronously.


### `fs.readFile(path[, options], callback)`
* `path <String>` - file path to be opened.
* `options <Object>` - options for the operation.
  * `encoding <String> | <null>` Default: `null` - encoding of the file.
  * `flag <String>` Default: `r` - file open flag.
* `callback <Function(err: null | Error, data: Buffer)>`

Reads entire file asynchronously.

**Example**

```js
fs.readFile('test.txt', function(err, data) {
  if (err) throw err;
  console.log(data);
});
```


### `fs.readFileSync(path[, options])`
* `path <String>` - file path to be opened.
* `options <Object>` - options for the operation.
  * `encoding <String> | <null>` Default: `null` - encoding of the file.
  * `flag <String>` Default: `r` - file open flag.
* Returns: `<Buffer>`

Reads entire file synchronously.


### `fs.rename(oldPath, newPath, callback)`
* `oldPath <String>` - old file path
* `newPath <String>` - new file path
* `callback <Function(err: null | Error)>`

Renames `oldPath` to `newPath` asynchronously.

**Example**

```js
fs.rename('test.txt', 'test.txt.async', function(err) {
  if (err) throw err;
});
```


### `fs.renameSync(oldPath, newPath)`
* `oldPath <String>` - old file path
* `newPath <String>` - new file path

Renames `oldPath` to `newPath` synchronously.


### `fs.stat(path, callback)`
* `path <String>` - file path to be stated
* `callback <Function(err: null | Error, stat: Object)>`


### `fs.statSync(path)`
* `path <String>` - file path to be stated


### `fs.fstat(fd, callback)`
* `fd <Integer>` - file descriptor to be stated
* `callback <Function(err: null | Error, stat: Object)>`


### `fs.fstatSync(fd)`
* `fd <Integer>` - file descriptor to be stated

 
### `fs.write(fd, buffer, offset, length, position, callback)`
* `fd <Integer>` - file descriptor.
* `buffer <Buffer>` - buffer that the data will be written from.
* `offset <Number>` - offset of the buffer where from start reading.
* `length <Number>` - number of bytes to write.
* `position <Number>` - specifying where to start write data to the file, if `null`, read from current position.
* `callback <Function(err: null | Error, bytesWrite : Integer)>`

Writes buffer to the file specified by fd asynchronously.


### `fs.writeSync(fd, buffer, offset, length, position)`
* `fd <Integer>` - file descriptor.
* `buffer <Buffer>` - buffer that the data will be written from.
* `offset <Number>` - offset of the buffer where from start reading.
* `length <Number>` - number of bytes to write.
* `position <Number>` - specifying where to start write data to the file, if `null`, read from current position.

Writes buffer to the file specified by fd synchronously.


### `fs.writeFile(path, data, [, options], callback)`
* `path <String>` - file path that the `data` will be written
* `data <Buffer>` - buffer that contains data
* `options: Object` - options for the operation.
 * `flag: String`, Default: `w` - file open flag.
* `callback <Function(err: null | Error)>`

Writes entire `data` to the file specified by `path` asynchronously.

**Example**

```js
fs.writeFile('test.txt', 'IoT.js', function(err) {
  if (err) throw err;
});
```


### `fs.writeFileSync(path, data, [, options])`
* `path <String>` - file path that the `data` will be written
* `data <Buffer>` - buffer that contains data
* `options: Object` - options for the operation.
 * `flag: String`, Default: `w` - file open flag.

Writes entire `data` to the file specified by `path` synchronously.


### `fs.unlink(path, callback)`
* `path <String>` - file path to be removed
* `callback <Function(err: null | Error)>`

Removes the file specified by `path` asynchronously.


### `fs.unlinkSync(path)`
* `path <String>` - file path to be removed

Removes the file specified by `path` synchronously.


### `fs.close(fd, callback)`
* `fd <Integer>` - file descriptor.
* `callback <Function(err: Error | null)>`

Closes the file of fd asynchronously.


### `fs.closeSync(fd)`
* `fd <Integer>` - file descriptor.

Closes the file of fd synchronously.


# Class: fs.Stats

fs.Stats class is a object returned from ```fs.stat()```,```fs.fstat()``` and their synchronous counterparts.

**Example**

```js
fs.stat('../resources', function(err, stats) {
  if (err) throw err;

  assert.equal(stats.isDirectory(), true);
  assert.equal(stats.isFile(), false);
});
```


## Prototype Functions


### `stats.isDirectory()`
* Returns: `<Boolean>`

Returns true if stated file is a directory


### `stats.isFile()`
* Returns: `<Boolean>`

Returns true if stated file is a file
