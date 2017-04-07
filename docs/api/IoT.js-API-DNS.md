### Platform Support

The following shows dns module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| dns.lookup | O | O | X |

※ dns.lookup on `nuttx` works but due to lack of `nuttx` implementation, internally it only allows valid `ip4` address,
but we are going to support `ip6` address, soon.


### Contents

- [DNS](#dns)
    - [Properties](#properties)
        - [`dns.ADDRCONFIG`](#dnsaddrconfig)
        - [`dns.V4MAPPED`](#dnsv4mapped)
    - [Module Functions](#module-functions)
        - [dns.lookup(hostname[, options], callback)](#dnslookuphostname-options-callback)


# DNS


## Properties


### `dns.ADDRCONFIG`
* `<Number>`

Returned address types are determined by the types of addresses supported by the current system.


### `dns.V4MAPPED`
* `<Number>`

If the IPv6 family was specified, but no IPv6 addresses were found, then return IPv4 mapped IPv6 addresses. 


## Module Functions


### `dns.lookup(hostname[, options], callback)`
* `hostname <String>`
* `options <Object> | <Number>`
  * `family <Number>` - The record family. Must be 4 or 6. IPv4 and IPv6 addresses are both returned by default.
  * `hints <Number>` - One or more supported getaddrinfo flags. Multiple flags may be passed by bitwise ORing their values.
* `callback <Function(err: Error, address: String, family: Number)>`
  * `err <Error>`
  * `address <String>` - A string representation of an IPv4 or IPv6 address.
  * `family <Number>` - 4 or 6, denoting the family of address.

Resolves a hostname into the first found A (IPv4) or AAAA (IPv6) record.

**Example**

```js
const dns = require('dns');
dns.lookup ('localhost', 4, function(err, ip, family) {
  assert.equal (err, null);
  assert.equal (isIPv4 (ip), true);
  assert.equal (ip, '127.0.0.1');
  assert.strictEqual (family, 4);
});
```
