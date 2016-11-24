## Module: fs

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


### Methods

#### fs.close(fd, callback)

* `fd: Int` - file descriptor.
* `callback: Function(err) `
 * `err: Error`

Closes the file of fd asynchronously.


#### fs.closeSync(fd)

* `fd: Int` - file descriptor.

Closes the file of fd synchronously.


#### fs.open(path, flags[, mode], callback)

* `path: String` - file path to be opened.
* `flags: String` - open flags.
* `mode: Number`, Default: `0666` - permission mode.
* `callback: Function(err, fd)`
 * `err: Error` - `Error` object if there was something wrong, otherwise `null`.
 * `fd: Number` - file descriptor.


#### fs.openSync(path, flags[, mode])

* `path: String` - file path to be opened.
* `flags: String` - open flags.
* `mode: Number`, Default: `0666` - permission mode.


#### fs.read(fd, buffer, offset, length, position, callback)

* `fd: Int` - file descriptor.
* `buffer: Buffer` - buffer that the data will be written to.
* `offset: Number` - offset of the buffer where to start writing.
* `length: Number` - number of bytes to read.
* `position: Number` - specifying where to start read data from the file, if `null`, read from current position.
* `callback: Function(err, bytesRead, buffer) `
 * `err: Error`
 * `bytesRead: Number`
 * `buffer: Buffer`


#### fs.readSync(fd, buffer, offset, length, position)

* `fd: Int` - file descriptor.
* `buffer: Buffer` - buffer that the data will be written to.
* `offset: Number` - offset of the buffer where to start writing.
* `length: Number` - number of bytes to read.
* `position: Number` - specifying where to start read data from the file, if `null`, read from current position.


#### fs.readFile(path[, options], callback)

* `path: String` - file path to be opened.
* `options: Object` - options for the operation.
 * `encoding: String`, Default: `null` - encoding of the file.
 * `flag: String`, Default: `r` - file open flag.
* `callback: Function(err, data)` - callback function.
 * `err: Error`
 * `data: Buffer`

Reads entire file asynchronously.


#### fs.readFileSync(path[, options])

* `path: String` - file path to be opened.
* `options: Object` - options for the operation.
 * `encoding: String`, Default: `null` - encoding of the file.
 * `flag: String`, Default: `r` - file open flag.

Reads entire file synchronously.


#### fs.rename(oldPath, newPath, callback)

* `oldPath: String` - old file path
* `newPath: String` - new file path
* `callback: Function(err)` - callback function.
 * `err: Error`

Renames `oldPath` to `newPath` asynchronously.


#### fs.renameSync(oldPath, newPath)

* `oldPath: String` - old file path
* `newPath: String` - new file path

Renames `oldPath` to `newPath` synchronously.


#### fs.stat(path, callback)

* `path: String` - file path to be stated
* callback: Function(err, stat)` - callback function.
 * `err: Error`
 * `stat: Object`


#### fs.statSync(path)

* `path: String` - file path to be stated


#### fs.fstat(fd, callback)

* `fd: Number` - file descriptor to be stated
* callback: Function(err, stat)` - callback function.
 * `err: Error`
 * `stat: Object`


#### fs.fstatSync(fd)

* `fd: Number` - file descriptor to be stated

 ### Class: fs.Stats

 fs.Stats class is a object returned from ```fs.stat()```,```fs.fstat()``` and their synchronous counterparts.

 ### Methods

 #### stats.isDirectory()

 Returns true if stated file is a directory

 #### stats.isFile()

 Returns true if stated file is a file


#### fs.write(fd, buffer, offset, length, position, callback)

* `fd: Int` - file descriptor.
* `buffer: Buffer` - buffer that the data will be written from.
* `offset: Number` - offset of the buffer where from start reading.
* `length: Number` - number of bytes to write.
* `position: Number` - specifying where to start write data to the file, if `null`, read from current position.
* callback: Function(err, bytesWrite)` - callback function.
 * `err: Error`
 * `byteWrite: Int`


#### fs.writeSync(fd, buffer, offset, length, position)

* `fd: Int` - file descriptor.
* `buffer: Buffer` - buffer that the data will be written from.
* `offset: Number` - offset of the buffer where from start reading.
* `length: Number` - number of bytes to write.
* `position: Number` - specifying where to start write data to the file, if `null`, read from current position.


#### fs.writeFile(path, data, [, options], callback)

* `path: String` - file path that the `data` will be written
* `data: Buffer` - buffer that contains data
* `options: Object` - options for the operation
* `callback: Function(err)` - callback function
 * `err: Error`

Writes entire `data` to the file specified by `path` asynchronously.


#### fs.writeFileSync(path, data, [, options])

* `path: String` - file path that the `data` will be written
* `data: Buffer` - buffer that contains data
* `options: Object` - options for the operation

Writes entire `data` to the file specified by `path` synchronously.


#### fs.unlink(path, callback)

* `path: String` - file path to be removed
* `callback: Function(err) `
 * `err: Error`

Removes the file specified by `path` asynchronously.


#### fs.unlinkSync(path)

* `path: String` - file path to be removed

Removes the file specified by `path` synchronously.
