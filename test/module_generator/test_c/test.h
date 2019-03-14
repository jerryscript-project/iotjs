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

#ifndef TEST_H
#define TEST_H

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

char c;
int i;
typedef enum { A, B = 10 } e;
float f;
double d;
_Bool b;
char* c_ptr;
char c_arr[5];
int* i_ptr;
int i_arr[5];

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

S s;
const_S const_s;
U u;
const_U const_u;

typedef int(func)(void);
typedef int (*func_ptr)(void);

void f_void(void);
int f_int(int);
char f_char(char);
e f_enum(e);
float f_float(float);
double f_double(double);
_Bool f_bool(_Bool);
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

#endif
