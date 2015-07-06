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


#ifndef IOTJS_TYPES_H
#define IOTJS_TYPES_H


typedef uint8_t jschar;  /* follow jerry_api_char_t type which is uint8_t */

#define JSCT(a) reinterpret_cast<const jschar*>((const char*)a)


typedef uint8_t octet;  /* for Buffer byte element */

#define OCTET(a) reinterpret_cast<octet*>(a)


#endif /* IOTJS_TYPES_H */
