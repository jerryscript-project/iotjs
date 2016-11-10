/* Copyright 2016 Samsung Electronics Co., Ltd.
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


#include <unistd.h>
#include <uv.h>

#include "iotjs_def.h"
#include "iotjs_systemio-linux.h"


namespace iotjs {


// uv fs request wrapper for auto cleanup.
class LocalDeviceFsReq : public uv_fs_t {
 public:
  ~LocalDeviceFsReq() {
    uv_fs_req_cleanup(this);
  }
};


// Checks if given directory exists.
bool DeviceCheckPath(const char* path) {
  const iotjs_environment_t* env = iotjs_environment_get();

  DDDLOG("DeviceCheckPath() - path: %s", path);

  // stat for the path.
  LocalDeviceFsReq fs_req;
  int err = uv_fs_stat(iotjs_environment_loop(env), &fs_req, path, NULL);

  // exist?
  if (err || fs_req.result) {
    return false;
  }

  DDDLOG("DeviceCheckPath() - path exist");

  return true;
}


bool DeviceOpenWriteClose(const char* path, char* value) {
  const iotjs_environment_t* env = iotjs_environment_get();
  uv_loop_t* loop = iotjs_environment_loop(env);

  DDDLOG("DeviceOpenWriteClose() - path %s, value: %s", path, value);

  // Open file.
  LocalDeviceFsReq fs_req;
  int fd = uv_fs_open(loop, &fs_req, path, O_WRONLY, 0666, NULL);
  if (fd < 0) {
    DDLOG("DeviceOpenWriteClose() - open %s failed: %d", path, fd);
    return false;
  }

  // Write value.
  uv_buf_t uvbuf = uv_buf_init(value, strlen(value));
  uv_fs_req_cleanup(&fs_req);
  int write_err = uv_fs_write(loop, &fs_req, fd, &uvbuf, 1, 0, NULL);

  // Close file.
  uv_fs_req_cleanup(&fs_req);
  int close_err = uv_fs_close(loop, &fs_req, fd, NULL);

  if (write_err < 0) {
    DDLOG("DeviceOpenWriteClose() - write %s %s failed: %d", path, value,
          write_err);
    return false;
  }

  if (close_err < 0) {
    DDLOG("DeviceOpenWriteClose() - close failed: %d", close_err);
    return false;
  }

  return true;
}


bool DeviceOpenReadClose(const char* path, char* buffer, int buffer_len) {
  const iotjs_environment_t* env = iotjs_environment_get();
  uv_loop_t* loop = iotjs_environment_loop(env);

  DDDLOG("DeviceOpenReadClose() - path %s", path);

  // Open file.
  LocalDeviceFsReq fs_open_req;
  int fd = uv_fs_open(loop, &fs_open_req, path, O_RDONLY, 0666, NULL);
  if (fd < 0) {
    DDLOG("DeviceOpenReadClose() - open %s failed: %d", path, fd);
    return false;
  }

  // Write value.
  LocalDeviceFsReq fs_write_req;
  uv_buf_t uvbuf = uv_buf_init(buffer, buffer_len);
  int err = uv_fs_read(loop, &fs_write_req, fd, &uvbuf, 1, 0, NULL);
  if (err < 0) {
    DDLOG("DeviceOpenReadClose() - read failed: %d", err);
    return false;
  }

  DDDLOG("DeviceOpenReadClose() - read value: %s", buffer);

  // Close file.
  LocalDeviceFsReq fs_close_req;
  err = uv_fs_close(loop, &fs_close_req, fd, NULL);
  if (err < 0) {
    DDLOG("DeviceOpenReadClose() - close failed: %d", err);
    return false;
  }

  return true;
}


// Export.
bool DeviceExport(const char* export_path, int value, const char* exported_path,
                  const char** created_files, int created_files_length) {

  // Be already exported
  if (DeviceCheckPath(exported_path)) {
    return true;
  }

  DDLOG("DeviceExport() - path: %s", export_path);

  // Write export pin.
  char buff[DEVICE_IO_PIN_BUFFER_SIZE] = {0};
  snprintf(buff, DEVICE_IO_PIN_BUFFER_SIZE - 1, "%d", value);

  if (!DeviceOpenWriteClose(export_path, buff)) {
    return false;
  }

  // Wait for directory creation.
  int count = 0;
  int count_limit = created_files_length * 10;
  char buffer[DEVICE_IO_PIN_BUFFER_SIZE];
  char path[DEVICE_IO_PATH_BUFFER_SIZE] = {0};
  char check_format[DEVICE_IO_PATH_BUFFER_SIZE] = {0};

  while (!DeviceCheckPath(exported_path) && count < count_limit) {
    usleep(100 * 1000); // sleep 100 miliseconds.
    count++;
  }

  strcat(check_format, exported_path);
  strcat(check_format, "%s");

  for (int i = 0; i < created_files_length; i++) {
    snprintf(path, DEVICE_IO_PATH_BUFFER_SIZE - 1,
             check_format, created_files[i]);

    DDLOG("DeviceExport() - created file: %s", path);

    while (!DeviceOpenReadClose(path, buffer, DEVICE_IO_PIN_BUFFER_SIZE) &&
           count < count_limit) {
      usleep(100 * 1000); // sleep 100 miliseconds.
      count++;
    }
  }

  usleep(1000 * 100); // sleep another 1000 milisec.

  return count < count_limit;
}


// Unexport.
bool DeviceUnexport(const char* export_path, int value) {

  DDDLOG("Device Unexport() - path: %s", DeviceUnexport);

  char buff[DEVICE_IO_PIN_BUFFER_SIZE] = {0};
  snprintf(buff, DEVICE_IO_PIN_BUFFER_SIZE - 1, "%d", value);

  if (!DeviceOpenWriteClose(export_path, buff)) {
    return false;
  }

  return true;
}


} // namespace iotjs
