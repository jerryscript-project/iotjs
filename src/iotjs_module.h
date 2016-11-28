/* Copyright 2015-2016 Samsung Electronics Co., Ltd.
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

#ifndef IOTJS_MODULE_H
#define IOTJS_MODULE_H

#include "iotjs_binding.h"


typedef iotjs_jval_t (*register_func)();


#define CONCATENATE(x, ...) x##__VA_ARGS__

#define IF(c) CONCATENATE(IF_, c)
#define IF_1(expr) expr
#define IF_0(expr)

// Check if specific module is enabled
#define E(F, UPPER, Camel, lower) \
  IF(ENABLE_MODULE_##UPPER)(F(UPPER, Camel, lower))

// List of builtin modules
#define MAP_MODULE_LIST(F)                 \
  E(F, BLECENTRAL, Blecentral, blecentral) \
  E(F, BUFFER, Buffer, buffer)             \
  E(F, CONSOLE, Console, console)          \
  E(F, CONSTANTS, Constants, constants)    \
  E(F, DNS, Dns, dns)                      \
  E(F, FS, Fs, fs)                         \
  E(F, GPIO, Gpio, gpio)                   \
  E(F, HTTPPARSER, Httpparser, httpparser) \
  E(F, I2C, I2c, i2c)                      \
  E(F, PIN, Pin, pin)                      \
  E(F, PROCESS, Process, process)          \
  E(F, PWM, Pwm, pwm)                      \
  E(F, TESTDRIVER, Testdriver, testdriver) \
  E(F, TCP, Tcp, tcp)                      \
  E(F, TIMER, Timer, timer)                \
  E(F, UDP, Udp, udp)

#define ENUMDEF_MODULE_LIST(upper, Camel, lower) MODULE_##upper,

typedef enum {
  MAP_MODULE_LIST(ENUMDEF_MODULE_LIST) // enumerate modules
  MODULE_COUNT,
} ModuleKind;

#undef ENUMDEF_MODULE_LIST


void iotjs_module_list_init();

void iotjs_module_list_cleanup();

const iotjs_jval_t* iotjs_module_initialize_if_necessary(ModuleKind kind);
const iotjs_jval_t* iotjs_module_get(ModuleKind kind);


#endif /* IOTJS_MODULE_H */
