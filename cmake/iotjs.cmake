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

# Module Configuration - listup all possible modules
file(GLOB IOTJS_MODULES_ALL ${CMAKE_SOURCE_DIR}/src/module/*.c)
separate_arguments(IOTJS_MODULES_ALL)
foreach(module ${IOTJS_MODULES_ALL})
    get_filename_component(IOTJS_MODULENAME ${module} NAME_WE)
    string(SUBSTRING ${IOTJS_MODULENAME} 13 -1 IOTJS_MODULENAME)
    string(TOUPPER ${IOTJS_MODULENAME} IOTJS_MODULENAME)
    set(IOTJS_CFLAGS "${IOTJS_CFLAGS} -DENABLE_MODULE_${IOTJS_MODULENAME}=0")
endforeach()

# System Configuration
set(IOTJS_PLATFORM_SRC "")
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    list(APPEND IOTJS_PLATFORM_SRC ${SRC_ROOT}/platform/iotjs_*-linux.c)
endif()

# Board Configuration (not module)
if(DEFINED CMAKE_TARGET_BOARD)
    set(BOARD_DESCRIPT "")
    string(TOLOWER ${CMAKE_TARGET_BOARD} BOARD_DESCRIPT)
    set(PLATFORM_SRC ${SRC_ROOT}/platform/${PLATFORM_DESCRIPT})
    set(PLATFORM_SRC ${PLATFORM_SRC}/iotjs_[^module]*${BOARD_DESCRIPT}.c)
    list(APPEND IOTJS_PLATFORM_SRC ${PLATFORM_SRC})
endif()

# Module Configuration - enable only selected modules and board
set(IOTJS_MODULE_SRC "")
separate_arguments(IOTJS_MODULES)
set(PLATFORM_SRC "${SRC_ROOT}/platform/${PLATFORM_DESCRIPT}/iotjs_module")
foreach(module ${IOTJS_MODULES})
    list(APPEND IOTJS_MODULE_SRC ${SRC_ROOT}/module/iotjs_module_${module}.c)
    set(MODULE_SRC "${PLATFORM_SRC}_${module}-${PLATFORM_DESCRIPT}.c")
    if(EXISTS "${MODULE_SRC}")
        list(APPEND IOTJS_MODULE_SRC ${MODULE_SRC})
    endif()
    if(DEFINED BOARD_DESCRIPT)
        set(MODULE_SRC "${PLATFORM_SRC}_${module}-")
        set(MODULE_SRC "${MODULE_SRC}${PLATFORM_DESCRIPT}-${BOARD_DESCRIPT}.c")
        if(EXISTS "${MODULE_SRC}")
            list(APPEND IOTJS_MODULE_SRC ${MODULE_SRC})
        endif()
    endif()
    string(TOUPPER ${module} module)
    set(IOTJS_CFLAGS "${IOTJS_CFLAGS} -UENABLE_MODULE_${module}")
    set(IOTJS_CFLAGS "${IOTJS_CFLAGS} -DENABLE_MODULE_${module}=1")
endforeach()



file(GLOB LIB_IOTJS_SRC ${SRC_ROOT}/*.c
                        ${IOTJS_MODULE_SRC}
                        ${IOTJS_PLATFORM_SRC})

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


set(SRC_MAIN ${ROOT}/iotjs_linux.c)

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
