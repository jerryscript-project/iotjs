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

#ifndef IOTJS_MODULE_BUFFER_H
#define IOTJS_MODULE_BUFFER_H


#include "iotjs_objectwrap.h"


namespace iotjs {


iotjs_jval_t InitBuffer();


// Create buffer object.
iotjs_jval_t CreateBuffer(size_t len);


class BufferWrap : public JObjectWrap {
 public:
  BufferWrap(const iotjs_jval_t* jbuiltin, size_t length);

  virtual ~BufferWrap();

  static BufferWrap* FromJBufferBuiltin(const iotjs_jval_t* jbuiltin);
  static BufferWrap* FromJBuffer(const iotjs_jval_t* jbuffer);

  iotjs_jval_t* jbuiltin();
  iotjs_jval_t jbuffer();

  char* buffer();
  size_t length();

  int Compare(const BufferWrap& other) const;

  size_t Copy(const char* src, size_t len);
  size_t Copy(const char* src, size_t src_from, size_t src_to, size_t dst_from);

 protected:
  char* _buffer;
  size_t _length;
};



} // namespace iotjs


#endif /* IOTJS_MODULE_BUFFER_H */
