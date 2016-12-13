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

#ifndef IOTJS_DEVICE_IO_LINUX_GENERAL_H
#define IOTJS_DEVICE_IO_LINUX_GENERAL_H


#define DEVICE_IO_PATH_BUFFER_SIZE 64
#define DEVICE_IO_PIN_BUFFER_SIZE 10


bool DeviceCheckPath(const char* path);

bool DeviceOpenWriteClose(const char* path, char* value);

bool DeviceOpenReadClose(const char* path, char* buffer, int buffer_len);

bool DeviceExport(const char* export_path, int value, const char* exported_path,
                  const char** created_files, int created_files_length);

bool DeviceUnexport(const char* export_path, int value);


#endif /* IOTJS_DEVICE_IO_LINUX_GENERAL_H */
