Validated struct is C struct wrapper for encapsulation and validity check.

* Validated Struct Declaration
* Constructors, Destructor, Methods
* Ownership of validated struct instance
  * Case 1: Validated struct instance as local variable
  * Case 2: Validated struct instance as parameter & return
  * Case 3: Validated struct instance as member variable of other struct
  * Case 4: Validated struct instance as data of asynchronous execution

# Validated Struct Declaration

```c
typedef struct {
  int a;
  void* b;
} IOTJS_VALIDATED_STRUCT(iotjs_myclass_t);
```

Above struct will make the member variable encapsulated by wrapping real members with wrapper like below.

```c
typedef struct {
  int a;
  void* b;
} iotjs_myclass_t_impl_t;

typedef struct {
  iotjs_myclass_impl_t unsafe;
  /* More members for struct validity check exist in debug mode */
} iotjs_myclass_t;

int main() {
  iotjs_myclass_t x;
}
```

Only wizards will access the members directly by using `x.unsafe.a`, `x.unafe.b`, ... . Otherwize the members are only accessible with its accessor function.

See `src/iotjs_def.h` for more details on real implementation.

# Constructors, Destructor, Methods

You should create C++-like constructors, destructor and methods with provided accessor. Then you can access the encapsulated member variables using `_this` variable, which has almost same role with C++ `this` keyword.
You must call `destroy` for every validated structs you've created.

```c
/* Constructor */
iotjs_myclass_t iotjs_myclass_create(int a) {
  iotjs_myclass_t instance;
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_myclass_t, &instance);

  _this->a = a;
  _this->b = malloc(a);

  return instance;
}

/* Destructor */
void iotjs_myclass_destroy(iotjs_myclass_t* instance) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_myclass_t, instance);
  free(_this->b);
}

/* Method */
int iotjs_myclass_get_a(iotjs_myclass_t* instance) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_myclass_t, instance);
  return _this->a;
}

int main() {
  /* Validated struct as local variable */
  iotjs_myclass_t local_instance = iotjs_myclass_create(3);
  printf("%d\n", iotjs_myclass_get_a(&local_instance));
  iotjs_myclass_destroy(&local_instance);
  return 0;
}
```

# Ownership of validated struct instance

The ground rule is:

* Use `iotjs_classname_t` typed variable if the variable *is* responsible for destruction of instance.
* Use `iotjs_classname_t*` typed variable if the variable *is not* responsible for destruction of instance.

Below Case 1 ~ Case 4 shows the case-by-case example of the ownership rule.

## Case 1: Validated struct instance as local variable
The `local_instance` variable in previous example was the local instance of validated struct.
Since `local_instance` should be destructed inside the function scope, `iotjs_myclass_t` type was used.

## Case 2: Validated struct instance as parameter & return
Previous example also included the example of validated struct instance as parameter and return.
When accessing member variable `a` by calling `iotjs_myclass_get_a()`,
`iotjs_myclass_t*` type was used as the parameter type, since it *does not* move the responsibility to destruct the instance.

And when returning the newly created instance by calling `iotjs_myclass_create()`,
`iotjs_myclass_t` type was used as return type, since it *does* move the responsibility to destruct the instance.

## Case 3: Validated struct instance as member variable of other struct

```c
/* Validated struct as member variable of other struct */

typedef struct {
  iotjs_myclass_t member_instance;
} IOTJS_VALIDATED_STRUCT(iotjs_otherclass_t)

iotjs_otherclass_t iotjs_otherclass_create() {
  /* Initialization steps for iotjs_otherclass_t */
  _this->member_instance = iotjs_myclass_create(3);
}

void iotjs_otherclass_destroy() {
  /* Finalization steps for iotjs_otherclass_t */
  iotjs_myclass_destroy(&_this->member_instance);
}
```

In the case above, `iotjs_myclass_t` instance is used as member variable of other class.
Since `iotjs_otherclass_t` is responsible for finalizing the `member_instance`,
it owns the variable as `iotjs_myclass_t` type, not pointer type.

## Case 4: Validated struct instance as data of asynchronous execution
Another usecase would be using validated struct as callback data.
Currently, our all asynchronous datas are wrapped with `iotjs_*wrap_t` type,
and they are destructed automatically.

```c
/*
 * Public APIs in iotjs_module_fs.h
 */

typedef struct {
  iotjs_reqwrap_t reqwrap;
  uv_fs_t req;
} IOTJS_VALIDATED_STRUCT(iotjs_fsreqwrap_t);

iotjs_fsreqwrap_t* iotjs_fsreqwrap_create(const iotjs_jval_t* jcallback);
void iotjs_fsreqwrap_dispatched(iotjs_fsreqwrap_t* fsreqwrap);
```

As you can see, constructor returns the `iotjs_fsreqwrap_t*` type,
because it does not pass the responsibility to destruct the return value.
It is destructed when request is dispatched, which can be informed by calling `iotjs_fsreqwrap_dispatched()`.
The destructor `iotjs_fsreqwrap_destroy()` is hidden in c file.

```c
/*
 * Implementation in iotjs_module_fs.c
 */

iotjs_fsreqwrap_t* iotjs_fsreqwrap_create(const iotjs_jval_t* jcallback) {
  iotjs_fsreqwrap_t* fsreqwrap = IOTJS_ALLOC(iotjs_fsreqwrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_fsreqwrap_t, fsreqwrap);
  iotjs_reqwrap_initialize(&_this->reqwrap, jcallback, (uv_req_t*)&_this->req);
  return fsreqwrap;
}

static void iotjs_fsreqwrap_destroy(iotjs_fsreqwrap_t* fsreqwrap) { // private function
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_fsreqwrap_t, fsreqwrap);
  uv_fs_req_cleanup(&_this->req);
  iotjs_reqwrap_destroy(&_this->reqwrap);
  IOTJS_RELEASE(fsreqwrap);
}

void iotjs_fsreqwrap_dispatched(iotjs_fsreqwrap_t* fsreqwrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_fsreqwrap_t, fsreqwrap);
  iotjs_fsreqwrap_destroy(fsreqwrap);
}

/*
 * Use of iotjs_fsreqwrap_t
 */

void callback(uv_fs_t* req) {
    do_something(req);
    iotjs_fsreqwrap_dispatched(req); /* Call iotjs_*reqwrap_dispatched() when callback called */
}

void request(iotjs_jval_t* jcallback) {
    iotjs_fsreqwrap_t* wrap = iotjs_fsreqwrap_create(jcallback);
    uv_fs_request(loop, wrap->req, callback);
}
```

In the case of tuv request wrapper, `iotjs_*reqwrap_dispatched()` should be called when the request has been dispatched.
In the case of tuv handle wrapper, `iotjs_handlewrap_close()` should be called when the handle has been closed.
in the case of JavaScript object wrapper, you don't have to do anything because JavaScript engine will call the destructor when the object becomes inaccessible.


