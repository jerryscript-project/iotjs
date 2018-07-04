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

# Host jerry for snapshot generation
set(DEPS_HOST_JERRY deps/jerry-host)
ExternalProject_Add(hostjerry
  PREFIX ${DEPS_HOST_JERRY}
  SOURCE_DIR ${ROOT_DIR}/deps/jerry/
  BUILD_IN_SOURCE 0
  BINARY_DIR ${DEPS_HOST_JERRY}
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/${DEPS_HOST_JERRY}
    -DENABLE_ALL_IN_ONE=ON
    -DENABLE_LTO=${ENABLE_LTO}
    -DJERRY_LIBC=OFF
    -DJERRY_CMDLINE=OFF
    -DJERRY_CMDLINE_SNAPSHOT=ON
    -DJERRY_EXT=OFF
    -DFEATURE_SNAPSHOT_SAVE=${ENABLE_SNAPSHOT}
    -DFEATURE_PROFILE=${FEATURE_PROFILE}
)
set(JERRY_HOST_SNAPSHOT
    ${CMAKE_BINARY_DIR}/${DEPS_HOST_JERRY}/bin/jerry-snapshot)
add_executable(jerry-snapshot IMPORTED)
add_dependencies(jerry-snapshot hostjerry)
set_property(TARGET jerry-snapshot PROPERTY
  IMPORTED_LOCATION ${JERRY_HOST_SNAPSHOT})

# Utility method to add -D<KEY>=<KEY_Value>
macro(add_cmake_arg TARGET_ARG KEY)
  if(${KEY})
    list(APPEND ${TARGET_ARG} -D${KEY}=${${KEY}})
  endif()
endmacro(add_cmake_arg)

# Target libjerry
set(JERRY_LIBS jerry-core jerry-port-default)
set(DEPS_LIB_JERRY_ARGS)

# Configure the MinSizeRel as the default build type
# for target jerry in release mode.
if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  set(JERRY_CMAKE_BUILD_TYPE MinSizeRel)
else()
  set(JERRY_CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE})
endif()


# use system libc/libm on Unix like targets
if("${TARGET_OS}" MATCHES "TIZENRT|NUTTX")
  list(APPEND JERRY_LIBS jerry-libm)
  list(APPEND DEPS_LIB_JERRY_ARGS
    -DJERRY_LIBC=OFF
    -DJERRY_LIBM=ON
    -DEXTERNAL_CMAKE_SYSTEM_PROCESSOR=${EXTERNAL_CMAKE_SYSTEM_PROCESSOR}
  )
elseif("${TARGET_OS}" MATCHES "LINUX|TIZEN|DARWIN|OPENWRT")
  list(APPEND JERRY_LIBS m)
  list(APPEND DEPS_LIB_JERRY_ARGS
    -DJERRY_LIBC=OFF
    -DJERRY_LIBM=OFF)
elseif("${TARGET_OS}" MATCHES "WINDOWS")
  list(APPEND DEPS_LIB_JERRY_ARGS
    -DJERRY_LIBC=OFF
    -DJERRY_LIBM=OFF)
else()
  list(APPEND JERRY_LIBS jerry-libm jerry-libc)
  list(APPEND DEPS_LIB_JERRY_ARGS
    -DEXTERNAL_CMAKE_SYSTEM_PROCESSOR=${EXTERNAL_CMAKE_SYSTEM_PROCESSOR}
  )
endif()

# Add a few cmake options based on buildtype/external cmake defines
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  list(APPEND DEPS_LIB_JERRY_ARGS -DFEATURE_ERROR_MESSAGES=ON)
endif()

# NuttX is not using the default port implementation of JerryScript
if("${TARGET_OS}" MATCHES "NUTTX")
  list(APPEND DEPS_LIB_JERRY_ARGS -DJERRY_PORT_DEFAULT=OFF)
else()
  list(APPEND DEPS_LIB_JERRY_ARGS -DJERRY_PORT_DEFAULT=ON)
endif()

