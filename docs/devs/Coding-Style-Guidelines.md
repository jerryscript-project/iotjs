* [Coding Style Guideline for C](#coding-style-guideline-for-c)
  * Validated Struct
  * Header Files
  * Formatting
  * Naming
  * Comments
* [Coding Style Guideline for Javascript](#coding-style-guideline-for-javascript)
  * Javascript Language Rules
  * Javascript Style Rules
    * Naming
    * Formatting
* [Coding Style Guideline for Python](#coding-style-guideline-for-python)


# Coding Style Guideline for C

Our coding style guideline is based on [google c++ coding standard](https://google.github.io/styleguide/cppguide.html),
but modified due to some difference between C and C++.
When this guideline is ambiguous, just follow the result of running `./tools/check_tidy.py`.

Here are `./tools/check_tidy.py` options:
```
--autoedit: Automatically edit the detected clang format errors. No diffs will be displayed.
```

## Validated Struct
Use [Validated Struct](../devs/Inside-IoT.js-Validated-Struct.md) whenever possible, for encapsulation and validity check.

## Header Files

### #define guard
Use #define guard in all header files. `<FILE>_H` format is recommended.

    #ifndef FILE_H
    #define FILE_H
        ...
    #endif // FILE_H

## Formatting

### Line length
maximum 80 characters in a line.

### Indentation
2 space indent at a time. Do not use a tab for indentation.

### Vertical whitespace
Add two blank lines between functions.

Otherwise minimize use of vertical whitespace.

This is more a principle than a rule: don't use blank lines when you don't have to. In particular, don't put more than two blank lines between functions, resist starting functions with a blank line, don't end functions with a blank line, and be discriminating with your use of blank lines inside functions.

### Function call
Write a function call all in a line if it fits. If not, break the line into multiple lines after assignment operator, or insert newline between parameters.
Do not insert spaces after open paren and before close paren.

    int value = foo(arg1, arg2, arg3);

    int value =
        foo(arg1, arg2, arg3);

    int value = foo(arg1, arg2,
                    arg3);

### Function Declaration and Definition
Use named parameters in function declaration.

    return_type function_name(int, char); // not allowed
    return_type function_name(int arg1, char arg2); // Use this

Return type should be on the same line as function name and parameters if it fits. If not, break between them aligned with the first argument.

    return_type function_name(int arg1,
                              char arg2);

If even first argument does not fit in a line, write it in a new line with 4 space indent.

    return_type function_name(
        int arg1, char arg2);


The open curly brace should be at the same line. The close curly brace should be either at the same line as its open curly brace or at new line.

    return_type function_name(int arg1, char arg2) { };
    return_type function_name(int arg1, char arg2) {
      ...
    }
    return_type function_name(int arg1, char arg2)
    {  // not allowed
      ...
    }

### Conditionals
Use a space between the if and open brace. Open brace on the same line as the if.

    if (condition) {
        ...
    }

Short conditional statements may be written without braces.

    if (condition)
      do_something();

### Loops and Switches
Use a space between the switch and loops(for, while, do-while) and open brace. Open brace on the same line as the switch and loops.

    while (condition) {
      ...
    }

Single loop body statement may be written without braces.

    while (condition)
      do_something(); // ok
    for (condition)
      do_something(); // ok


## Naming

### Type names
Use lower cases and underscore for struct names, and add prefix `iotjs_` and suffix `_t`.

    typedef struct {
      ...
    } iotjs_name_t;

### Function names
Use lower cases and underscore for function names.

For constructors, destructor, and methods of validated struct `iotjs_mystruct_t`, use names starting with `iotjs_mystruct_*`.
Constructor function name should be either `iotjs_mystruct_create` or `iotjs_mystruct_initialize`,
depending on whether the constructor returns the instance as return value, or the constructor just initializes the instance passed by parameter.

```c
typedef struct {
} IOTJS_VALIDATED_STRUCT(iotjs_mystruct_t);

iotjs_mystruct_t iotjs_mystruct_create(); // Ok
iotjs_mystruct_t* iotjs_mystruct_create(); // Ok
void iotjs_mystruct_initialize(iotjs_mystruct_t*); // Ok

void iotjs_mystruct_destroy();

int iotjs_mystruct_method();
```

### Variable names
Use lower cases and underscore for variable names.

    int lower_case_variable;


## Comments

### Comment style
Use either // or /* */ style comments. However, // style is much prefered.



# Coding Style Guideline for Javascript

This coding standard is based on [google javascript coding standard](https://google.github.io/styleguide/javascriptguide.xml)

## Javascript Language Rules

### var
Always declare variable before use.

### Semicolons
Always use semicolons.

### Function Declaration in blocks
Do not declare functions within a block.

### Wrapper objects of primitive types
Do not use wrapper objects for primitive types.

### with
Do not use `with` statement.

### Modifying prototypes of builtin objects
Do not modify prototypes of builtin objects

## Javascript Style Rules

### Naming
Use lowerCamelCase for varible names and function names.

    var myFirstVariable;
    function myFirstFunction {
      ...
    }

Use UpperCamelCase for constructor names

    function MyConstructorFunction(input) {
      this.variable = input;
      ...
    }

### Formatting
Follow C/C++ formatting above.


# Coding Style Guideline For Python

The coding conventions for Python code follows [PEP 8 - Style Guide for Python Code](https://www.python.org/dev/peps/pep-0008/)
