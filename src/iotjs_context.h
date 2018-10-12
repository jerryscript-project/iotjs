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

#include "iotjs_env.h"
#include "iotjs_external_context.h"

struct iotjs_context_t {
  // iotjs_environment_t struct
  iotjs_environment_t *current_env;

  // Initialized
  bool initialized;
};

#ifdef IOTJS_ENABLE_EXTERNAL_CONTEXT

#define IOTJS_GET_CURRENT_CONTEXT() (iotjs_port_get_current_context())

#define IOTJS_CONTEXT(field) (IOTJS_GET_CURRENT_CONTEXT()->field)

#else /* !IOTJS_ENABLE_EXTERNAL_CONTEXT */

extern iotjs_context_t iotjs_global_context;

#define IOTJS_CONTEXT(field) (iotjs_global_context.field)

#endif /* IOTJS_ENABLE_EXTERNAL_CONTEXT  */
