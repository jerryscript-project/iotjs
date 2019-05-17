/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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

#ifndef IOTJS_STRING_H
#define IOTJS_STRING_H

#include <stdbool.h>

typedef struct {
  size_t size;
  char* data;
} iotjs_string_t;

// Create new string
iotjs_string_t iotjs_string_create(void);
iotjs_string_t iotjs_string_create_with_size(const char* data, size_t size);
iotjs_string_t iotjs_string_create_with_buffer(char* buffer, size_t size);

// Destroy string
void iotjs_string_destroy(iotjs_string_t* str);

// Check if string is empty
bool iotjs_string_is_empty(const iotjs_string_t* str);

// Append `data` to tail of the string.
void iotjs_string_append(iotjs_string_t* str, const char* data, size_t size);

// Returns pointer to the bytes (never returns NULL)
const char* iotjs_string_data(const iotjs_string_t* str);

unsigned iotjs_string_size(const iotjs_string_t* str);


#endif /* IOTJS_STRING_H */
