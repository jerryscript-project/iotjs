## Module: dns

### Platform Support

The following shows dns module APIs available for each platform.

|  | Linux<br/>(Ubuntu) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) |
| :---: | :---: | :---: | :---: |
| dns.lookup | O | O | X |

※ dns.lookup on `nuttx` works but due to lack of `nuttx` implementation, internally it only allows valid `ip4` address,
but we are going to support `ip6` address, soon.

IoT.js provides dns module.

### Methods

#### dns.lookup(hostname[, options], callback)
* `hostname: String`
* `options: Object | Number`
* `callback: Function(err: Error, address: String, family: Number)`

Resolves a hostname into the first found A (IPv4) or AAAA (IPv6) record.
