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

#include "iotjs_external_context.h"

/**
 * Pointer to the current context.
 * Note that it is a global variable, and is not a thread safe implementation.
 */
static iotjs_context_t *current_context_p = NULL;

/**
 * Set the current_context_p as the passed pointer.
 */
void iotjs_port_default_set_context(iotjs_context_t *context_p) {
  current_context_p = context_p;
} /* iotjs_port_default_set_context */

__attribute__((weak)) iotjs_context_t *iotjs_port_get_current_context(void) {
  return current_context_p;
} /* iotjs_port_get_current_context */
