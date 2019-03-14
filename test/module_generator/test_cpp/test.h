/* Copyright 2019-present Samsung Electronics Co., Ltd. and other contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#define BIN 0b101
#define DEC 42
#define OCT 017
#define HEX 0xFF
#define one_l 1l
#define one_L 1L
#define one_u 1u
#define one_U 1U
#define SIGNED -42
#define FLOAT 1.5
#define SFLOAT -1.5
#define PI 314159E-5
#define CH 'a'
#define STRING "AaBb"
#define ONE 1
#define TWO ONE + 1
#define THREE (ONE) | (TWO)

extern char c;
extern int i;
typedef enum { A, B = 10 } e;
extern float f;
extern double d;
extern bool b;
extern char* c_ptr;
extern char c_arr[5];
extern int* i_ptr;
extern int i_arr[5];

typedef struct {
  int i;
  char c;
} S;

typedef struct { const int i; } const_S;

typedef union {
  int i;
  char c;
} U;

typedef union { const int i; } const_U;

extern S s;
extern const_S const_s;
extern U u;
extern const_U const_u;

typedef int(func)(void);
typedef int (*func_ptr)(void);

void f_void(void);
int f_int(int);
char f_char(char);
e f_enum(e);
float f_float(float);
double f_double(double);
bool f_bool(bool);
S f_struct(S);
U f_union(U);
char* f_char_ptr(char*);
char* f_char_arr(char[5]);
int* f_int_ptr(int*);
int* f_int_arr(int[5]);
int f_func(func);
int f_func_ptr(func_ptr);
void f_struct_ptr(S*);
void f_union_ptr(U*);

class O {
  int _i;

 public:
  O() : _i(42) {
  }
  int get_i() {
    return _i;
  }
  void set_i(int i) {
    _i = i;
  }
};

class Test {
  char _c;
  int _i;
  float _f;
  double _d;
  bool _b;
  char* _c_ptr;
  char _c_arr[5];
  int* _i_ptr;
  int _i_arr[5];
  S _s;
  U _u;
  O _o;

 public:
  char c;
  int i;
  float f;
  double d;
  bool b;
  char* c_ptr;
  char c_arr[5];
  int* i_ptr;
  int i_arr[5];
  S s;
  U u;
  O o;

  char get_c();
  void set_c(char);
  int get_i();
  void set_i(int);
  float get_f();
  void set_f(float);
  double get_d();
  void set_d(double);
  bool get_b();
  void set_b(bool);
  char* get_c_ptr();
  void set_c_ptr(char*, int);
  char* get_c_arr();
  void set_c_arr(char[5]);
  int* get_i_ptr();
  void set_i_ptr(int*, int);
  int* get_i_arr();
  void set_i_arr(int[5]);
  S get_s();
  void set_s(S);
  U get_u();
  void set_u(U);
  O get_o();
  void set_o(O);
};

namespace test_ns {
namespace nested_ns {
class A {
 public:
  int foo();
};
}
class A {
 public:
  int foo();
};
}
