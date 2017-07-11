# Native Module vs JS Module

This document provides a basic guide on when to write a builtin module in Javascript instead of C.

## What are Native module and Javascipt module?

Native module is mainly aimed to get a direct access to low-level API of the operation system and is written in C code. While, Javascript module mainly exists as a bridge between native module and users. And, in some situations, it also includes handy functions and handles errors for API coverage.

## When to write JS module

In many cases, a well-written native code could prove better performance than the JavaScript equivalent. So, a feature, where  computational performance is important, is recommended to be written in native module. However, that doesn't mean that every module needs be written in C only. Because there are many cases where the regular Javascript code is useful. Here are a few ground rules as a guide on when to write Javascript module.

1. Support Node.js API usages

   One of IoT.js purposes is to support users who get used to Node.JS api as much as possible. So the builtin basic APIs should be implemented based on this purpose. A module in Node.Js generally consists a pair of native module and Javascript module. Even though there might be some optimization points on them, leave the pairs to facilitate future maintainance.

2. Use Javascript module in the case where we can implement a feature more efficiently.

   The `efficiently` implies effciency in terms of performance, memory, implementation time, and so on. If a certain feature could be easily and rapidly implemented in Javascript but no big difference on performance or memory compared with native module, write it in javascript module. The following includes such a case but not limited to: Type conversion, basic mathematical operations, simple functionality and so on.
   
3. Avoid 'Reinventing the wheel'

   In case that a well-peformed feature already exists in builtin modules, it's basically not allowed to wipe it out to native module since it's unnecessary.

## Argument Validation & Type Conversion

For avoiding security issue, argument validation must be handled in native module. However, native module should not take a role to convert the given type for extending API coverage. It should be done in Javascript module.

  e.g) In case of `new Buffer(str[, encoding])`, native module has a responsiblity to validate if the type of the first argument is `string`. However, in user perspective, giving `Number` as the first argument also works since Javascript module appropriately handles the given type to pass it over to the corresponding native module.
