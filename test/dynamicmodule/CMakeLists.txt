# Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# This is a standalone shared libray which
# only requires the iotjs and jerry header file(s).
#
cmake_minimum_required(VERSION 2.8)
set(NAME dynamicmodule)

# Currently only Linux and Tizen targets are supported
if(("${TARGET_OS}" STREQUAL "LINUX") OR ("${TARGET_OS}" STREQUAL "TIZEN"))

  if((NOT IOTJS_INCLUDE_DIR) OR (NOT JERRY_INCLUDE_DIR))
    message(FATAL_ERROR "No 'IOTJS_INCLUDE_DIR' or 'JERRY_INCLUDE_DIR'")
  endif()

  string(TOLOWER ${TARGET_OS} TARGET_OS_NAME)

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
  set(OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/${TARGET_OS_NAME}")

  add_library(${NAME} SHARED
              src/module_entry.c)
  target_include_directories(${NAME}
                             PRIVATE ${IOTJS_INCLUDE_DIR} ${JERRY_INCLUDE_DIR})
  set_target_properties(${NAME} PROPERTIES
                        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY}
                        PREFIX ""
                        SUFFIX ".iotjs")
endif()
