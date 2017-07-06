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
#include "iotjs_objectwrap.h"
#include "iotjs_reqwrap.h"
#include <ocstack.h>
#include <stdlib.h>

/* Checks if jval is not null or undefined on JS-side.
 *
 * @param jval JS-side object.
 *
 * @returns true if jval is not null or undefined, false otherwise.
 */
bool iotjs_exist_on_js_side(iotjs_jval_t* jval) {
  return !(iotjs_jval_is_undefined(jval) || iotjs_jval_is_null(jval)) ? true
                                                                      : false;
}

/* Gets current JS array size.
 *
 * @param array JS array.
 * @param maxSize Max possible array size.
 *
 * @returns JS array size.
 */
int get_jarray_size(const iotjs_jval_t* jarray) {
  int length;
  iotjs_jval_t jval;
  for (length = 0;; length++) {
    jval = iotjs_jval_get_property_by_index(jarray, (uint32_t)length);
    if (!iotjs_exist_on_js_side(&jval)) {
      iotjs_jval_destroy(&jval);
      break;
    }
    iotjs_jval_destroy(&jval);
  }
  return length;
}

/* Makes pure C-string from iotjs string type.
 *
 * @param str Iotjs string type.
 *
 * @returns Pure C-string.
 */
char* get_string_from_iotjs(iotjs_string_t* str) {
  const IOTJS_VALIDATED_STRUCT_METHOD(iotjs_string_t, str);
  if (_this->data == NULL) {
    return "";
  }
  char* cstr = strdup(_this->data);
  return cstr;
}

/* Sets string to object field if jval is not null or undefined.
 * Some structures from IoTivity C API could have empty string fields.
 *
 * @param jval JS-side value to convert.
 *
 * @returns C string.
 */
char* get_string_from_jval(iotjs_jval_t jval) {
  if (iotjs_exist_on_js_side(&jval)) {
    if (iotjs_jval_is_string(&jval)) {
      iotjs_string_t str_jval = iotjs_jval_as_string(&jval);
      char* str = get_string_from_iotjs(&str_jval);
      iotjs_string_destroy(&str_jval);
      return str;
    }
  }
  return NULL;
}

void free_linked_list(OCStringLL* list) {
  OCStringLL *item, *next_item;
  for (item = list; item;) {
    next_item = item->next;
    free(item->value);
    free(item);
    item = next_item;
  }
}

/* Sets OCStringLL struct with values from JS-side.
 *
 * @param destination OCStringLL C structure.
 * @param jarray Array from JS-side.
 */
void c_OCStringLL(OCStringLL** destination, iotjs_jval_t* jarray) {
  OCStringLL *first = NULL, **next = &first;

  if (!iotjs_exist_on_js_side(jarray)) {
    return;
  }

  iotjs_jval_t js_value;
  int i, length = get_jarray_size(jarray);

  for (i = 0; i < length; ++i) {
    js_value = iotjs_jval_get_property_by_index(jarray, (uint32_t)i);
    if (!iotjs_jval_is_string(&js_value)) {
      break;
    }
    (*next) = IOTJS_ALLOC(OCStringLL);
    (*next)->next = NULL;
    (*next)->value = get_string_from_jval(js_value);

    next = &((*next)->next);
  }
  if (length > 0) {
    iotjs_jval_destroy(&js_value);
  }

  *destination = first;
}

/* Sets JS-side OCStringLL-type array with values from C struct.
 *
 * @param jarray JS-side array.
 * @param str OCStringLL C struct.
 */
void js_OCStringLL(const iotjs_jval_t* jarray, OCStringLL* str) {
  uint32_t index;

  for (index = 0; str != NULL; index++) {
    iotjs_string_t str_value = iotjs_string_create();
    iotjs_string_append(&str_value, str->value, strlen(str->value));
    iotjs_jval_t value = iotjs_jval_create_string(&str_value);

    iotjs_jval_set_property_by_index(jarray, index, &value);

    iotjs_jval_destroy(&value);
    iotjs_string_destroy(&str_value);

    str = str->next;
  }
}

/* Sets JS-side OCIdentity-type array with values from C struct.
 *
 * @param jarray JS-side object.
 * @param identity OCIdentity C struct.
 */
void js_OCIdentity(const iotjs_jval_t* jarray, OCIdentity* identity) {
  uint32_t index;

  for (index = 0; index < identity->id_length; index++) {
    iotjs_jval_t value = iotjs_jval_create_number(identity->id[index]);
    iotjs_jval_set_property_by_index(jarray, index, &value);
    iotjs_jval_destroy(&value);
  }
}
