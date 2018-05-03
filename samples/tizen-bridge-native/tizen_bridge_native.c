/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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
#include "iotjs_tizen_service_app.h"

/* thread */
#include <pthread.h>
#include <unistd.h>
/* printf */
#include <stdio.h>


static void user_cb(int err, const char* data) {
  printf("err: %d, data: %s\n", err, data);
}


void* thread(void* data) {
  sleep(1);

  char* str = "1234567A1234567B1234567C";
  IOTJS_TIZEN_CALL_JFUNC("hello", "world", user_cb);
  IOTJS_TIZEN_CALL_JFUNC("hello", str, user_cb);
  IOTJS_TIZEN_CALL_JFUNC("hello", "", user_cb);
  IOTJS_TIZEN_CALL_JFUNC("", "", user_cb);
  IOTJS_TIZEN_CALL_JFUNC("", "", NULL);
  IOTJS_TIZEN_CALL_JFUNC("notReturnString", "", user_cb);
  return 0;
}


int main(int argc, char** argv) {
  pthread_t tid;
  if (pthread_create(&(tid), NULL, &thread, NULL)) {
    return 1;
  }

  return iotjs_entry(argc, argv);
}
