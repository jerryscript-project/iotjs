# Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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

include(CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR armv7l)

# Only set the compiler if not provided already
if(NOT "${CMAKE_C_COMPILER}" STREQUAL "")
  find_program(COMPILER_PATH ${CMAKE_C_COMPILER})
  if(COMPILER_PATH STREQUAL "")
    message(WARNING "Command ${CMAKE_C_COMPILER} not found")
    unset(CMAKE_C_COMPILER)
  endif()
  unset(COMPILER_PATH)
endif()

if("${CMAKE_C_COMPILER}" STREQUAL "")
  set(CMAKE_C_COMPILER arm-linux-gnueabi-gcc)
endif()
