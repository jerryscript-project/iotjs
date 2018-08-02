### Platform Support

The following chart shows the availability of each HTTP Signature module API function on each platform.

|  | Linux<br/>(Ubuntu) | Tizen<br/>(Raspberry Pi) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: | :---: |
| httpSignature.parseRequest  | O | O | O | O | O |
| httpSignature.verify  | O | O | O | O | O |

# HTTP Signature
The module makes it possible to verify the signature on HTTP Requests as stated in the RFC Standard (https://tools.ietf.org/html/draft-cavage-http-signatures-10).

### httpSignature.parseRequest(request)
Parses an `HTTP request` and returns with the parsed object.
  - `request` {Object} A valid `HTTP Request`

The returned object can be used to later to `verify` the `signature` of the `request`.

### httpSignature.verify(parsedRequest, publicKey)
Verifies the `parsedRequest`'s `signature` against the given `publicKey`. Returns `true` if the verification succeeds, `false` otherwise.
  - `parsedRequest` {Object} An `HTTP Request` parsed by `httpSignature.parseRequest()` function.
  - `publicKey` {Buffer | string} The RSA Public key.

**Example**
```js
var httpSign = require('http_signature');

...

function myHTTPListener(req, res) {
  var parsedRequest = httpSign.parseRequest(req);
  if (!httpSign.verify(parsedRequest))
    // Verification failed
    return res.send(401);
  }

  // Signature is OK, handle the request normally
  requestHandler(req, res);
}
```
