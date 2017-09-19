# Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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

project(IOTJS C)

set(IOTJS_VERSION_MAJOR 0)
set(IOTJS_VERSION_MINOR 1)

# Do a few default checks
if(NOT DEFINED PLATFORM_DESCRIPTOR)
  message(FATAL_ERROR "No PLATFORM_DESCRIPTOR specified (format: <arch>-<os>)")
endif()

string(REPLACE "-" ";" PLATFORM_ARGS ${PLATFORM_DESCRIPTOR})
if(NOT DEFINED TARGET_OS)
  list(GET PLATFORM_ARGS 1 TARGET_OS)
  message(
    "TARGET_OS not specified, using '${TARGET_OS}' from PLATFORM_DESCRIPTOR")
endif()
string(TOUPPER "${TARGET_OS}" TARGET_OS)

if(NOT CMAKE_BUILD_TYPE)
  message("CMAKE_BUILD_TYPE was not set! Configuring for Debug build!")
  set(CMAKE_BUILD_TYPE Debug)
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  message("CMAKE_BUILD_TYPE was set to Release, switching to MinSizeRel")
  set(CMAKE_BUILD_TYPE MinSizeRel)
endif()

if(NOT DEFINED BUILD_LIB_ONLY)
  set(BUILD_LIB_ONLY OFF)
endif()

if(NOT DEFINED ENABLE_SNAPSHOT)
  message("Snapshot mode force enabled")
  set(ENABLE_SNAPSHOT ON)
endif()

if(NOT DEFINED ENABLE_LTO)
  message("LTO force disabled")
  set(ENABLE_LTO OFF)
endif()

set(ROOT_DIR ${CMAKE_SOURCE_DIR})

# Common compile flags
set(CFLAGS_COMMON
  -Wall
  -Wextra
  -Werror
  -Wno-unused-parameter
  -Wsign-conversion
  -std=gnu99
)

include(ExternalProject)

# Include external projects
include(cmake/jerry.cmake)
include(cmake/http-parser.cmake)
include(cmake/libtuv.cmake)

include(cmake/iotjs.cmake)

