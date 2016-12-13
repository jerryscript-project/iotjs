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


typedef struct {
  unsigned size;
  unsigned cap;
  char* data;
} IOTJS_VALIDATED_STRUCT(iotjs_string_t);

// Create new string
iotjs_string_t iotjs_string_create(const char* data);
iotjs_string_t iotjs_string_create_with_size(const char* data, unsigned size);
iotjs_string_t iotjs_string_create_with_buffer(char* buffer, unsigned size);

// Destroy string
void iotjs_string_destroy(iotjs_string_t* str);

// Reserve string capacity
void iotjs_string_reserve(iotjs_string_t* str, unsigned capacity);

// Check if string is empty
bool iotjs_string_is_empty(const iotjs_string_t* str);

// Make string empty
void iotjs_string_make_empty(iotjs_string_t* str);

// Append `data` to tail of the string.
void iotjs_string_append(iotjs_string_t* str, const char* data, int size);
static inline void iotjs_string_append_without_size(iotjs_string_t* str,
                                                    const char* data) {
  iotjs_string_append(str, data, -1);
}

// Returns pointer to the bytes (never returns NULL)
const char* iotjs_string_data(const iotjs_string_t* str);

unsigned iotjs_string_size(const iotjs_string_t* str);


#endif /* IOTJS_STRING_H */
