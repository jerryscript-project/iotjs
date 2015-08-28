/* Copyright 2015 Samsung Electronics Co., Ltd.
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

#ifndef IOTJS_UTIL_H
#define IOTJS_UTIL_H


#include <assert.h>


namespace iotjs {


class String;
class JObject;


String ReadFile(const char* path);

char* AllocBuffer(size_t size);
char* ReallocBuffer(char* buffer, size_t size);
void ReleaseBuffer(char* buff);


void PrintBacktrace();


class String {
 public:
  // Create emtpy string
  String();

  // Create string object from either ascii or utf8 encoded string data.
  // This constuctor will allocate new buffer to hold given string data.
  // If the second parameter `size` was given as `-1` the function calls
  // `strlen` to determin buffer size.
  // Third parameter determines initial capacity. If given as `-1` set the value
  // equal to `size`.
  explicit String(const char* data, int size = -1, int cap = -1);

  // Create string object from other string object.
  String(const String& other);

  // Destructor
  // Release allocated buffer.
  ~String();

  bool IsEmpty() const;

  // Make empty string
  void MakeEmpty();

  // Append `data` to tail of the String.
  void Append(const char* data, int size = -1);

  // Returns pointer to the bytes or NULL for empty string.
  char* data() const;

  int size() const;

 protected:
  // Buffer for containing the string data.
  char* _data;

  int _size;
  int _cap;

 private:
  // Prevent reassignments.
  String& operator=(const String& a) = delete;
};


} // namespace iotjs


#endif /* IOTJS_UTIL_H */