add_cmake_arg(DEPS_LIB_JERRY_ARGS ENABLE_LTO)
add_cmake_arg(DEPS_LIB_JERRY_ARGS FEATURE_MEM_STATS)
add_cmake_arg(DEPS_LIB_JERRY_ARGS FEATURE_ERROR_MESSAGES)
add_cmake_arg(DEPS_LIB_JERRY_ARGS FEATURE_DEBUGGER)
add_cmake_arg(DEPS_LIB_JERRY_ARGS MEM_HEAP_SIZE_KB)
add_cmake_arg(DEPS_LIB_JERRY_ARGS JERRY_HEAP_SECTION_ATTR)

separate_arguments(EXTRA_JERRY_CMAKE_PARAMS)

build_lib_name(JERRY_CORE_NAME jerry-core)
build_lib_name(JERRY_LIBC_NAME jerry-libc)
build_lib_name(JERRY_LIBM_NAME jerry-libm)

set(DEPS_LIB_JERRY deps/jerry)
set(DEPS_LIB_JERRY_SRC ${ROOT_DIR}/${DEPS_LIB_JERRY})
ExternalProject_Add(libjerry
  PREFIX ${DEPS_LIB_JERRY}
  SOURCE_DIR ${ROOT_DIR}/deps/jerry/
  BUILD_IN_SOURCE 0
  BINARY_DIR ${DEPS_LIB_JERRY}
  INSTALL_COMMAND
    ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_BINARY_DIR}/${DEPS_LIB_JERRY}/lib/${CONFIG_TYPE}
    ${CMAKE_BINARY_DIR}/lib/
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_BUILD_TYPE=${JERRY_CMAKE_BUILD_TYPE}
    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DENABLE_ALL_IN_ONE=ON
    -DJERRY_CMDLINE=OFF
    -DFEATURE_SNAPSHOT_EXEC=${ENABLE_SNAPSHOT}
    -DFEATURE_SNAPSHOT_SAVE=OFF
    -DFEATURE_PROFILE=${FEATURE_PROFILE}
    -DFEATURE_LINE_INFO=${FEATURE_JS_BACKTRACE}
    -DFEATURE_VM_EXEC_STOP=ON
    -DENABLE_LTO=${ENABLE_LTO}
    ${DEPS_LIB_JERRY_ARGS}
    ${EXTRA_JERRY_CMAKE_PARAMS}
)

set_property(DIRECTORY APPEND PROPERTY
  ADDITIONAL_MAKE_CLEAN_FILES
    ${CMAKE_BINARY_DIR}/lib/${JERRY_CORE_NAME}
    ${CMAKE_BINARY_DIR}/lib/${JERRY_LIBC_NAME}
    ${CMAKE_BINARY_DIR}/lib/${JERRY_LIBM_NAME}
)

# define external jerry-core target
add_library(jerry-core STATIC IMPORTED)
add_dependencies(jerry-core libjerry)
set_property(TARGET jerry-core PROPERTY
  IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/lib/${JERRY_CORE_NAME})

# define external jerry-libc target
add_library(jerry-libc STATIC IMPORTED)
add_dependencies(jerry-libc libjerry)
set_property(TARGET jerry-libc PROPERTY
  IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/lib/${JERRY_LIBC_NAME})

# define external jerry-libm target
add_library(jerry-libm STATIC IMPORTED)
add_dependencies(jerry-libm libjerry)
set_property(TARGET jerry-libm PROPERTY
  IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/lib/${JERRY_LIBM_NAME})

if(NOT "${TARGET_OS}" MATCHES "NUTTX")
  build_lib_name(JERRY_PORT_NAME jerry-port)
  build_lib_name(JERRY_PORT_DEFAULT_NAME jerry-port-default)
  set_property(DIRECTORY APPEND PROPERTY
    ADDITIONAL_MAKE_CLEAN_FILES
      ${CMAKE_BINARY_DIR}/lib/${JERRY_PORT_NAME}
  )

  # define external jerry-port-default target
  add_library(jerry-port-default STATIC IMPORTED)
  add_dependencies(jerry-port-default libjerry)
  set_property(TARGET jerry-port-default PROPERTY
    IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/lib/${JERRY_PORT_DEFAULT_NAME})

  set(JERRY_PORT_DIR ${DEPS_LIB_JERRY_SRC}/jerry-port/default)
endif()

set(JERRY_INCLUDE_DIR ${DEPS_LIB_JERRY_SRC}/jerry-core/include)
