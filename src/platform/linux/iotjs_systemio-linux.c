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


#include <uv.h>
#include <unistd.h>

#include "iotjs_systemio-linux.h"


// Checks if given directory exists.
bool iotjs_systemio_check_path(const char* path) {
  const iotjs_environment_t* env = iotjs_environment_get();

  DDDLOG("%s - path: %s", __func__, path);

  // stat for the path.
  uv_fs_t fs_req;
  int err = uv_fs_stat(iotjs_environment_loop(env), &fs_req, path, NULL);
  uv_fs_req_cleanup(&fs_req);

  // exist?
  if (err || fs_req.result) {
    return false;
  }

  DDDLOG("%s - path exist", __func__);

  return true;
}


bool iotjs_systemio_open_write_close(const char* path, const char* value) {
  const iotjs_environment_t* env = iotjs_environment_get();
  uv_loop_t* loop = iotjs_environment_loop(env);

  DDDLOG("%s - path %s, value: %s", __func__, path, value);

  // Open file
  uv_fs_t fs_req;
  int fd = uv_fs_open(loop, &fs_req, path, O_WRONLY, 0666, NULL);
  uv_fs_req_cleanup(&fs_req);
  if (fd < 0) {
    DLOG("%s - open %s failed: %d", __func__, path, fd);
    return false;
  }

  // Write value
  // We remove const because `uv_buf_init` requires char* for only reading case.
  uv_buf_t uvbuf = uv_buf_init((char*)value, strlen(value));
  int write_err = uv_fs_write(loop, &fs_req, fd, &uvbuf, 1, 0, NULL);
  uv_fs_req_cleanup(&fs_req);

  // Close file
  int close_err = uv_fs_close(loop, &fs_req, fd, NULL);
  uv_fs_req_cleanup(&fs_req);

  if (write_err < 0) {
    DLOG("%s - write %s %s failed: %d", __func__, path, value, write_err);
    return false;
  }

  if (close_err < 0) {
    DLOG("%s - close failed: %d", __func__, close_err);
    return false;
  }

  return true;
}


bool iotjs_systemio_open_read_close(const char* path, char* buffer,
                                    unsigned buffer_len) {
  const iotjs_environment_t* env = iotjs_environment_get();
  uv_loop_t* loop = iotjs_environment_loop(env);

  DDDLOG("%s - path %s", __func__, path);

  // Open file
  uv_fs_t fs_open_req;
  int fd = uv_fs_open(loop, &fs_open_req, path, O_RDONLY, 0666, NULL);
  uv_fs_req_cleanup(&fs_open_req);
  if (fd < 0) {
    DLOG("%s - open %s failed: %d", __func__, path, fd);
    return false;
  }

  // Read value
  uv_fs_t fs_write_req;
  uv_buf_t uvbuf = uv_buf_init(buffer, buffer_len);
  int err = uv_fs_read(loop, &fs_write_req, fd, &uvbuf, 1, 0, NULL);
  uv_fs_req_cleanup(&fs_write_req);
  if (err < 0) {
    DLOG("%s - read failed: %d", __func__, err);
    return false;
  }

  DDDLOG("%s - read value: %s", __func__, buffer);

  // Close file
  uv_fs_t fs_close_req;
  err = uv_fs_close(loop, &fs_close_req, fd, NULL);
  uv_fs_req_cleanup(&fs_close_req);
  if (err < 0) {
    DLOG("%s - close failed: %d", __func__, err);
    return false;
  }

  return true;
}


// Device Open
bool iotjs_systemio_device_open(const char* export_path, uint32_t value,
                                const char* exported_path,
                                const char** created_files,
                                int created_files_length) {
  // Be already exported
  if (iotjs_systemio_check_path(exported_path)) {
    return true;
  }

  DDDLOG("%s - path: %s", __func__, export_path);

  // Write export pin.
  char buff[DEVICE_IO_PIN_BUFFER_SIZE] = { 0 };
  snprintf(buff, DEVICE_IO_PIN_BUFFER_SIZE - 1, "%d", value);

  if (!iotjs_systemio_open_write_close(export_path, buff)) {
    return false;
  }

  // Wait for directory creation.
  int count = 0;
  int count_limit = created_files_length * 10;
  char buffer[DEVICE_IO_PIN_BUFFER_SIZE];
  char path[DEVICE_IO_PATH_BUFFER_SIZE] = { 0 };

  while (!iotjs_systemio_check_path(exported_path) && count < count_limit) {
    usleep(100 * 1000); // sleep 100 miliseconds.
    count++;
  }

  for (int i = 0; i < created_files_length; i++) {
    snprintf(path, DEVICE_IO_PATH_BUFFER_SIZE - 1, "%s%s", exported_path,
             created_files[i]);

    DDDLOG("%s - created file: %s", __func__, path);

    while (!iotjs_systemio_open_read_close(path, buffer,
                                           DEVICE_IO_PIN_BUFFER_SIZE) &&
           count < count_limit) {
      usleep(100 * 1000); // sleep 100 miliseconds.
      count++;
    }
  }

  usleep(1000 * 100); // sleep another 1000 milisec.

  return count < count_limit;
}


// Device close
bool iotjs_systemio_device_close(const char* export_path, uint32_t value) {
  DDDLOG("%s - path: %s", __func__, export_path);

  char buff[DEVICE_IO_PIN_BUFFER_SIZE];
  snprintf(buff, DEVICE_IO_PIN_BUFFER_SIZE - 1, "%d", value);

  if (!iotjs_systemio_open_write_close(export_path, buff)) {
    return false;
  }

  return true;
}
