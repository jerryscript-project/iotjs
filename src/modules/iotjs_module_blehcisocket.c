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


iotjs_blehcisocket_t* iotjs_blehcisocket_create(jerry_value_t jble) {
  iotjs_blehcisocket_t* blehcisocket = IOTJS_ALLOC(iotjs_blehcisocket_t);

  blehcisocket->jobject = jble;
  jerry_set_object_native_pointer(jble, blehcisocket, &this_module_native_info);

  iotjs_blehcisocket_initialize(blehcisocket);

  return blehcisocket;
}


static void iotjs_blehcisocket_destroy(iotjs_blehcisocket_t* blehcisocket) {
  iotjs_blehcisocket_close(blehcisocket);
  IOTJS_RELEASE(blehcisocket);
}


JS_FUNCTION(Start) {
  JS_DECLARE_THIS_PTR(blehcisocket, blehcisocket);

  iotjs_blehcisocket_start(blehcisocket);

  return jerry_create_undefined();
}


JS_FUNCTION(BindRaw) {
  JS_DECLARE_THIS_PTR(blehcisocket, blehcisocket);
  JS_CHECK(jargc >= 1);

  int devId = 0;
  int* pDevId = NULL;

  if (jerry_value_is_number(jargv[0])) {
    devId = iotjs_jval_as_number(jargv[0]);
    pDevId = &devId;
  }

  int ret = iotjs_blehcisocket_bindRaw(blehcisocket, pDevId);

  return jerry_create_number(ret);
}


JS_FUNCTION(BindUser) {
  JS_DECLARE_THIS_PTR(blehcisocket, blehcisocket);
  DJS_CHECK_ARGS(1, number);

  int devId = JS_GET_ARG(0, number);
  int* pDevId = &devId;

  int ret = iotjs_blehcisocket_bindUser(blehcisocket, pDevId);

  return jerry_create_number(ret);
}


JS_FUNCTION(BindControl) {
  JS_DECLARE_THIS_PTR(blehcisocket, blehcisocket);

  iotjs_blehcisocket_bindControl(blehcisocket);

  return jerry_create_undefined();
}


JS_FUNCTION(IsDevUp) {
  JS_DECLARE_THIS_PTR(blehcisocket, blehcisocket);

  bool ret = iotjs_blehcisocket_isDevUp(blehcisocket);

  return jerry_create_boolean(ret);
}


JS_FUNCTION(SetFilter) {
  JS_DECLARE_THIS_PTR(blehcisocket, blehcisocket);
  DJS_CHECK_ARGS(1, object);

  iotjs_bufferwrap_t* buffer =
      iotjs_bufferwrap_from_jbuffer(JS_GET_ARG(0, object));

  iotjs_blehcisocket_setFilter(blehcisocket, buffer->buffer,
                               iotjs_bufferwrap_length(buffer));

  return jerry_create_undefined();
}


JS_FUNCTION(Stop) {
  JS_DECLARE_THIS_PTR(blehcisocket, blehcisocket);

  iotjs_blehcisocket_stop(blehcisocket);

  return jerry_create_undefined();
}


JS_FUNCTION(Write) {
  JS_DECLARE_THIS_PTR(blehcisocket, blehcisocket);
  DJS_CHECK_ARGS(1, object);

  iotjs_bufferwrap_t* buffer =
      iotjs_bufferwrap_from_jbuffer(JS_GET_ARG(0, object));

  iotjs_blehcisocket_write(blehcisocket, buffer->buffer,
                           iotjs_bufferwrap_length(buffer));

  return jerry_create_undefined();
}


JS_FUNCTION(BleHciSocketCons) {
  DJS_CHECK_THIS();

  // Create object
  jerry_value_t jblehcisocket = JS_GET_THIS();
  iotjs_blehcisocket_t* blehcisocket = iotjs_blehcisocket_create(jblehcisocket);
  IOTJS_ASSERT(blehcisocket ==
               (iotjs_blehcisocket_t*)(iotjs_jval_get_object_native_handle(
                   jblehcisocket)));
  return jerry_create_undefined();
}


jerry_value_t InitBlehcisocket() {
  jerry_value_t jblehcisocketCons =
      jerry_create_external_function(BleHciSocketCons);

  jerry_value_t prototype = jerry_create_object();

  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_START, Start);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_BINDRAW, BindRaw);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_BINDUSER, BindUser);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_BINDCONTROL, BindControl);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_ISDEVUP, IsDevUp);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_SETFILTER, SetFilter);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_STOP, Stop);
  iotjs_jval_set_method(prototype, IOTJS_MAGIC_STRING_WRITE, Write);

  iotjs_jval_set_property_jval(jblehcisocketCons, IOTJS_MAGIC_STRING_PROTOTYPE,
                               prototype);

  jerry_release_value(prototype);

  return jblehcisocketCons;
}
