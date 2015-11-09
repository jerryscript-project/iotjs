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

file(GLOB LIB_IOTJS_SRC ${SRC_ROOT}/*.cpp
                        ${SRC_ROOT}/platform/${PLATFORM_DESCRIPT}/*.cpp)

string(REPLACE ";" " " IOTJS_CFLAGS_STR "${IOTJS_CFLAGS}")
string(REPLACE ";" " " IOTJS_LINK_FLAGS_STR "${IOTJS_LINK_FLAGS}")
separate_arguments(EXTERNAL_INCLUDE_DIR)
separate_arguments(EXTERNAL_STATIC_LIB)
separate_arguments(EXTERNAL_SHARED_LIB)

set(LIB_IOTJS_CFLAGS ${IOTJS_CFLAGS_STR})
set(LIB_IOTJS_INCDIR ${EXTERNAL_INCLUDE_DIR}
                     ${INC_ROOT}
                     ${SRC_ROOT}
                     ${JERRY_INCDIR}
                     ${LIBTUV_INCDIR}
                     ${HTTPPARSER_INCDIR})

add_custom_target(targetLibIoTjs)

function(BuildLibIoTjs)
  set(targetName libiotjs)

  add_library(${targetName} STATIC ${LIB_IOTJS_SRC})
  set_property(TARGET ${targetName}
               PROPERTY COMPILE_FLAGS ${LIB_IOTJS_CFLAGS})
  target_include_directories(${targetName} PRIVATE ${LIB_IOTJS_INCDIR})
endfunction()

BuildLibIoTjs()


set(SRC_MAIN ${ROOT}/iotjs_linux.cpp)

set(BIN_IOTJS_CFLAGS ${IOTJS_CFLAGS_STR})
set(BIN_IOTJS_LINK_FLAGS ${IOTJS_LINK_FLAGS_STR})
set(BIN_IOTJS_INCDIR ${LIB_IOTJS_INCDIR})

function(BuildIoTjs)
  set(targetName iotjs)

  add_executable(${targetName} ${SRC_MAIN})
  set_property(TARGET ${targetName}
               PROPERTY COMPILE_FLAGS ${BIN_IOTJS_CFLAGS})
  set_property(TARGET ${targetName}
               PROPERTY LINK_FLAGS ${BIN_IOTJS_LINK_FLAGS})
  target_include_directories(${targetName} PRIVATE ${BIN_IOTJS_INCDIR})
  target_link_libraries(${targetName} libiotjs ${JERRY_LIB} ${LIBTUV_LIB}
    ${HTTPPARSER_LIB} ${EXTERNAL_STATIC_LIB} ${EXTERNAL_SHARED_LIB})
  add_dependencies(targetLibIoTjs ${targetName})

endfunction()

function(BuildIoTjsLib)
  set(targetName iotjs)

  add_library(${targetName} ${SRC_MAIN})
  set_property(TARGET ${targetName}
               PROPERTY COMPILE_FLAGS ${BIN_IOTJS_CFLAGS})
  set_property(TARGET ${targetName}
               PROPERTY LINK_FLAGS ${BIN_IOTJS_LINK_FLAGS})
  target_include_directories(${targetName} PRIVATE ${BIN_IOTJS_INCDIR})
  target_link_libraries(${targetName} libiotjs ${JERRY_LIB} ${LIBTUV_LIB}
    ${HTTPPARSER_LIB} ${EXTERNAL_STATIC_LIB} ${EXTERNAL_SHARED_LIB})
  add_dependencies(targetLibIoTjs ${targetName})
endfunction()

if(${BUILD_TO_LIB})
  BuildIoTjsLib()
else()
  BuildIoTjs()
endif()
