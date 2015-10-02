/* Copyright 2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "iotjs_def.h"


namespace iotjs {

//
// strings in iotjs built-in scripts
//
#define JERRY_MAGIC_STRING_ITEMS \
  MAGICSTR_EX_DEF1(process) \
  MAGICSTR_EX_DEF1(startIoTjs) \
  MAGICSTR_EX_DEF1(initGlobal) \
  MAGICSTR_EX_DEF1(initTimers) \
  MAGICSTR_EX_DEF1(initProcess) \
  MAGICSTR_EX_DEF1(module) \
  MAGICSTR_EX_DEF1(Native) \
  MAGICSTR_EX_DEF1(require) \
  MAGICSTR_EX_DEF1(runMain) \
  MAGICSTR_EX_DEF1(GLOBAL) \
  MAGICSTR_EX_DEF1(root) \
  MAGICSTR_EX_DEF1(console) \
  MAGICSTR_EX_DEF1(Buffer) \
  MAGICSTR_EX_DEF1(buffer) \
  MAGICSTR_EX_DEF1(setTimeout) \
  MAGICSTR_EX_DEF1(t) \
  MAGICSTR_EX_DEF1(timers) \
  MAGICSTR_EX_DEF1(setInterval) \
  MAGICSTR_EX_DEF1(clearTimeout) \
  MAGICSTR_EX_DEF1(clearInterval) \
  MAGICSTR_EX_DEF1(initProcessArgv) \
  MAGICSTR_EX_DEF1(initProcessEvents) \
  MAGICSTR_EX_DEF1(initProcessNextTick) \
  MAGICSTR_EX_DEF1(initProcessUncaughtException) \
  MAGICSTR_EX_DEF1(initProcessExit) \
  MAGICSTR_EX_DEF1(argv) \
  MAGICSTR_EX_DEF1(_initArgv) \
  MAGICSTR_EX_DEF1(EventEmitter) \
  MAGICSTR_EX_DEF1(events) \
  MAGICSTR_EX_DEF1(key) \
  MAGICSTR_EX_DEF1(nextTickQueue) \
  MAGICSTR_EX_DEF1(nextTick) \
  MAGICSTR_EX_DEF1(_onNextTick) \
  MAGICSTR_EX_DEF1(callbacks) \
  MAGICSTR_EX_DEF1(e) \
  MAGICSTR_EX_DEF1(_onUncaughtExcecption) \
  MAGICSTR_EX_DEF1(callback) \
  MAGICSTR_EX_DEF1(error) \
  MAGICSTR_EX_DEF1(event) \
  MAGICSTR_EX_DEF1(uncaughtException) \
  MAGICSTR_EX_DEF1(_events) \
  MAGICSTR_EX_DEF1(emit) \
  MAGICSTR_EX_DEF1(exit) \
  MAGICSTR_EX_DEF1(exitCode) \
  MAGICSTR_EX_DEF1(_exiting) \
  MAGICSTR_EX_DEF1(emitExit) \
  MAGICSTR_EX_DEF1(code) \
  MAGICSTR_EX_DEF1(doExit) \
  MAGICSTR_EX_DEF1(id) \
  MAGICSTR_EX_DEF1(filename) \
  MAGICSTR_EX_DEF1(exports) \
  MAGICSTR_EX_DEF1(cache) \
  MAGICSTR_EX_DEF1(native) \
  MAGICSTR_EX_DEF1(nativeMod) \
  MAGICSTR_EX_DEF1(native_sources) \
  MAGICSTR_EX_DEF1(fn) \
  MAGICSTR_EX_DEF1(compileNativePtr) \
  MAGICSTR_EX_DEF1(JSONParse) \
  MAGICSTR_EX_DEF1(text) \
  MAGICSTR_EX_DEF1(util) \
  MAGICSTR_EX_DEF1(consoleBuiltin) \
  MAGICSTR_EX_DEF1(binding) \
  MAGICSTR_EX_DEF1(Console) \
  MAGICSTR_EX_DEF1(info) \
  MAGICSTR_EX_DEF1(stdout) \
  MAGICSTR_EX_DEF1(format) \
  MAGICSTR_EX_DEF1(warn) \
  MAGICSTR_EX_DEF1(stderr) \
  MAGICSTR_EX_DEF1(isNull) \
  MAGICSTR_EX_DEF1(arg) \
  MAGICSTR_EX_DEF1(isUndefined) \
  MAGICSTR_EX_DEF1(isNullOrUndefined) \
  MAGICSTR_EX_DEF1(isNumber) \
  MAGICSTR_EX_DEF1(isBoolean) \
  MAGICSTR_EX_DEF1(isString) \
  MAGICSTR_EX_DEF1(isObject) \
  MAGICSTR_EX_DEF1(isFunction) \
  MAGICSTR_EX_DEF1(isBuffer) \
  MAGICSTR_EX_DEF1(inherits) \
  MAGICSTR_EX_DEF1(ctor) \
  MAGICSTR_EX_DEF1(superCtor) \
  MAGICSTR_EX_DEF1(s) \
  MAGICSTR_EX_DEF1(arrs) \
  MAGICSTR_EX_DEF1(formatValue) \
  MAGICSTR_EX_DEF1(args) \
  MAGICSTR_EX_DEF1(str) \
  MAGICSTR_EX_DEF1(v) \
  MAGICSTR_EX_DEF1(bufferBuiltin) \
  MAGICSTR_EX_DEF1(subject) \
  MAGICSTR_EX_DEF1(byteLength) \
  MAGICSTR_EX_DEF1(_builtin) \
  MAGICSTR_EX_DEF1(write) \
  MAGICSTR_EX_DEF1(copy) \
  MAGICSTR_EX_DEF1(list) \
  MAGICSTR_EX_DEF1(pos) \
  MAGICSTR_EX_DEF1(equals) \
  MAGICSTR_EX_DEF1(otherBuffer) \
  MAGICSTR_EX_DEF1(compare) \
  MAGICSTR_EX_DEF1(target) \
  MAGICSTR_EX_DEF1(targetStart) \
  MAGICSTR_EX_DEF1(sourceStart) \
  MAGICSTR_EX_DEF1(sourceEnd) \
  MAGICSTR_EX_DEF1(boundRange) \
  MAGICSTR_EX_DEF1(offset) \
  MAGICSTR_EX_DEF1(remaining) \
  MAGICSTR_EX_DEF1(start) \
  MAGICSTR_EX_DEF1(end) \
  MAGICSTR_EX_DEF1(low) \
  MAGICSTR_EX_DEF1(upper) \
  MAGICSTR_EX_DEF1(type) \
  MAGICSTR_EX_DEF1(err) \
  MAGICSTR_EX_DEF1(listeners) \
  MAGICSTR_EX_DEF1(len) \
  MAGICSTR_EX_DEF1(addListener) \
  MAGICSTR_EX_DEF1(listener) \
  MAGICSTR_EX_DEF1(on) \
  MAGICSTR_EX_DEF1(once) \
  MAGICSTR_EX_DEF1(f) \
  MAGICSTR_EX_DEF1(removeListener) \
  MAGICSTR_EX_DEF1(removeAllListeners) \
  MAGICSTR_EX_DEF1(fs) \
  MAGICSTR_EX_DEF1(Module) \
  MAGICSTR_EX_DEF1(parent) \
  MAGICSTR_EX_DEF1(wrapper) \
  MAGICSTR_EX_DEF1(wrap) \
  MAGICSTR_EX_DEF1(moduledirs) \
  MAGICSTR_EX_DEF1(platform) \
  MAGICSTR_EX_DEF1(nuttx) \
  MAGICSTR_EX_DEF1(cwd) \
  MAGICSTR_EX_DEF1(env) \
  MAGICSTR_EX_DEF1(HOME) \
  MAGICSTR_EX_DEF1(concatdir) \
  MAGICSTR_EX_DEF1(a) \
  MAGICSTR_EX_DEF1(b) \
  MAGICSTR_EX_DEF1(rlist) \
  MAGICSTR_EX_DEF1(resolveDirectories) \
  MAGICSTR_EX_DEF1(dirs) \
  MAGICSTR_EX_DEF1(resolveFilepath) \
  MAGICSTR_EX_DEF1(directories) \
  MAGICSTR_EX_DEF1(dir) \
  MAGICSTR_EX_DEF1(filepath) \
  MAGICSTR_EX_DEF1(tryPath) \
  MAGICSTR_EX_DEF1(packagepath) \
  MAGICSTR_EX_DEF1(jsonpath) \
  MAGICSTR_EX_DEF1(pkgSrc) \
  MAGICSTR_EX_DEF1(readSource) \
  MAGICSTR_EX_DEF1(pkgMainFile) \
  MAGICSTR_EX_DEF1(main) \
  MAGICSTR_EX_DEF1(resolveModPath) \
  MAGICSTR_EX_DEF1(path) \
  MAGICSTR_EX_DEF1(stats) \
  MAGICSTR_EX_DEF1(statPath) \
  MAGICSTR_EX_DEF1(isDirectory) \
  MAGICSTR_EX_DEF1(statSync) \
  MAGICSTR_EX_DEF1(ex) \
  MAGICSTR_EX_DEF1(load) \
  MAGICSTR_EX_DEF1(isMain) \
  MAGICSTR_EX_DEF1(modPath) \
  MAGICSTR_EX_DEF1(cachedModule) \
  MAGICSTR_EX_DEF1(SetModuleDirs) \
  MAGICSTR_EX_DEF1(self) \
  MAGICSTR_EX_DEF1(requireForThis) \
  MAGICSTR_EX_DEF1(constants) \
  MAGICSTR_EX_DEF1(fsBuiltin) \
  MAGICSTR_EX_DEF1(O_APPEND) \
  MAGICSTR_EX_DEF1(O_CREAT) \
  MAGICSTR_EX_DEF1(O_EXCL) \
  MAGICSTR_EX_DEF1(O_RDONLY) \
  MAGICSTR_EX_DEF1(O_RDWR) \
  MAGICSTR_EX_DEF1(O_SYNC) \
  MAGICSTR_EX_DEF1(O_TRUNC) \
  MAGICSTR_EX_DEF1(O_WRONLY) \
  MAGICSTR_EX_DEF1(Stats) \
  MAGICSTR_EX_DEF1(stat) \
  MAGICSTR_EX_DEF1(dev) \
  MAGICSTR_EX_DEF1(mode) \
  MAGICSTR_EX_DEF1(nlink) \
  MAGICSTR_EX_DEF1(uid) \
  MAGICSTR_EX_DEF1(gid) \
  MAGICSTR_EX_DEF1(rdev) \
  MAGICSTR_EX_DEF1(blksize) \
  MAGICSTR_EX_DEF1(ino) \
  MAGICSTR_EX_DEF1(size) \
  MAGICSTR_EX_DEF1(blocks) \
  MAGICSTR_EX_DEF1(S_IFMT) \
  MAGICSTR_EX_DEF1(S_IFDIR) \
  MAGICSTR_EX_DEF1(_createStat) \
  MAGICSTR_EX_DEF1(checkArgString) \
  MAGICSTR_EX_DEF1(checkArgFunction) \
  MAGICSTR_EX_DEF1(fstat) \
  MAGICSTR_EX_DEF1(fd) \
  MAGICSTR_EX_DEF1(checkArgNumber) \
  MAGICSTR_EX_DEF1(fstatSync) \
  MAGICSTR_EX_DEF1(close) \
  MAGICSTR_EX_DEF1(closeSync) \
  MAGICSTR_EX_DEF1(open) \
  MAGICSTR_EX_DEF1(flags) \
  MAGICSTR_EX_DEF1(convertFlags) \
  MAGICSTR_EX_DEF1(convertMode) \
  MAGICSTR_EX_DEF1(openSync) \
  MAGICSTR_EX_DEF1(read) \
  MAGICSTR_EX_DEF1(position) \
  MAGICSTR_EX_DEF1(cb) \
  MAGICSTR_EX_DEF1(bytesRead) \
  MAGICSTR_EX_DEF1(checkArgBuffer) \
  MAGICSTR_EX_DEF1(readSync) \
  MAGICSTR_EX_DEF1(written) \
  MAGICSTR_EX_DEF1(writeSync) \
  MAGICSTR_EX_DEF1(readFile) \
  MAGICSTR_EX_DEF1(buffers) \
  MAGICSTR_EX_DEF1(r) \
  MAGICSTR_EX_DEF1(_fd) \
  MAGICSTR_EX_DEF1(afterRead) \
  MAGICSTR_EX_DEF1(readFileSync) \
  MAGICSTR_EX_DEF1(flag) \
  MAGICSTR_EX_DEF1(rs) \
  MAGICSTR_EX_DEF1(sr) \
  MAGICSTR_EX_DEF1(w) \
  MAGICSTR_EX_DEF1(wx) \
  MAGICSTR_EX_DEF1(xw) \
  MAGICSTR_EX_DEF1(ax) \
  MAGICSTR_EX_DEF1(xa) \
  MAGICSTR_EX_DEF1(def) \
  MAGICSTR_EX_DEF1(checkArgType) \
  MAGICSTR_EX_DEF1(checkFunc) \
  MAGICSTR_EX_DEF1(assert) \
  MAGICSTR_EX_DEF1(http) \
  MAGICSTR_EX_DEF1(responseCheck) \
  MAGICSTR_EX_DEF1(server) \
  MAGICSTR_EX_DEF1(createServer) \
  MAGICSTR_EX_DEF1(req) \
  MAGICSTR_EX_DEF1(res) \
  MAGICSTR_EX_DEF1(body) \
  MAGICSTR_EX_DEF1(url) \
  MAGICSTR_EX_DEF1(data) \
  MAGICSTR_EX_DEF1(chunk) \
  MAGICSTR_EX_DEF1(endHandler) \
  MAGICSTR_EX_DEF1(writeHead) \
  MAGICSTR_EX_DEF1(Connection) \
  MAGICSTR_EX_DEF1(listen) \
  MAGICSTR_EX_DEF1(msg) \
  MAGICSTR_EX_DEF1(options) \
  MAGICSTR_EX_DEF1(method) \
  MAGICSTR_EX_DEF1(POST) \
  MAGICSTR_EX_DEF1(port) \
  MAGICSTR_EX_DEF1(headers) \
  MAGICSTR_EX_DEF1(postResponseHandler) \
  MAGICSTR_EX_DEF1(res_body) \
  MAGICSTR_EX_DEF1(equal) \
  MAGICSTR_EX_DEF1(statusCode) \
  MAGICSTR_EX_DEF1(1) \
  MAGICSTR_EX_DEF1(request) \
  MAGICSTR_EX_DEF1(GET) \
  MAGICSTR_EX_DEF1(getResponseHandler) \
  MAGICSTR_EX_DEF1(2) \
  MAGICSTR_EX_DEF1(getReq) \
  MAGICSTR_EX_DEF1(finalMsg) \
  MAGICSTR_EX_DEF1(finalOptions) \
  MAGICSTR_EX_DEF1(finalResponseHandler) \
  MAGICSTR_EX_DEF1(3) \
  MAGICSTR_EX_DEF1(finalReq) \
  MAGICSTR_EX_DEF1(AssertionError) \
  MAGICSTR_EX_DEF1(actual) \
  MAGICSTR_EX_DEF1(expected) \
  MAGICSTR_EX_DEF1(operator) \
  MAGICSTR_EX_DEF1(getMessage) \
  MAGICSTR_EX_DEF1(assertion) \
  MAGICSTR_EX_DEF1(fail) \
  MAGICSTR_EX_DEF1(notEqual) \
  MAGICSTR_EX_DEF1(strictEqual) \
  MAGICSTR_EX_DEF1(notStrictEqual) \
  MAGICSTR_EX_DEF1(throws) \
  MAGICSTR_EX_DEF1(block) \
  MAGICSTR_EX_DEF1(doesNotThrow) \
  MAGICSTR_EX_DEF1(Server) \
  MAGICSTR_EX_DEF1(http_server) \
  MAGICSTR_EX_DEF1(client) \
  MAGICSTR_EX_DEF1(http_client) \
  MAGICSTR_EX_DEF1(HTTPParser) \
  MAGICSTR_EX_DEF1(httpparser) \
  MAGICSTR_EX_DEF1(ClientRequest) \
  MAGICSTR_EX_DEF1(requestListener) \
  MAGICSTR_EX_DEF1(METHODS) \
  MAGICSTR_EX_DEF1(methods) \
  MAGICSTR_EX_DEF1(net) \
  MAGICSTR_EX_DEF1(IncomingMessage) \
  MAGICSTR_EX_DEF1(http_incoming) \
  MAGICSTR_EX_DEF1(OutgoingMessage) \
  MAGICSTR_EX_DEF1(http_outgoing) \
  MAGICSTR_EX_DEF1(common) \
  MAGICSTR_EX_DEF1(http_common) \
  MAGICSTR_EX_DEF1(STATUS_CODES) \
  MAGICSTR_EX_DEF1(Continue) \
  MAGICSTR_EX_DEF1(OK) \
  MAGICSTR_EX_DEF1(Created) \
  MAGICSTR_EX_DEF1(Accepted) \
  MAGICSTR_EX_DEF1(Found) \
  MAGICSTR_EX_DEF1(Unauthorized) \
  MAGICSTR_EX_DEF1(Forbidden) \
  MAGICSTR_EX_DEF1(Conflict) \
  MAGICSTR_EX_DEF1(Gone) \
  MAGICSTR_EX_DEF1(ServerResponse) \
  MAGICSTR_EX_DEF1(HEAD) \
  MAGICSTR_EX_DEF1(_hasBody) \
  MAGICSTR_EX_DEF1(statusMessage) \
  MAGICSTR_EX_DEF1(_implicitHeader) \
  MAGICSTR_EX_DEF1(reason) \
  MAGICSTR_EX_DEF1(obj) \
  MAGICSTR_EX_DEF1(unknown) \
  MAGICSTR_EX_DEF1(statusLine) \
  MAGICSTR_EX_DEF1(_headers) \
  MAGICSTR_EX_DEF1(_storeHeader) \
  MAGICSTR_EX_DEF1(assignSocket) \
  MAGICSTR_EX_DEF1(socket) \
  MAGICSTR_EX_DEF1(_httpMessage) \
  MAGICSTR_EX_DEF1(connection) \
  MAGICSTR_EX_DEF1(onServerResponseClose) \
  MAGICSTR_EX_DEF1(detachSocket) \
  MAGICSTR_EX_DEF1(allowHalfOpen) \
  MAGICSTR_EX_DEF1(httpAllowHalfOpen) \
  MAGICSTR_EX_DEF1(connectionListener) \
  MAGICSTR_EX_DEF1(clientError) \
  MAGICSTR_EX_DEF1(conn) \
  MAGICSTR_EX_DEF1(destroy) \
  MAGICSTR_EX_DEF1(timeout) \
  MAGICSTR_EX_DEF1(ms) \
  MAGICSTR_EX_DEF1(parser) \
  MAGICSTR_EX_DEF1(createHTTPParser) \
  MAGICSTR_EX_DEF1(_url) \
  MAGICSTR_EX_DEF1(onIncoming) \
  MAGICSTR_EX_DEF1(parserOnIncoming) \
  MAGICSTR_EX_DEF1(incoming) \
  MAGICSTR_EX_DEF1(socketOnData) \
  MAGICSTR_EX_DEF1(socketOnEnd) \
  MAGICSTR_EX_DEF1(socketOnError) \
  MAGICSTR_EX_DEF1(socketOnClose) \
  MAGICSTR_EX_DEF1(serverTimeout) \
  MAGICSTR_EX_DEF1(reqTimeout) \
  MAGICSTR_EX_DEF1(complete) \
  MAGICSTR_EX_DEF1(resTimeout) \
  MAGICSTR_EX_DEF1(ret) \
  MAGICSTR_EX_DEF1(execute) \
  MAGICSTR_EX_DEF1(finish) \
  MAGICSTR_EX_DEF1(_socketState) \
  MAGICSTR_EX_DEF1(shouldKeepAlive) \
  MAGICSTR_EX_DEF1(resOnFinish) \
  MAGICSTR_EX_DEF1(destroySoon) \
  MAGICSTR_EX_DEF1(prefinish) \
  MAGICSTR_EX_DEF1(stream) \
  MAGICSTR_EX_DEF1(TCP) \
  MAGICSTR_EX_DEF1(tcp) \
  MAGICSTR_EX_DEF1(createTCP) \
  MAGICSTR_EX_DEF1(SocketState) \
  MAGICSTR_EX_DEF1(connecting) \
  MAGICSTR_EX_DEF1(connected) \
  MAGICSTR_EX_DEF1(readable) \
  MAGICSTR_EX_DEF1(destroyed) \
  MAGICSTR_EX_DEF1(Socket) \
  MAGICSTR_EX_DEF1(Duplex) \
  MAGICSTR_EX_DEF1(_timer) \
  MAGICSTR_EX_DEF1(_timeout) \
  MAGICSTR_EX_DEF1(handle) \
  MAGICSTR_EX_DEF1(_handle) \
  MAGICSTR_EX_DEF1(onSocketFinish) \
  MAGICSTR_EX_DEF1(onSocketEnd) \
  MAGICSTR_EX_DEF1(connect) \
  MAGICSTR_EX_DEF1(state) \
  MAGICSTR_EX_DEF1(normalizeConnectArgs) \
  MAGICSTR_EX_DEF1(resetSocketTimeout) \
  MAGICSTR_EX_DEF1(dns) \
  MAGICSTR_EX_DEF1(host) \
  MAGICSTR_EX_DEF1(localhost) \
  MAGICSTR_EX_DEF1(dnsopts) \
  MAGICSTR_EX_DEF1(family) \
  MAGICSTR_EX_DEF1(hints) \
  MAGICSTR_EX_DEF1(_host) \
  MAGICSTR_EX_DEF1(lookup) \
  MAGICSTR_EX_DEF1(ip) \
  MAGICSTR_EX_DEF1(_destroy) \
  MAGICSTR_EX_DEF1(_write) \
  MAGICSTR_EX_DEF1(afterWrite) \
  MAGICSTR_EX_DEF1(owner) \
  MAGICSTR_EX_DEF1(status) \
  MAGICSTR_EX_DEF1(Writable) \
  MAGICSTR_EX_DEF1(clearSocketTimeout) \
  MAGICSTR_EX_DEF1(_writableState) \
  MAGICSTR_EX_DEF1(ended) \
  MAGICSTR_EX_DEF1(finished) \
  MAGICSTR_EX_DEF1(setKeepAlive) \
  MAGICSTR_EX_DEF1(enable) \
  MAGICSTR_EX_DEF1(delay) \
  MAGICSTR_EX_DEF1(msecs) \
  MAGICSTR_EX_DEF1(afterConnect) \
  MAGICSTR_EX_DEF1(onSocketConnect) \
  MAGICSTR_EX_DEF1(emitError) \
  MAGICSTR_EX_DEF1(onclose) \
  MAGICSTR_EX_DEF1(_server) \
  MAGICSTR_EX_DEF1(sockets) \
  MAGICSTR_EX_DEF1(_sockets) \
  MAGICSTR_EX_DEF1(idx) \
  MAGICSTR_EX_DEF1(count) \
  MAGICSTR_EX_DEF1(_emitCloseIfDrained) \
  MAGICSTR_EX_DEF1(maybeDestroy) \
  MAGICSTR_EX_DEF1(_readyToWrite) \
  MAGICSTR_EX_DEF1(onread) \
  MAGICSTR_EX_DEF1(readStart) \
  MAGICSTR_EX_DEF1(nread) \
  MAGICSTR_EX_DEF1(isEOF) \
  MAGICSTR_EX_DEF1(Readable) \
  MAGICSTR_EX_DEF1(_readableState) \
  MAGICSTR_EX_DEF1(shutdown) \
  MAGICSTR_EX_DEF1(normalizeListenArgs) \
  MAGICSTR_EX_DEF1(backlog) \
  MAGICSTR_EX_DEF1(listening) \
  MAGICSTR_EX_DEF1(onconnection) \
  MAGICSTR_EX_DEF1(_handleTemp) \
  MAGICSTR_EX_DEF1(clientHandle) \
  MAGICSTR_EX_DEF1(createConnection) \
  MAGICSTR_EX_DEF1(EE) \
  MAGICSTR_EX_DEF1(Stream) \
  MAGICSTR_EX_DEF1(stream_readable) \
  MAGICSTR_EX_DEF1(stream_writable) \
  MAGICSTR_EX_DEF1(stream_duplex) \
  MAGICSTR_EX_DEF1(ReadableState) \
  MAGICSTR_EX_DEF1(defaultEncoding) \
  MAGICSTR_EX_DEF1(utf8) \
  MAGICSTR_EX_DEF1(flowing) \
  MAGICSTR_EX_DEF1(endEmitted) \
  MAGICSTR_EX_DEF1(n) \
  MAGICSTR_EX_DEF1(readBuffer) \
  MAGICSTR_EX_DEF1(emitEnd) \
  MAGICSTR_EX_DEF1(ev) \
  MAGICSTR_EX_DEF1(resume) \
  MAGICSTR_EX_DEF1(isPaused) \
  MAGICSTR_EX_DEF1(pause) \
  MAGICSTR_EX_DEF1(emitData) \
  MAGICSTR_EX_DEF1(encoding) \
  MAGICSTR_EX_DEF1(onEof) \
  MAGICSTR_EX_DEF1(emitReadable) \
  MAGICSTR_EX_DEF1(er) \
  MAGICSTR_EX_DEF1(defaultHighWaterMark) \
  MAGICSTR_EX_DEF1(WriteReq) \
  MAGICSTR_EX_DEF1(WritableState) \
  MAGICSTR_EX_DEF1(hwm) \
  MAGICSTR_EX_DEF1(highWaterMark) \
  MAGICSTR_EX_DEF1(ready) \
  MAGICSTR_EX_DEF1(writing) \
  MAGICSTR_EX_DEF1(writingLength) \
  MAGICSTR_EX_DEF1(needDrain) \
  MAGICSTR_EX_DEF1(ending) \
  MAGICSTR_EX_DEF1(writeAfterEnd) \
  MAGICSTR_EX_DEF1(writeOrBuffer) \
  MAGICSTR_EX_DEF1(onwrite) \
  MAGICSTR_EX_DEF1(unreachable) \
  MAGICSTR_EX_DEF1(endWritable) \
  MAGICSTR_EX_DEF1(writeBuffered) \
  MAGICSTR_EX_DEF1(_onwrite) \
  MAGICSTR_EX_DEF1(doWrite) \
  MAGICSTR_EX_DEF1(onEmptyBuffer) \
  MAGICSTR_EX_DEF1(emitFinish) \
  MAGICSTR_EX_DEF1(emitDrain) \
  MAGICSTR_EX_DEF1(drain) \
  MAGICSTR_EX_DEF1(addHeaders) \
  MAGICSTR_EX_DEF1(_sentHeader) \
  MAGICSTR_EX_DEF1(_header) \
  MAGICSTR_EX_DEF1(_send) \
  MAGICSTR_EX_DEF1(_finish) \
  MAGICSTR_EX_DEF1(headerStr) \
  MAGICSTR_EX_DEF1(setHeader) \
  MAGICSTR_EX_DEF1(removeHeader) \
  MAGICSTR_EX_DEF1(getHeader) \
  MAGICSTR_EX_DEF1(REQUEST) \
  MAGICSTR_EX_DEF1(OnHeaders) \
  MAGICSTR_EX_DEF1(parserOnHeaders) \
  MAGICSTR_EX_DEF1(OnHeadersComplete) \
  MAGICSTR_EX_DEF1(parserOnHeadersComplete) \
  MAGICSTR_EX_DEF1(OnBody) \
  MAGICSTR_EX_DEF1(parserOnBody) \
  MAGICSTR_EX_DEF1(OnMessageComplete) \
  MAGICSTR_EX_DEF1(parserOnMessageComplete) \
  MAGICSTR_EX_DEF1(status_msg) \
  MAGICSTR_EX_DEF1(skipBody) \
  MAGICSTR_EX_DEF1(shouldkeepalive) \
  MAGICSTR_EX_DEF1(buf) \
  MAGICSTR_EX_DEF1(AddHeader) \
  MAGICSTR_EX_DEF1(dest) \
  MAGICSTR_EX_DEF1(src) \
  MAGICSTR_EX_DEF1(hostname) \
  MAGICSTR_EX_DEF1(l) \
  MAGICSTR_EX_DEF1(response) \
  MAGICSTR_EX_DEF1(setupConnection) \
  MAGICSTR_EX_DEF1(firstHeaderLine) \
  MAGICSTR_EX_DEF1(reinitialize) \
  MAGICSTR_EX_DEF1(RESPONSE) \
  MAGICSTR_EX_DEF1(parserOnIncomingClient) \
  MAGICSTR_EX_DEF1(d) \
  MAGICSTR_EX_DEF1(responseOnEnd) \
  MAGICSTR_EX_DEF1(isHeadResponse) \
  MAGICSTR_EX_DEF1(emitTimeout) \
  MAGICSTR_EX_DEF1(Timer) \
  MAGICSTR_EX_DEF1(timer) \
  MAGICSTR_EX_DEF1(TIMEOUT_MAX) \
  MAGICSTR_EX_DEF1(timersList) \
  MAGICSTR_EX_DEF1(Timeout) \
  MAGICSTR_EX_DEF1(after) \
  MAGICSTR_EX_DEF1(isrepeat) \
  MAGICSTR_EX_DEF1(handler) \
  MAGICSTR_EX_DEF1(handleTimeout) \
  MAGICSTR_EX_DEF1(timeoutObj) \
  MAGICSTR_EX_DEF1(activate) \
  MAGICSTR_EX_DEF1(repeat) \
  MAGICSTR_EX_DEF1(stop) \
  MAGICSTR_EX_DEF1(dnsBuiltin) \
  MAGICSTR_EX_DEF1(dnsException) \
  MAGICSTR_EX_DEF1(syscall) \
  MAGICSTR_EX_DEF1(errno) \
  MAGICSTR_EX_DEF1(ADDRCONFIG) \
  MAGICSTR_EX_DEF1(V4MAPPED) \
  MAGICSTR_EX_DEF1(getaddrinfo) \
  MAGICSTR_EX_DEF1(address) \
  MAGICSTR_EX_DEF1(errObj) \
  MAGICSTR_EX_DEF1(AI_ADDRCONFIG) \
  MAGICSTR_EX_DEF1(AI_V4MAPPED) \
  \
  MAGICSTR_EX_DEF2(ZZZ_001, "r+") \
  MAGICSTR_EX_DEF2(ZZZ_002, "rs+") \
  MAGICSTR_EX_DEF2(ZZZ_003, "sr+") \
  MAGICSTR_EX_DEF2(ZZZ_004, "w+") \
  MAGICSTR_EX_DEF2(ZZZ_005, "wx+") \
  MAGICSTR_EX_DEF2(ZZZ_006, "xw+") \
  MAGICSTR_EX_DEF2(ZZZ_007, "a+") \
  MAGICSTR_EX_DEF2(ZZZ_008, "ax+") \
  MAGICSTR_EX_DEF2(ZZZ_009, "xa+") \
  MAGICSTR_EX_DEF2(ZZZ_010, "Content-Length") \
  \
  MAGICSTR_EX_DEF2(ZZZ_011, "HTTP/1.1 ")\
  MAGICSTR_EX_DEF2(ZZZ_012, "127.0.0.1") \
  MAGICSTR_EX_DEF2(ZZZ_013, "/node_modules/") \
  MAGICSTR_EX_DEF2(ZZZ_014, "uncaughtException: ") \
  MAGICSTR_EX_DEF2(ZZZ_015, "Bad arguments: ") \
  MAGICSTR_EX_DEF2(ZZZ_016, "read error: ") \
  MAGICSTR_EX_DEF2(ZZZ_017, "accept error: ") \
  MAGICSTR_EX_DEF2(ZZZ_018, "0.0.0.0") \
  MAGICSTR_EX_DEF2(ZZZ_019, " HTTP/1.1\r\n")\
  MAGICSTR_EX_DEF2(ZZZ_020, ": ")\
  \
  MAGICSTR_EX_DEF2(ZZZ_021, "No module found") \
  MAGICSTR_EX_DEF2(ZZZ_022, "Bad argument: flags") \
  MAGICSTR_EX_DEF2(ZZZ_023, "close server") \
  MAGICSTR_EX_DEF2(ZZZ_025, "http request test msg") \
  MAGICSTR_EX_DEF2(ZZZ_026, "Switching Protocols") \
  MAGICSTR_EX_DEF2(ZZZ_027, "No Content") \
  MAGICSTR_EX_DEF2(ZZZ_028, "Reset Content") \
  MAGICSTR_EX_DEF2(ZZZ_029, "Partial Content") \
  MAGICSTR_EX_DEF2(ZZZ_030, "Multiple Choices") \
  MAGICSTR_EX_DEF2(ZZZ_031, "Moved Permanently") \
  MAGICSTR_EX_DEF2(ZZZ_032, "See Other") \
  MAGICSTR_EX_DEF2(ZZZ_033, "Not Modified") \
  MAGICSTR_EX_DEF2(ZZZ_034, "Use Proxy") \
  MAGICSTR_EX_DEF2(ZZZ_035, "Bad Request") \
  MAGICSTR_EX_DEF2(ZZZ_036, "Payment Required") \
  MAGICSTR_EX_DEF2(ZZZ_037, "Not Found") \
  MAGICSTR_EX_DEF2(ZZZ_038, "Method Not Allowed") \
  MAGICSTR_EX_DEF2(ZZZ_039, "Not Acceptable") \
  MAGICSTR_EX_DEF2(ZZZ_040, "Request Timeout") \
  MAGICSTR_EX_DEF2(ZZZ_041, "Length Required") \
  MAGICSTR_EX_DEF2(ZZZ_042, "Precondition Failed") \
  MAGICSTR_EX_DEF2(ZZZ_043, "Payload Too Large") \
  MAGICSTR_EX_DEF2(ZZZ_044, "URI Too Large") \
  MAGICSTR_EX_DEF2(ZZZ_045, "Unsupported Media Type") \
  MAGICSTR_EX_DEF2(ZZZ_046, "Range Not Satisfiable") \
  MAGICSTR_EX_DEF2(ZZZ_047, "Expectation Failed") \
  MAGICSTR_EX_DEF2(ZZZ_048, "Upgrade Required") \
  MAGICSTR_EX_DEF2(ZZZ_049, "Internal Server Error") \
  MAGICSTR_EX_DEF2(ZZZ_050, "Not Implemented") \
  MAGICSTR_EX_DEF2(ZZZ_051, "Bad Gateway") \
  MAGICSTR_EX_DEF2(ZZZ_052, "Service Unavailable") \
  MAGICSTR_EX_DEF2(ZZZ_053, "Gateway Time-out") \
  MAGICSTR_EX_DEF2(ZZZ_054, "invalid argument") \
  MAGICSTR_EX_DEF2(ZZZ_055, "Not running") \
  MAGICSTR_EX_DEF2(ZZZ_056, "Invalid chunk") \
  MAGICSTR_EX_DEF2(ZZZ_057, "not implemented") \
  MAGICSTR_EX_DEF2(ZZZ_058, "write after end") \
  MAGICSTR_EX_DEF2(ZZZ_059, "write during writing") \
  MAGICSTR_EX_DEF2(ZZZ_060, "socket hang up") \
  MAGICSTR_EX_DEF2(ZZZ_061, "Temporary Redirect") \
  MAGICSTR_EX_DEF2(ZZZ_062, "socket hang up") \
  \
  MAGICSTR_EX_DEF2(ZZZ_071, "%") \
  MAGICSTR_EX_DEF2(ZZZ_072, "%%") \
  MAGICSTR_EX_DEF2(ZZZ_073, "%s") \
  MAGICSTR_EX_DEF2(ZZZ_074, "%d") \
  MAGICSTR_EX_DEF2(ZZZ_075, "%j") \
  \
  MAGICSTR_EX_DEF2(ZZZ_081, ".js") \
  MAGICSTR_EX_DEF2(ZZZ_082, "==") \
  MAGICSTR_EX_DEF2(ZZZ_083, "===") \
  MAGICSTR_EX_DEF2(ZZZ_084, "!=") \
  MAGICSTR_EX_DEF2(ZZZ_085, "!==") \


//
// declare string items
//
#define MAGICSTR_EX_DEF1(NAME) \
  static const char jerry_magic_string_ex_ ## NAME[] = # NAME;
#define MAGICSTR_EX_DEF2(NAME, STRING) \
  static const char jerry_magic_string_ex_ ## NAME[] = STRING;

JERRY_MAGIC_STRING_ITEMS

#undef MAGICSTR_EX_DEF1
#undef MAGICSTR_EX_DEF2


//
// declare strings length array
//
static const jerry_api_length_t magic_string_lengths[] =
{
#define MAGICSTR_EX_DEF1(NAME) \
  (jerry_api_length_t)(sizeof(jerry_magic_string_ex_ ## NAME) - 1u),
#define MAGICSTR_EX_DEF2(NAME, STRING) \
  (jerry_api_length_t)(sizeof(jerry_magic_string_ex_ ## NAME) - 1u),

  JERRY_MAGIC_STRING_ITEMS

#undef MAGICSTR_EX_DEF1
#undef MAGICSTR_EX_DEF2
};


//
// declare strings table
//
static const jerry_api_char_ptr_t magic_string_items[] =
{
#define MAGICSTR_EX_DEF1(NAME) \
  (const jerry_api_char_ptr_t)jerry_magic_string_ex_ ## NAME,
#define MAGICSTR_EX_DEF2(NAME, STRING) \
  (const jerry_api_char_ptr_t)jerry_magic_string_ex_ ## NAME,

  JERRY_MAGIC_STRING_ITEMS

#undef MAGICSTR_EX_DEF1
#undef MAGICSTR_EX_DEF2
};


void InitJerryMagicStringEx(void) {
  uint32_t num_magic_string_items = (uint32_t)(sizeof(magic_string_items)
                                    / sizeof(jerry_api_char_ptr_t));
  jerry_register_external_magic_strings(magic_string_items,
                                        num_magic_string_items,
                                        magic_string_lengths);
}

} // namespace iotjs
