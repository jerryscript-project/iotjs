## Class: fs


### Methods

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

#### fs.readSync(fd, buffer, offset, length, position, callback)

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

Asynchronously read entire file.

#### fs.readFileSync(path[, options], callback)

* `path: String` - file path to be opened.
* `options: Object` - options for the operation.
 * `encoding: String`, Default: `null` - encoding of the file.
 * `flag: String`, Default: `r` - file open flag.

synchronously read entire file.

#### fs.write(fd, buffer, offset, length, position, callback) 

* `fd: Int` - file descriptor.
* `buffer: Buffer` - buffer that the data will be written from.
* `offset: Number` - offset of the buffer where from start reading.
* `length: Number` - number of bytes to write.
* `position: Number` - specifying where to start write data to the file, if `null`, read from current position.
* callback: Function(err, byteWrite)` - callback function.
 * `err: Error`
 * `byteWrite: Int`

#### fs.writeSync(fd, buffer, offset, length, position) 

* `fd: Int` - file descriptor.
* `buffer: Buffer` - buffer that the data will be written from.
* `offset: Number` - offset of the buffer where from start reading.
* `length: Number` - number of bytes to write.
* `position: Number` - specifying where to start write data to the file, if `null`, read from current position.

#### fs.close(fd, callback)

* `fd: Int` - file descriptor.

Close the file of fd asynchronously.

#### fs.closeSync(fd)

* `fd: Int` - file descriptor.

Close the file of fd synchronously.

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

## Class: fs.Stats

fs.Stats class is a object returned from ```fs.stats()```,```fs.fstats()``` and their synchronous counterparts.

### Methods

#### stats.isDirectory()

return true if stated file is a directory

#### stats.isFile()

return true if stated file is a file