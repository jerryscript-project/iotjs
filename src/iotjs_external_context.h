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


#include "iotjs.h"
#include <stdint.h>
#include <stdio.h>

/**
 * Get the current context of the engine. Each port should provide its own
 * implementation of this interface.
 *
 * Note:
 *      This port function is called by jerry-core when
 *      IOTJS_ENABLE_EXTERNAL_CONTEXT is defined. Otherwise this function is not
 *      used.
 *
 * @return the pointer to the engine context.
 */

iotjs_context_t *iotjs_port_get_current_context(void);

void iotjs_port_default_set_context(iotjs_context_t *context_p);
