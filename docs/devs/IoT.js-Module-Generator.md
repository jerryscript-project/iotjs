# C/C++ API to IoT.js module generator

The module generator is an automatic code generating tool, which gives help for developers to avoid writing lots of code. It generates a binding layer between a C/C++ API and the IoT.js framework and creates a native module which can be easily imported and used in JavaScript.

**NOTE:** The tool can't handle perfect when there are complex user-defined types, pointers to structures, multiple indirections etc. in the native library.

The input of the generator is a directory, which contains the C/C++ header files and the static library of the API.

1. [Dependencies](#dependencies)
2. [Features](#features)
    - [Classes](#classes)
    - [Functions](#functions)
    - [Variables](#variables)
    - [Enums](#enums)
    - [Macros](#macros)
    - [Namespaces](#namespaces)
3. [Supported types](#supported-types)
    - [Examples](#examples)
4. [Usage](#usage)
    - [Optional arguments](#optional-arguments)
5. [Quick example](#quick-example)

## Dependencies:

The tool uses libclang to analyze the given library and get the necessary informations.

#### Clang library:

```bash
apt install libclang1-6.0
```

#### Python binding for Clang:

```bash
apt install python-clang-6.0
```

(The tool has been tested with the 5.0 and 6.0 versions.)


## Features:

The C/C++ library is represented as an object in the JavaScript environment. This object is the result of the `require` function call with the right module name parameter. The generated module name is the name of the input folder with `'_module'` suffix. If the input folder is `my_api` then you can load the module with the code below:

```javascript
var lib = require('my_api_module');
```

#### Classes:

If there is a class in the C++ library, the module object has a property with the name of the class, which is a constructor. You can create an object in JavaScript, if you call the constructor with the right parameters. The returned JavaScript variable has some properties, which are represent the members and methods of the class.

C++ header:
```cpp
class MyClass {
  int x;
public:
  MyClass(): x(0) {}
  MyClass(int x): x(x) {}

  void foo(void); // print x
};
```
JS file:
```javascript
var cpp_lib = require('module_name');

var obj1 = new cpp_lib.MyClass();
var obj2 = new cpp_lib.MyClass(42);

obj1.foo(); // print 0
obj2.foo(); // print 42
```

#### Functions:

Every function from the C/C++ library are represented as properties of the library object.

**C :**

If there is a declaration, like `void foo(int);` in the C library, then the object has a property with the name `foo`.

```javascript
var c_lib = require('module_name');
c_lib.foo(42); // call the C function
```

**C++ :**

The different between C and C++ functions, that you can call C++ functions with the same name, but with different parameter lists. If there is a declaration, like `void foo(int = 0);` in the C++ library, you can use it as below. It works in the case of constructors and methods too.

```javascript
var cpp_lib = require('module_name');
cpp_lib.foo(); // call the C++ function with the default parameter
cpp_lib.foo(42);
```

**NOTE**: There are cases when you can't decide on the API side what is the real type of a JavaScript value. For example there are two overloads of a C++ function:

`void f(int);`

`void f(double);`

In the binding layer you can check that the parameter is a number or not, but you don't know it is an integer or a floating point number, so it isn't clear which overload you should call. The generator's solution for the problem is using suffixes. If you generate the binding layer for the example code above you will get a message like that:
```
WARN: The following overload of f has been renamed to f_$0 :
void f ( int )
WARN: The following overload of f has been renamed to f_$1 :
void f ( double )
```
The rigth usage of the **f** function in that case is the following:
```javascript
var cpp_lib = require('module_name');
cpp_lib.f_$0(1); // Use f_$0 with integer parameter
cpp_lib.f_$1(1.5); // Use f_$1 with floating point parameter
```


#### Variables:

The global variables of the C/C++ library are also represented as properties. If there is a declaration, like `int a;` in the C library, then the object has a property with the name `a`, and you can get and set its value, but if there is a definition, like `const int b = 42;` you can only read the value from the property and you can not modify it.

C/C++ header:
```c
int i;
```

JS file:
```javascript
var lib = require('module_name');
lib.i = 1; // set the value of 'i'
console.log(lib.i); // print 1
```

#### Enums:

Enums work like constants above. You can read the value of the enumerator from a property, but you can not modify it.

C/C++ header:
```c
enum abc {A, B, C};
```

JS file:
```javascript
var lib = require('module_name');
console.log(lib.A); // print 0
console.log(lib.B); // print 1
console.log(lib.C); // print 2
```

#### Macros:

Macros also work like constants. You can read the value of the macro from a property, but you can not modify it. There are three supported macro types.
* If the macro defines a character literal, like `#define C 'c'`.
* If the macro defines a string literal, like `#define STR "str"`.
* If the macro defines a numeric literal, or contains some kind of operation, but the result is a number, like `#defines ZERO 0` or `#define TWO 1 + 1`. It also works, if the macro contains other macro identifiers.

C/C++ header:
```c
#define ONE 1
#define TWO 2
#define THREE ONE + TWO
```

JS file:
```javascript
var lib = require('module_name');
console.log(lib.ONE); // print 1
console.log(lib.TWO); // print 2
console.log(lib.THREE); // print 3
```

#### Namespaces:

In JavaScript a namespace represented as an object, which is set to another object as property. Concretely to the object, which represent the scope where the namespace is.

C++ header:
```c
namespace my_ns {
  void foo(void);

  namespace nested {
    void foo(void);
  }
}
```

JS file:
```javascript
var cpp_lib = require('module_name');

cpp_lib.my_ns.foo(); // my_ns::foo

with (lib.my_ns.nested) {
  foo(); // my_ns::nested::foo
}
```

**NOTE**: If there is a `using` command for a namespace in the native header, you also have to call functions etc. through the namespace object. You can use `with` in JavaScript to reduce the code.

## Supported types:

The table below shows which JavaScript type represent the particular C/C++ type.

### Fundamental types:

| C/C++ type | JS type |
| - | - |
| void | undefined |
| char | one length String |
| integers (short/int/long etc.) | Number |
| enum | Number |
| float / double | Number |
| _Bool / bool | Boolean |

### Record types:

If you would like to create a record type variable you have to call a constructor through the library object.

| C/C++ type | JS type |
| - | - |
| struct / union / class | Object |

### Pointer types:

If there is a char* or a pointer to a number (short/int/float etc.) in a native function's parameter list and you call this function from JavaScript with a String or TypedArray the binding layer alloc memory for the native pointers. If after the native call the pointers won't be used you should modify the source code of the binding layer and free them.

| C/C++ type | JS type |
| - | - |
| char* | String / Null |
| signed char* | Int8Array |
| unsigned char* | Uint8Array |
| short* | Int16Array |
| unsigned short* | Uint16Array |
| int* / long* / long long* | Int32Array |
| unsigned int* / unsigned long* / unsigned long long* | Uint32Array |
| float* | Float32Array |
| double* / long double* | Float64Array |
| function pointer (only as function parameter) | Function / Null |
| record pointer (only as function parameter) | Object / Null |

**NOTE**: Other types are not supported, which means that you need to implement how you would like to use these parts of the C/C++ API.

#### Examples:

##### `void`
```c
void f(void);
```
```javascript
var a = lib.f(); // 'a' == undefined
```

##### `char`
```c
char c;
char f(char);
```
```javascript
lib.c = 'c';
var a = lib.f('b');
```

##### `integers`
```c
int i;
int f(int);
```
```javascript
lib.i = 42;
var a = lib.f(5);
```

##### `enum`
```c
typedef enum {A, B, C} my_enum;
my_enum e;
my_enum f(my_enum);
```
```javascript
lib.e = lib.B;
var a = lib.f(lib.A);
```

##### `float/double`
```c
float f;
double d;
float f(float);
double g(double);
```
```javascript
lib.f = 1.5;
lib.d = 2.5;
var f = lib.f(1.5);
var d = lib.g(lib.d);
```

##### `bool`
```c
_Bool b;
_Bool f(_Bool);
```
```javascript
lib.b = true;
var a = lib.f(false);
```

##### `char*/char[]`

If there is global pointer to a char, its value can be `null` or a `String`.

```c
char * c_ptr;
char c_arr[6];
char* f(char*);
char* g(char[5]);
```
```javascript
lib.c_ptr = 'some string';
// lib.c_arr = 'maximum string length is 5'; NOT WORK
lib.c_arr = 'works';
var f = lib.f('other string'); // returned value can be null or String
var g = lib.g('1234');
```

##### `int*/int[]`

If there is global pointer to a number, its value can be `null` or a `TypedArray`. If there is an array of numbers, it will be a `TypedArray` in the JS environment, and you can set the values by indexing.

```c
int * i_ptr;
int i_arr[5];
int* f(int*);
int* g(int[5]);
```
```javascript
var typed_array = new Int32Array(new ArrayBuffer(8), 0, 2);
typed_array[0] = 10;
typed_array[1] = 20;
lib.i_ptr = typed_array;
lib.i_ptr = null;
// lib.i_arr = typed_array; NOT WORK
lib.i_arr[0] = 1;
lib.i_arr[1] = 2;
lib.i_arr[2] = 3;
var f = lib.f(null); // returned value can be null or TypedArray
var g = lib.g(typed_array);
```

##### `function`

Function pointers supported as parameters. There can be cases when it does not work correctly, if the function will be called asynchronous.

```c
typedef int (callback)(void);

int f(callback c) {
  return c();
}
```
```javascript
var a = lib.f(function () {
  return 42;
});
```

Let's see a dummy example, when function pointers work incorrectly.

```c
typedef void (cb)(void);

cb cb_arr[2];

void foo(cb c) {
  static int i = 0;
  cb_arr[i++] = c;
}

void bar(void) {
  cb_arr[0]();
}
```
```javascript
lib.foo(function() {
  console.log('first callback');
});

lib.foo(function() {
  console.log('second callback');
});

// the second foo call overwrite the first callback function
// it will print 'second callback', which is not the expected
lib.bar();
```

##### `struct / union / class`

```cpp
typedef struct {
  int i;
  char c;
} S;

typedef union {
  int i;
  char c;
} U;

class C {
  int i = 42;
public:
  int get_i() {return i;}
};

S s;
U u;
C c;
S f(S);
U g(U);
C h(C);
void ptr(S*);
```
```javascript
var s = new lib.S();
var u = new lib.U();
var c = new lib.C();

s.i = 42;
s.c = 's';
lib.s = s;
lib.s.i = 0;

// var o = {
//   i: 42,
//   c: 'o'
// }
//
// lib.f(o); NOT WORK 'o' is not a valid S type object
var other_s = lib.f(s);
var other_u = lib.g(u);
var other_c = lib.h(c);
lib.ptr(s);

console.log(lib.c.get_i());
```

## Usage:

You can generate a module using the following command:

```bash
# assuming you are in iotjs folder
$ tools/iotjs-generate-module.py <INPUT_FOLDER> <LANG> [OPTIONS]
```

The `<INPUT_FOLDER>` should contain the header files and the static library of the C/C++ API. `<LANG>` is the language of the API, which can be `c` or `c++`. These are required arguments for the script. The script generates the source files to the `iotjs/tools/module_generator/output/<INPUT_FOLDER>_module/` folder. The module name will be `<INPUT_FOLDER>_module`. If you would like to modify how the module should work, you have to make some changes in the generated `.c` or `.cpp` file.

#### Optional arguments:

The script has some optional arguments, which are the following:

##### `--out-dir`

The output folder of the generated module. Default is `tools/module_generator/output`

```bash
$ tools/iotjs-generate-module.py <INPUT_FOLDER> <LANG> --out-dir <OUTPUT_FOLDER>
```

##### `--off`
* `functions` | `variables` | `enums` | `macros`

Turn off the processing of the given part of the API, which means that the script will not generate any code for this part, so you can not use this in the JS environment.

```bash
$ tools/iotjs-generate-module.py <INPUT_FOLDER> <LANG> --off=enums --off=macros
```

##### `--define`

Define a macro for the clang preprocessor.

```bash
$ tools/iotjs-generate-module.py <INPUT_FOLDER> <LANG> --define FOO --define BAR=42
```

##### `--defines`

A file, which contains macro definitions for the clang preprocessor.

`macro_defines.txt`:
```txt
FOO
BAR=42
```

```bash
$ tools/iotjs-generate-module.py <INPUT_FOLDER> <LANG> --defines macro_defines.txt
```

##### `--include`

Add include path to search for other files.

```bash
$ tools/iotjs-generate-module.py <INPUT_FOLDER> <LANG> --include path/to/the/include/folder/
```

##### `--includes`

A file, which contains include paths.

`includes.txt`:
```txt
path/to/include/folder
other/path/to/other/folder
```

```bash
$ tools/iotjs-generate-module.py <INPUT_FOLDER> <LANG> --includes includes.txt
```

## Quick example:

#### Directory structure:

* iotjs/
* my_api/
  * foo/
    * foo.h
  * bar.h
  * libexample.a

#### Header files:

foo.h:
```c
#define N 10
int foo(int x); //return x+x
```

bar.h:
```c
typedef enum {A, B, C} flags;
void bar(); // print "Hello!"
```

#### Build:
```bash
# assuming you are in iotjs folder
$ tools/iotjs-generate-module.py ../my_api/ c
tools/build.py --external-module=tools/module_generator/output/my_api_module --cmake-param=-DENABLE_MODULE_MY_API_MODULE=ON
```

#### Usage:
api.js:
```javascript
// the name of the module is same as the directory name with '_module' suffix
var c_lib = require('my_api_module');
var x = c_lib.foo(2);
console.log(x); // print 4
c_lib.bar(); // print 'Hello!'
console.log(c_lib.N); // print 10
console.log(c_lib.B); // print 1
```

#### Generated binding layer:
my_api_js_binding.c:
```c
#include <stdlib.h>
#include <string.h>
#include "jerryscript.h"
#include "my_api_js_binding.h"


// external function for API functions or for getters / setters
jerry_value_t bar_handler (const jerry_value_t function_obj,
                      const jerry_value_t this_val,
                      const jerry_value_t args_p[],
                      const jerry_length_t args_cnt)
{

  // check the count of the external function's arguments
  if (args_cnt != 0)
  {
    char const *msg = "Wrong argument count for bar(), expected 0.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // native function call
  bar ();


  jerry_value_t ret_val = jerry_create_undefined ();

  return ret_val;
}


// external function for API functions or for getters / setters
jerry_value_t foo_handler (const jerry_value_t function_obj,
                      const jerry_value_t this_val,
                      const jerry_value_t args_p[],
                      const jerry_length_t args_cnt)
{

  // check the count of the external function's arguments
  if (args_cnt != 1)
  {
    char const *msg = "Wrong argument count for foo(), expected 1.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[0]))
  {
    char const *msg = "Wrong argument type for foo(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  int arg_0 = (int)jerry_get_number_value (args_p[0]);

  // native function call
  int result = foo (arg_0);


  jerry_value_t ret_val = jerry_create_number (result);

  return ret_val;
}


// init function for the module
jerry_value_t Init_my_api()
{

  jerry_value_t object = jerry_create_object();


  // set an external function as a property to the module object
  jerry_value_t bar_name = jerry_create_string ((const jerry_char_t*)"bar");
  jerry_value_t bar_func = jerry_create_external_function (bar_handler);
  jerry_value_t bar_ret = jerry_set_property (object, bar_name, bar_func);
  jerry_release_value (bar_name);
  jerry_release_value (bar_func);
  jerry_release_value (bar_ret);


  // set an external function as a property to the module object
  jerry_value_t foo_name = jerry_create_string ((const jerry_char_t*)"foo");
  jerry_value_t foo_func = jerry_create_external_function (foo_handler);
  jerry_value_t foo_ret = jerry_set_property (object, foo_name, foo_func);
  jerry_release_value (foo_name);
  jerry_release_value (foo_func);
  jerry_release_value (foo_ret);


  // set an enum constant as a property to the module object
  jerry_property_descriptor_t A_prop_desc;
  jerry_init_property_descriptor_fields (&A_prop_desc);
  A_prop_desc.is_value_defined = true;
  A_prop_desc.value = jerry_create_number (A);
  jerry_value_t A_name = jerry_create_string ((const jerry_char_t *)"A");
  jerry_value_t A_ret = jerry_define_own_property (object, A_name, &A_prop_desc);
  jerry_release_value (A_ret);
  jerry_release_value (A_name);
  jerry_free_property_descriptor_fields (&A_prop_desc);


  // set an enum constant as a property to the module object
  jerry_property_descriptor_t B_prop_desc;
  jerry_init_property_descriptor_fields (&B_prop_desc);
  B_prop_desc.is_value_defined = true;
  B_prop_desc.value = jerry_create_number (B);
  jerry_value_t B_name = jerry_create_string ((const jerry_char_t *)"B");
  jerry_value_t B_ret = jerry_define_own_property (object, B_name, &B_prop_desc);
  jerry_release_value (B_ret);
  jerry_release_value (B_name);
  jerry_free_property_descriptor_fields (&B_prop_desc);


  // set an enum constant as a property to the module object
  jerry_property_descriptor_t C_prop_desc;
  jerry_init_property_descriptor_fields (&C_prop_desc);
  C_prop_desc.is_value_defined = true;
  C_prop_desc.value = jerry_create_number (C);
  jerry_value_t C_name = jerry_create_string ((const jerry_char_t *)"C");
  jerry_value_t C_ret = jerry_define_own_property (object, C_name, &C_prop_desc);
  jerry_release_value (C_ret);
  jerry_release_value (C_name);
  jerry_free_property_descriptor_fields (&C_prop_desc);


  jerry_value_t N_js = jerry_create_number (N);


  // set a global constant or a macro as a property to the module object
  jerry_property_descriptor_t N_prop_desc;
  jerry_init_property_descriptor_fields (&N_prop_desc);
  N_prop_desc.is_value_defined = true;
  N_prop_desc.value = N_js;
  jerry_value_t N_prop_name = jerry_create_string ((const jerry_char_t *)"N");
  jerry_value_t N_return_value = jerry_define_own_property (object, N_prop_name, &N_prop_desc);
  jerry_release_value (N_return_value);
  jerry_release_value (N_prop_name);
  jerry_free_property_descriptor_fields (&N_prop_desc);

  return object;
}
```
