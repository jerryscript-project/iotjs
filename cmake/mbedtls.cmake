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

cmake_minimum_required(VERSION 2.8)

set(DEPS_MBEDTLS deps/mbedtls)
set(DEPS_MBEDTLS_SRC ${ROOT_DIR}/${DEPS_MBEDTLS})
set(DEPS_MBEDTLS_BUILD_DIR ${CMAKE_BINARY_DIR}/${DEPS_MBEDTLS}/library)
set(MODULE_NAME "tls")
set(MODULE_BINARY_DIR ${DEPS_MBEDTLS_BUILD_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-sign-conversion")

ExternalProject_Add(mbedtls
  PREFIX ${DEPS_MBEDTLS}
  SOURCE_DIR ${DEPS_MBEDTLS_SRC}
  BUILD_IN_SOURCE 0
  BINARY_DIR ${DEPS_MBEDTLS}
  INSTALL_COMMAND
    COMMAND ${CMAKE_COMMAND} -E copy
      ${DEPS_MBEDTLS_BUILD_DIR}/libmbedx509.a ${ARCHIVE_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy
      ${DEPS_MBEDTLS_BUILD_DIR}/libmbedtls.a ${ARCHIVE_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy
      ${DEPS_MBEDTLS_BUILD_DIR}/libmbedcrypto.a ${ARCHIVE_DIR}
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DENABLE_PROGRAMS=OFF
    -DENABLE_TESTING=OFF
)

# define external mbedtls target
add_library(libmbedtls STATIC IMPORTED)
add_dependencies(libmbedtls mbedtls)
set_property(TARGET libmbedtls PROPERTY
  IMPORTED_LOCATION ${ARCHIVE_DIR}/libmbedtls.a)

# define external mbedx509 target
add_library(libmbedx509 STATIC IMPORTED)
add_dependencies(libmbedx509 mbedx509)
set_property(TARGET libmbedx509 PROPERTY
  IMPORTED_LOCATION ${ARCHIVE_DIR}/libmbedx509.a)

# define external libmbedcrypto target
add_library(libmbedcrypto STATIC IMPORTED)
add_dependencies(libmbedcrypto mbedcrypto)
set_property(TARGET libmbedcrypto PROPERTY
  IMPORTED_LOCATION ${ARCHIVE_DIR}/libmbedcrypto.a)

set_property(DIRECTORY APPEND PROPERTY
  ADDITIONAL_MAKE_CLEAN_FILES
  ${ARCHIVE_DIR}/libmbedx509.a
  ${ARCHIVE_DIR}/libmbedtls.a
  ${ARCHIVE_DIR}/libmbedcrypto.a
)

set(MBEDTLS_LIBS libmbedtls libmbedx509 libmbedcrypto)
set(MBEDTLS_INCLUDE_DIR ${DEPS_MBEDTLS}/include)
