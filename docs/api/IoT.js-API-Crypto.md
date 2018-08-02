### Platform Support

The following chart shows the availability of each Crypto module API function on each platform.

|  | Linux<br/>(Ubuntu) | Tizen<br/>(Raspberry Pi) | Raspbian<br/>(Raspberry Pi) | Nuttx<br/>(STM32F4-Discovery) | TizenRT<br/>(Artik053) |
| :---: | :---: | :---: | :---: | :---: | :---: |
| crypto.createHash  | O | O | O | O | O |
| crypto.createVerify  | O | O | O | O | O |
| crypto.getHashes  | O | O | O | O | O |

# Crypto

The module provides limited cryptographic functionality, namely hashing and signature verification.
To access this module use `require('crypto')`.

### crypto.createVerify(hashType)
Creates and returns a `Verify` object. This object can not be created with the `new` keyword.
  - `hashType` {string} Hash type of the signature. {`sha1 | sha256`}

Note: We currently only support `rsa-sha1` and `rsa-sha256` signatures.

### crypto.createHash(hashType)
Creates and returns a `Hash` object. This object can not be created with the `new` keyword.
  - `hashType` {string} Type of the hash. {`sha1 | sha256`}

Note: We currently only support `sha1` and `sha256` hashes.

### crypto.getHashes()
Returns the available hashing methods.

## Class: Verify
The `Verify` class allows the user to verify a signature against a public key.

### verify.update(data)
Updates the `Verify` object with the given `data`.
  - `data` {Buffer | string} Updates the object with the `data`. If there is already `data` in the object, concatenates them.

**Example**
```js
var crypto = require('crypto');
var myVerifyObject = crypto.createVerify('sha256');

myVerifyObject.update('This data should be verified');
myVerifyObject.update('\nAnd this belongs there too.');
```

### verify.verify(publicKey, signature)
Verifies the `signature` against the `publicKey` using the `data` added with `verify.update()`.
  - `publicKey` {string | Buffer} A valid RSA Public key.
  - `signature` {string | Buffer} A base64 encoded `rsa-sha1` or `rsa-sha256` signature.

Returns `true` if the verification succeeds, `false` otherwise.

**Example**
```js
var crypto = require('crypto');

var myVerifyObject = crypto.createVerify('sha256');
var myKey = getPublicKeySomehow();
var myData = getSomeStringToVerify();
var mySignature = getSignatureSomehow();


myVerifyObject.update(myData);
var success = myVerifyObject.verify(myKey, mySignature);

if (!success) {
  throw new Error('Invalid signature !');
}
```

## Class: Hash
The `Hash` class creates hashes from the data given.

### hash.update(data)
Updates the `Hash` object with the given `data`.
  - `data` {Buffer | String} Updates the object with the `data`. If there is already `data` in the object, concatenates them.

### hash.digest(encoding)
Returns an `encoded` hash of the input `data` as a `string` or `Buffer`.
  - `encoding` {string} Encodes the result of the hashing to the given format. Can be {`hex | base64`}. If no `encoding` is given, or the given `encoding` doesn't match the known formats, returns the raw `hash` in a `Buffer`.

Digest can only be called once on a given `Hash` object.

**Example**
```js
var crypto = require('crypto');

var myData = 'Some data to hash';
var myHashObj = crypto.createHash('sha1');
myHashObj.update(myData);
var myHash = myHashObj.digest('hex');
```
