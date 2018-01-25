/* Copyright 2016-present Samsung Electronics Co., Ltd. and other contributors
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

#ifndef IOTJS_SYSTEMIO_LINUX_H
#define IOTJS_SYSTEMIO_LINUX_H

#include "iotjs_def.h"

#define DEVICE_IO_PATH_BUFFER_SIZE 64
#define DEVICE_IO_PIN_BUFFER_SIZE 10


bool iotjs_systemio_check_path(const char* path);

bool iotjs_systemio_open_write_close(const char* path, const char* value);

bool iotjs_systemio_open_read_close(const char* path, char* buffer,
                                    unsigned buffer_len);

bool iotjs_systemio_device_open(const char* export_path, uint32_t value,
                                const char* exported_path,
                                const char** created_files,
                                int created_files_length);

bool iotjs_systemio_device_close(const char* export_path, uint32_t value);


#endif /* IOTJS_SYSTEMIO_LINUX_H */
