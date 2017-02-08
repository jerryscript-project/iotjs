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


#include "iotjs_def.h"
#include "iotjs_module_blehcisocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define THIS iotjs_blehcisocket_t* blehcisocket


static void iotjs_blehcisocket_destroy(THIS);


iotjs_blehcisocket_t* iotjs_blehcisocket_create(const iotjs_jval_t* jble) {
  THIS = IOTJS_ALLOC(iotjs_blehcisocket_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_blehcisocket_t, blehcisocket);

  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, jble,
                               (JFreeHandlerType)iotjs_blehcisocket_destroy);

  // initialize

  iotjs_blehcisocket_initialize(blehcisocket);

  return blehcisocket;
}


iotjs_blehcisocket_t* iotjs_blehcisocket_instance_from_jval(
    const iotjs_jval_t* jble) {
  iotjs_jobjectwrap_t* jobjectwrap = iotjs_jobjectwrap_from_jobject(jble);
  return (iotjs_blehcisocket_t*)jobjectwrap;
}


static void iotjs_blehcisocket_destroy(THIS) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_blehcisocket_t, blehcisocket);
  iotjs_jobjectwrap_destroy(&_this->jobjectwrap);
  IOTJS_RELEASE(blehcisocket);
}


JHANDLER_FUNCTION(Start) {
  JHANDLER_CHECK_THIS(object);
  // JHANDLER_CHECK_ARGS(1, number);

  const iotjs_jval_t* jblehcisocket = JHANDLER_GET_THIS(object);

  iotjs_blehcisocket_t* blehcisocket =
      iotjs_blehcisocket_instance_from_jval(jblehcisocket);

  iotjs_blehcisocket_start(blehcisocket);

  // iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(BindRaw) {
  JHANDLER_CHECK_THIS(object);
  // JHANDLER_CHECK_ARGS(1, number);

  const iotjs_jval_t* jblehcisocket = JHANDLER_GET_THIS(object);

  iotjs_blehcisocket_t* blehcisocket =
      iotjs_blehcisocket_instance_from_jval(jblehcisocket);

  iotjs_blehcisocket_bindRaw(blehcisocket, 0);

  // iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(BindUser) {
  JHANDLER_CHECK_THIS(object);
  // JHANDLER_CHECK_ARGS(1, number);

  const iotjs_jval_t* jblehcisocket = JHANDLER_GET_THIS(object);

  iotjs_blehcisocket_t* blehcisocket =
      iotjs_blehcisocket_instance_from_jval(jblehcisocket);

  iotjs_blehcisocket_bindUser(blehcisocket, 0);

  // iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(BindControl) {
  JHANDLER_CHECK_THIS(object);
  // JHANDLER_CHECK_ARGS(1, number);

  const iotjs_jval_t* jblehcisocket = JHANDLER_GET_THIS(object);

  iotjs_blehcisocket_t* blehcisocket =
      iotjs_blehcisocket_instance_from_jval(jblehcisocket);

  iotjs_blehcisocket_bindControl(blehcisocket);

  // iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(IsDevUp) {
  JHANDLER_CHECK_THIS(object);
  // JHANDLER_CHECK_ARGS(1, number);

  const iotjs_jval_t* jblehcisocket = JHANDLER_GET_THIS(object);

  iotjs_blehcisocket_t* blehcisocket =
      iotjs_blehcisocket_instance_from_jval(jblehcisocket);

  iotjs_blehcisocket_isDevUp(blehcisocket);

  // iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(SetFilter) {
  JHANDLER_CHECK_THIS(object);
  // JHANDLER_CHECK_ARGS(1, number);

  const iotjs_jval_t* jblehcisocket = JHANDLER_GET_THIS(object);

  iotjs_blehcisocket_t* blehcisocket =
      iotjs_blehcisocket_instance_from_jval(jblehcisocket);

  iotjs_blehcisocket_setFilter(blehcisocket, 0, 0);

  // iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Stop) {
  JHANDLER_CHECK_THIS(object);
  // JHANDLER_CHECK_ARGS(1, number);

  const iotjs_jval_t* jblehcisocket = JHANDLER_GET_THIS(object);

  iotjs_blehcisocket_t* blehcisocket =
      iotjs_blehcisocket_instance_from_jval(jblehcisocket);

  iotjs_blehcisocket_stop(blehcisocket);

  // iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Write) {
  JHANDLER_CHECK_THIS(object);
  // JHANDLER_CHECK_ARGS(1, number);

  const iotjs_jval_t* jblehcisocket = JHANDLER_GET_THIS(object);

  iotjs_blehcisocket_t* blehcisocket =
      iotjs_blehcisocket_instance_from_jval(jblehcisocket);

  iotjs_blehcisocket_write(blehcisocket, 0, 0);

  // iotjs_jhandler_return_null(jhandler);
}

JHANDLER_FUNCTION(BleHciSocketCons) {
  JHANDLER_CHECK_THIS(object);
  // JHANDLER_CHECK_ARGS(1, number);

  // Create object
  const iotjs_jval_t* jblehcisocket = JHANDLER_GET_THIS(object);
  iotjs_blehcisocket_t* blehcisocket = iotjs_blehcisocket_create(jblehcisocket);
  IOTJS_ASSERT(blehcisocket ==
               (iotjs_blehcisocket_t*)(iotjs_jval_get_object_native_handle(
                   jblehcisocket)));
}


iotjs_jval_t InitBlehcisocket() {
  iotjs_jval_t jblehcisocketCons = iotjs_jval_create_function(BleHciSocketCons);

  iotjs_jval_t prototype = iotjs_jval_create_object();

  iotjs_jval_set_method(&prototype, "start", Start);
  iotjs_jval_set_method(&prototype, "bindRaw", BindRaw);
  iotjs_jval_set_method(&prototype, "bindUser", BindUser);
  iotjs_jval_set_method(&prototype, "bindControl", BindControl);
  iotjs_jval_set_method(&prototype, "isDevUp", IsDevUp);
  iotjs_jval_set_method(&prototype, "setFilter", SetFilter);
  iotjs_jval_set_method(&prototype, "stop", Stop);
  iotjs_jval_set_method(&prototype, "write", Write);

  iotjs_jval_set_property_jval(&jblehcisocketCons, "prototype", &prototype);

  iotjs_jval_destroy(&prototype);

  return jblehcisocketCons;
}
