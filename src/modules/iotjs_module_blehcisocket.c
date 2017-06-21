/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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

/* Copyright (C) 2015 Sandeep Mistry sandeep.mistry@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "iotjs_def.h"
#include "iotjs_module_blehcisocket.h"
#include "iotjs_module_buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define THIS iotjs_blehcisocket_t* blehcisocket


IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(blehcisocket);


iotjs_blehcisocket_t* iotjs_blehcisocket_create(const iotjs_jval_t* jble) {
  THIS = IOTJS_ALLOC(iotjs_blehcisocket_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_blehcisocket_t, blehcisocket);

  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, jble,
                               &this_module_native_info);

  iotjs_blehcisocket_initialize(blehcisocket);

  return blehcisocket;
}


iotjs_blehcisocket_t* iotjs_blehcisocket_instance_from_jval(
    const iotjs_jval_t* jble) {
  iotjs_jobjectwrap_t* jobjectwrap = iotjs_jobjectwrap_from_jobject(jble);
  return (iotjs_blehcisocket_t*)jobjectwrap;
}


static void iotjs_blehcisocket_destroy(THIS) {
  iotjs_blehcisocket_close(blehcisocket);

  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_blehcisocket_t, blehcisocket);
  iotjs_jobjectwrap_destroy(&_this->jobjectwrap);
  IOTJS_RELEASE(blehcisocket);
}


JHANDLER_FUNCTION(Start) {
  JHANDLER_DECLARE_THIS_PTR(blehcisocket, blehcisocket);
  DJHANDLER_CHECK_ARGS(0);

  iotjs_blehcisocket_start(blehcisocket);

  iotjs_jhandler_return_undefined(jhandler);
}


JHANDLER_FUNCTION(BindRaw) {
  JHANDLER_DECLARE_THIS_PTR(blehcisocket, blehcisocket);
  JHANDLER_CHECK(ge(iotjs_jhandler_get_arg_length(jhandler), 1));

  int devId = 0;
  int* pDevId = NULL;

  const iotjs_jval_t* raw = iotjs_jhandler_get_arg(jhandler, 0);
  if (iotjs_jval_is_number(raw)) {
    devId = iotjs_jval_as_number(raw);
    pDevId = &devId;
  }

  int ret = iotjs_blehcisocket_bindRaw(blehcisocket, pDevId);

  iotjs_jhandler_return_number(jhandler, ret);
}


JHANDLER_FUNCTION(BindUser) {
  JHANDLER_DECLARE_THIS_PTR(blehcisocket, blehcisocket);
  DJHANDLER_CHECK_ARGS(1, number);

  int devId = JHANDLER_GET_ARG(0, number);
  int* pDevId = &devId;

  int ret = iotjs_blehcisocket_bindUser(blehcisocket, pDevId);

  iotjs_jhandler_return_number(jhandler, ret);
}


JHANDLER_FUNCTION(BindControl) {
  JHANDLER_DECLARE_THIS_PTR(blehcisocket, blehcisocket);
  DJHANDLER_CHECK_ARGS(0);

  iotjs_blehcisocket_bindControl(blehcisocket);

  iotjs_jhandler_return_undefined(jhandler);
}


JHANDLER_FUNCTION(IsDevUp) {
  JHANDLER_DECLARE_THIS_PTR(blehcisocket, blehcisocket);
  DJHANDLER_CHECK_ARGS(0);

  bool ret = iotjs_blehcisocket_isDevUp(blehcisocket);

  iotjs_jhandler_return_boolean(jhandler, ret);
}


JHANDLER_FUNCTION(SetFilter) {
  JHANDLER_DECLARE_THIS_PTR(blehcisocket, blehcisocket);
  DJHANDLER_CHECK_ARGS(1, object);

  iotjs_bufferwrap_t* buffer =
      iotjs_bufferwrap_from_jbuffer(JHANDLER_GET_ARG(0, object));

  iotjs_blehcisocket_setFilter(blehcisocket, iotjs_bufferwrap_buffer(buffer),
                               iotjs_bufferwrap_length(buffer));

  iotjs_jhandler_return_undefined(jhandler);
}


JHANDLER_FUNCTION(Stop) {
  JHANDLER_DECLARE_THIS_PTR(blehcisocket, blehcisocket);
  DJHANDLER_CHECK_ARGS(0);

  iotjs_blehcisocket_stop(blehcisocket);

  iotjs_jhandler_return_undefined(jhandler);
}


JHANDLER_FUNCTION(Write) {
  JHANDLER_DECLARE_THIS_PTR(blehcisocket, blehcisocket);
  DJHANDLER_CHECK_ARGS(1, object);

  iotjs_bufferwrap_t* buffer =
      iotjs_bufferwrap_from_jbuffer(JHANDLER_GET_ARG(0, object));

  iotjs_blehcisocket_write(blehcisocket, iotjs_bufferwrap_buffer(buffer),
                           iotjs_bufferwrap_length(buffer));

  iotjs_jhandler_return_undefined(jhandler);
}


JHANDLER_FUNCTION(BleHciSocketCons) {
  DJHANDLER_CHECK_THIS(object);
  DJHANDLER_CHECK_ARGS(0);

  // Create object
  const iotjs_jval_t* jblehcisocket = JHANDLER_GET_THIS(object);
  iotjs_blehcisocket_t* blehcisocket = iotjs_blehcisocket_create(jblehcisocket);
  IOTJS_ASSERT(blehcisocket ==
               (iotjs_blehcisocket_t*)(iotjs_jval_get_object_native_handle(
                   jblehcisocket)));
}


iotjs_jval_t InitBlehcisocket() {
  iotjs_jval_t jblehcisocketCons =
      iotjs_jval_create_function_with_dispatch(BleHciSocketCons);

  iotjs_jval_t prototype = iotjs_jval_create_object();

  iotjs_jval_set_method(&prototype, IOTJS_MAGIC_STRING_START, Start);
  iotjs_jval_set_method(&prototype, IOTJS_MAGIC_STRING_BINDRAW, BindRaw);
  iotjs_jval_set_method(&prototype, IOTJS_MAGIC_STRING_BINDUSER, BindUser);
  iotjs_jval_set_method(&prototype, IOTJS_MAGIC_STRING_BINDCONTROL,
                        BindControl);
  iotjs_jval_set_method(&prototype, IOTJS_MAGIC_STRING_ISDEVUP, IsDevUp);
  iotjs_jval_set_method(&prototype, IOTJS_MAGIC_STRING_SETFILTER, SetFilter);
  iotjs_jval_set_method(&prototype, IOTJS_MAGIC_STRING_STOP, Stop);
  iotjs_jval_set_method(&prototype, IOTJS_MAGIC_STRING_WRITE, Write);

  iotjs_jval_set_property_jval(&jblehcisocketCons, IOTJS_MAGIC_STRING_PROTOTYPE,
                               &prototype);

  iotjs_jval_destroy(&prototype);

  return jblehcisocketCons;
}
