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

#include "test.h"

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
const_S const_s = {0};
U u;
const_U const_u = {0};

void f_void (void)
{
  return;
}

int f_int (int a)
{
  return a;
}

char f_char (char a)
{
  return a;
}

e f_enum (e a)
{
  return a;
}

float f_float (float a)
{
  return a;
}

double f_double (double a)
{
  return a;
}

bool f_bool (bool a)
{
  return a;
}

S f_struct (S a)
{
  return a;
}

U f_union (U a)
{
  return a;
}

char* f_char_ptr (char* a)
{
  return a;
}

char* f_char_arr (char a[5])
{
  return a;
}

int* f_int_ptr (int* a)
{
  return a;
}

int* f_int_arr (int a[5])
{
  return a;
}

int f_func (func f)
{
  if (f)
  {
    return f();
  }
  return 0;
}

int f_func_ptr (func_ptr f)
{
  if (f)
  {
    return f();
  }
  return 0;
}

void f_struct_ptr(S* s) {
  s->c = 's';
  s->i = 42;
}

void f_union_ptr(U* u) {
  u->i = 65;
}

char Test::get_c()
{
  return _c;
}

void Test::set_c(char c)
{
  _c = c;
}

int Test::get_i()
{
  return _i;
}

void Test::set_i(int i)
{
  _i = i;
}

float Test::get_f()
{
  return _f;
}

void Test::set_f(float f)
{
  _f = f;
}

double Test::get_d()
{
  return _d;
}

void Test::set_d(double d)
{
  _d = d;
}

bool Test::get_b()
{
  return _b;
}

void Test::set_b(bool b)
{
  _b = b;
}

char* Test::get_c_ptr()
{
  return _c_ptr;
}

void Test::set_c_ptr(char* c_ptr, int size)
{
  if (_c_ptr)
  {
    delete _c_ptr;
  }
  _c_ptr = new char[sizeof(char) * size + 1];
  for (int i = 0; i < size; i++)
  {
    _c_ptr[i] = c_ptr[i];
  }
  _c_ptr[size] = '\0';
}

char* Test::get_c_arr()
{
  return _c_arr;
}

void Test::set_c_arr(char c_arr[5])
{
  for (int i = 0; i < 4; i++)
  {
    _c_arr[i] = c_arr[i];
  }
}

int* Test::get_i_ptr()
{
  return _i_ptr;
}

void Test::set_i_ptr(int* i_ptr, int size)
{
  if (_i_ptr)
  {
    delete _i_ptr;
  }
  _i_ptr = new int[sizeof(int) * size];
  for (int i = 0; i < size; i++)
  {
    _i_ptr[i] = i_ptr[i];
  }
}

int* Test::get_i_arr()
{
  return _i_arr;
}

void Test::set_i_arr(int i_arr[5])
{
  for (int i = 0; i < 5; i++)
  {
    _i_arr[i] = i_arr[i];
  }
}

S Test::get_s()
{
  return _s;
}

void Test::set_s(S s)
{
  _s.i = s.i;
  _s.c = s.c;
}

U Test::get_u()
{
  return _u;
}

void Test::set_u(U u)
{
  _u.i = u.i;
  _u.c = u.c;
}

O Test::get_o()
{
  return _o;
}

void Test::set_o(O o)
{
  _o = o;
}

int test_ns::A::foo()
{
  return 1;
}

int test_ns::nested_ns::A::foo()
{
  return 2;
}
