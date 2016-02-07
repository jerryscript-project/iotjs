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

#include "iotjs_def.h"
#include "iotjs_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


namespace iotjs {


String ReadFile(const char* path) {
  FILE* file = fopen(path, "rb");
  IOTJS_ASSERT(file != NULL);

  fseek(file, 0, SEEK_END);
  long len = ftell(file);
  IOTJS_ASSERT(len >= 0);
  fseek(file, 0, SEEK_SET);

  String contents(NULL, 0, len);

  char buff[128];
  size_t total = 0;

  while (total < len) {
    size_t read = fread(buff, 1, 128, file);
    IOTJS_ASSERT(read > 0);

    contents.Append(buff, read);
    total += read;
  }

  fclose(file);

  return contents;
}


char* AllocBuffer(size_t size) {
  char* buff = reinterpret_cast<char*>(malloc(size));
  memset(buff, 0, size);
  return buff;
}


char* ReallocBuffer(char* buffer, size_t size) {
  return reinterpret_cast<char*>(realloc(buffer, size));
}


void ReleaseBuffer(char* buffer) {
  free(buffer);
}


String::String() {
  _size = _cap = 0;
  _data = NULL;
}


String::String(const char* data, int size, int cap) {
  if (size < 0) {
    if (data != NULL) {
      _size = strlen(data);
    } else {
      _size = 0;
    }
  } else {
    _size = size;
  }

  if (cap < 0) {
    _cap = _size;
  } else {
    _cap = cap;
  }

  if (_cap < _size) {
    _cap = _size;
  }

  IOTJS_ASSERT(_size >= 0);
  IOTJS_ASSERT(_cap >= 0);

  if (_cap > 0) {
    _data = AllocBuffer(_cap + 1);
  } else {
    _data = NULL;
  }

  if (_data != NULL && data != NULL) {
    strncpy(_data, data, _size);
  }
}


String::~String() {
  IOTJS_ASSERT(_cap == 0 || _data != NULL);

  if (_data != NULL) {
    ReleaseBuffer(_data);
  }
}


bool String::IsEmpty() const {
  IOTJS_ASSERT(_size >= 0);
  return _size == 0;
}


void String::MakeEmpty() {
  if (_data != NULL) {
    ReleaseBuffer(_data);
    _cap = _size = 0;
    _data = NULL;
  }
}


void String::Append(const char* data, int size) {
  const int kCapacityIncreaseFactor = 2;

  IOTJS_ASSERT(_cap == 0 || _data != NULL);
  IOTJS_ASSERT(_cap >= _size);

  if (data == NULL) {
    return;
  }
  if (size < 0) {
    size = strlen(data);
  }
  if (size == 0) {
    return;
  }

  if (_cap == 0) {
    // No buffer was allocated.
    IOTJS_ASSERT(_size == 0);
    IOTJS_ASSERT(_data == NULL);
    _cap = _size = size;
    _data = AllocBuffer(_cap + 1);
    strncpy(_data, data, _size);
  } else if (_cap >= _size + size) {
    // Have enough capacity to append data.
    strncpy(_data + _size, data, size);
    _size += size;
  } else {
    // Lack of capacity, calculate next capacity.
    while (_cap < _size + size) {
      _cap *= kCapacityIncreaseFactor;
    }
    // Reallocate buffer and copy data.
    _data = ReallocBuffer(_data, _cap + 1);
    strncpy(_data + _size, data, size);
    _size += size;
  }
  _data[_size] = 0;
}


char* String::data() const {
  IOTJS_ASSERT(_size == 0 || _data != NULL);
  return _data;
}


int String::size() const {
  IOTJS_ASSERT(_size >= 0);
  return _size;
}


#define ONEBYTECESU(var)   (!((((uint8_t)var)>>7) & 1))     // 0xxxxxxx
#define TWOBYTECESU(var)   ((((uint8_t)var)>>5) == 6)       // 110xxxxx
#define THREEBYTECESU(var) ((((uint8_t)var)>>5) == 7)       // 1110xxxx


int String::UTF16be(uint16_t* out) {
  // Convert _data(cesu8 encoded) to utf16 format
  int out_bytes = 0;
  int processed_bytes = 0;
  uint8_t* src = _data;
  while (processed_bytes < _size-1) {
    if(ONEBYTECESU(*src)) {
      if (out != NULL) *out = (uint16_t)(*src);
      // 1 cesu byte to 1 utf16 code unit(2bytes)
      src++;
      processed_bytes++;
    }
    else if(TWOBYTECESU(*src)) {
      uint16_t high = ((*src) & 0x1f);  // mask out first 3 bits
      src++;
      processed_bytes++;
      uint16_t low = ((*src) & 0x3f);   // mask out first 2 bits
      src++;
      processed_bytes++;
      if (out != NULL) *out = ((high << 6) | low);
    }
    else if(THREEBYTECESU(*src)) {
      uint16_t high1 = ((*src) & 0x0f);  // mask out first 4 bits
      src++;
      processed_bytes++;
      uint16_t high2 = ((*src) & 0x3f); // mask out first 2 bits
      src++;
      processed_bytes++;
      uint16_t low = ((*src) & 0x3f);  // mask out first 2 bits
      src++;
      processed_bytes++;
      if (out != NULL) *out = ((high1 << 12) | (high2 << 6)
                               | low);
    }
    out_bytes += 2;
    if(out != NULL) out++;
  }

  return out_bytes;
}

#define ONEBYTEUTF16(var)   ((var) <= 0x7f)     // 000000000xxxxxxx
#define TWOBYTEUTF16(var)   ((var) <= 0x7ff)    // 00000xxxxxxxxxxx
#define FOURBYTEUTF16(var)  ((var >> 10) == 54) // 110110yy yyxxxxxx
                                                // 110111xx xxxxxxxx



// in_len is not # of bytes to be processed, # of utf16 code unit(16bits)
int UTF16toUTF8(uint16_t* in, size_t in_len, uint8_t* out) {
  int processed_codeunit = 0;
  int out_bytes = 0;
  while(processed_codeunit < in_len) {
    if (ONEBYTEUTF16(*in)) {
      if (out != NULL) {
        (*out) = (uint8_t)*in;
        out++;
      }
      processed_codeunit++;
      in++;
      out_bytes++;
    }
    else if (TWOBYTEUTF16(*in)) {
      uint16_t high = (*in >> 6) | (6 << 5);
      uint16_t low = ((*in & 0x3f ) | (2 << 6));

      if (out != NULL) {
        *out = (uint8_t)high;
        out++;
      }
      if (out != NULL) {
        *out = (uint8_t)low;
        out++;
      }

      in++;
      processed_codeunit++;
      out_bytes += 2;
    }
    else if (FOURBYTEUTF16(*in)) {
      uint16_t yyyy = (*in << 6) >> 12;
      uint16_t zzzzz = yyyy + 1;
      uint16_t xxxxxx = (*in & 0x3f);

      if (out != NULL) {
        *out = (uint8_t)((zzzzz >> 2) | (30 << 3));
        out++;
      }
      if (out != NULL) {
        *out = (uint8_t)((2 << 6) | ((zzzzz >> 3) << 4) | (xxxxxx >> 2));
        out++;
      }
      in++;

      uint16_t xxxxxxxxxx = (*in << 6) >> 6;
      if (out != NULL) {
        *out = (uint8_t)((2 << 6) | ((xxxxxx >> 4) << 4) | (xxxxxxxxxx >> 6) );
        out++;
      }
      if (out != NULL) {
        *out = (uint8_t)((2 << 6) | ((xxxxxxxxxx & 0x3f)));
        out++;
      }
      in++;

      processed_codeunit += 2;
      out_bytes += 4;

    }
    else { // three bytes for utf8
      if (out != NULL) {
        *out = (uint8_t)((*in >> 12) | (14 << 4));
        out++;
      }
      if (out != NULL) {
        *out = (uint8_t)((*in >> 10) | (2 << 6));
        out++;
      }
      if (out != NULL) {
        *out = (uint8_t)((*in & 0x3f) | (2 << 6));
        out++;
      }

      in++;
      processed_codeunit++;
      out_bytes += 3;
    }
  }
  return out_bytes;
}


int String::UTF8(uint8_t* out) {
  // Convert _data(cesu8 encoded) to utf8 format
  // 1. cesu8 to utf16
  int utf16len = UTF16be(NULL);
  uint16_t *buf_utf16 = (uint16_t*)AllocBuffer(utf16len);
  UTF16be(buf_utf16);

  // 2. utf16be to utf8
  int utf8len = UTF16toUTF8(buf_utf16, utf16len/2, NULL);
  UTF16toUTF8(buf_utf16, utf16len/2, out);

  ReleaseBuffer((char*)buf_utf16);
  return utf8len;
}

} // namespace iotjs
