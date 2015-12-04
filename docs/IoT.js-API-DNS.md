## Module: dns

IoT.js provides dns module

### Methods

#### dns.lookup(hostname[, options], callback)
* `hostname: String`
* `options: Object | Number`
* `callback: Function(err: Error, address: String, family: Number)`

Resolves a hostname into the first found A (IPv4) or AAAA (IPv6) record. 

#### dns.resolve(hostname[, rrtype], callback)
* `hostname: String`
* `rrtype: String`
* `callback: Function(err: Error, addresses: [String])`

Resolves a hostname into an array of all records.

#### dns.reverse(ip, callback)
* `ip: String`
* `callback: Function(err: Error, addresses: [String])`

Inverse of `dns.resolve()`. Resolves an ip address into an array of all hostnames.
