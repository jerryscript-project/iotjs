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

void f_void(void) {
  return;
}

int f_int(int a) {
  return a;
}

char f_char(char a) {
  return a;
}

e f_enum(e a) {
  return a;
}

float f_float(float a) {
  return a;
}

double f_double(double a) {
  return a;
}

_Bool f_bool(_Bool a) {
  return a;
}

S f_struct(S a) {
  return a;
}

U f_union(U a) {
  return a;
}

char* f_char_ptr(char* a) {
  return a;
}

char* f_char_arr(char a[5]) {
  return a;
}

int* f_int_ptr(int* a) {
  return a;
}

int* f_int_arr(int a[5]) {
  return a;
}

int f_func(func f) {
  if (f) {
    return f();
  }
  return 0;
}

int f_func_ptr(func_ptr f) {
  if (f) {
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
