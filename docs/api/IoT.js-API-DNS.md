### Platform Support

The following shows dns module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | NuttX<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: |
| dns.lookup | O | O | X | - |

※ dns.lookup currently only returns IPv4 addresses. Support for IPv6 addresses are on the roadmap.


# DNS

The `dns` module provides a method to perform host name resolution. The functionality is
implemented on top of the underlying operation system facilities. Please see
[Implementation considerations section](#implementation-considerations).

The following properties are flags which can be passed as hints for the
[`dns.lookup()`](#dnslookuphostname-options-callback) method.

### dns.ADDRCONFIG
* `{number}`

Returned address types are determined by the types of addresses supported by the current system.


### dns.V4MAPPED
* `{number}`

If the IPv6 family was specified, but no IPv6 addresses were found, then return IPv4 mapped IPv6 addresses.



### dns.lookup(hostname[, options], callback)
* `hostname` {string} Hostname to be resolved.
* `options` {Object|number}
  * `family` {number} The record family. If specified must be 4. Specifies that IPv4 addresses should be returned.
  * `hints` {number} One or more supported getaddrinfo flags. Multiple flags may be passed by bitwise `OR`ing their values.
* `callback` {Function}
  * `err` {Error|null} If there is no error the value is `null`.
  * `address` {string} A string representation of an IPv4 address.
  * `family` {number} 4, denoting the family of `address`.

Resolves a hostname (e.g. `iotjs.net`) into the first found A (IPv4) or AAAA (IPv6) record. All `option` properties
are option. If `option` is a number, then it must be `4`. If `options` is not provided, then IPv4 addresses are
returned if found.

On error, `err` is an `Error` object where `err.code` is the error code returned by the underlying system call.

The `dns.lookup()` does not necessarily do DNS requests to resolve a hostname. The implementation uses an
operating system facility to associate names with addresses. Please read the
[Implementation considerations section](#implementation-considerations) for more system dependent information.

**Example**

```js
var dns = require('dns');
dns.lookup ('localhost', 4, function(err, ip, family) {
  console.log('ip: ' + ip + ' family: ' + family);
});
// ip: 127.0.0.1 family: 4

var options = {
  family: 4,
  hints: dns.ADDRCONFIG,
};

dns.lookup ('iotjs.net', options, function(err, ip, family) {
  console.log('ip: ' + ip + ' family: ' + family);
});
// ip: 192.30.252.154 family: 4
```

# Implementation considerations

The current implementation only supports host name resolution to IPv4 addresses.

On NuttX currently only valid IPv4 addresses are allowed for the
[`dns.lookup()`](#dnslookuphostname-options-callback) method.
