# Copyright 2015 Samsung Electronics Co., Ltd.
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

cmake_minimum_required(VERSION 2.8)

set(CC ${CMAKE_C_COMPILER})
set(CXX ${CMAKE_CXX_COMPILER})

set(CFLAGS ${CMAKE_CXX_FLAGS})
#set(CFLAGS "${CFLAGS} -flto")
if (NOT(${NO_PTHREAD}))
    set(CFLAGS "${CFLAGS} -pthread -Wall")
endif()

set(ROOT ${CMAKE_SOURCE_DIR})
set(SRC_ROOT ${ROOT}/src)
set(INC_ROOT ${ROOT}/include)
set(DEP_ROOT ${ROOT}/deps)
set(BIN_ROOT ${CMAKE_BINARY_DIR})
set(LIB_ROOT ${BIN_ROOT}/../libs)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
  set(CFLAGS "${CFLAGS} -O2")
else ()
  set(CFLAGS "${CFLAGS} -g")
  set(CFLAGS "${CFLAGS} -DENABLE_DEBUG_LOG")
endif()
