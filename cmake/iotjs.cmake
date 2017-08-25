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

set(IOTJS_SOURCE_DIR ${ROOT_DIR}/src)

function(find_value RESULT VALUE VALUE_TRUE VALUE_FALSE)
  list(FIND ARGN ${VALUE} idx)
  if(${idx} GREATER -1)
    set(${RESULT} ${VALUE_TRUE} PARENT_SCOPE)
  else()
    set(${RESULT} ${VALUE_FALSE} PARENT_SCOPE)
  endif()
endfunction(find_value)

# System Configuration (not module)
string(TOLOWER ${CMAKE_SYSTEM_NAME} IOTJS_SYSTEM_OS)
set(PLATFORM_OS_DIR
    ${IOTJS_SOURCE_DIR}/platform/${IOTJS_SYSTEM_OS})
file(GLOB IOTJS_PLATFORM_SRC ${PLATFORM_OS_DIR}/iotjs_*.c)
file(GLOB PLATFORM_MODULE_SRC ${PLATFORM_OS_DIR}/iotjs_module_*.c)
if (IOTJS_PLATFORM_SRC AND PLATFORM_MODULE_SRC)
  list(REMOVE_ITEM IOTJS_PLATFORM_SRC ${PLATFORM_MODULE_SRC})
endif()

# Board Configuration (not module)
if(NOT "${TARGET_BOARD}" STREQUAL "None")
  set(PLATFORM_BOARD_DIR
      ${PLATFORM_OS_DIR}/${TARGET_BOARD})
  file(GLOB IOTJS_BOARD_SRC ${PLATFORM_BOARD_DIR}/iotjs_*.c)
  file(GLOB PLATFORM_MODULE_SRC ${PLATFORM_BOARD_DIR}/iotjs_module_*.c)
  if (IOTJS_BOARD_SRC AND PLATFORM_MODULE_SRC)
    list(REMOVE_ITEM IOTJS_BOARD_SRC ${PLATFORM_MODULE_SRC})
  endif()
  list(APPEND IOTJS_PLATFORM_SRC ${IOTJS_BOARD_SRC})
endif()

# Run js/native module analyzer
if(ENABLE_MINIMAL)
    set(MODULE_ANALYZER_ARGS --iotjs-minimal-profile)
endif()

execute_process(
  COMMAND python ${ROOT_DIR}/tools/module_analyzer.py
          --mode cmake-dump
          --target-os ${TARGET_OS}
          --iotjs-include-module "${IOTJS_INCLUDE_MODULE}"
          --iotjs-exclude-module "${IOTJS_EXCLUDE_MODULE}"
          ${MODULE_ANALYZER_ARGS}
  RESULT_VARIABLE MODULE_ANALYZER_RETURN_CODE
  ERROR_VARIABLE MODULE_ANALYZER_OUTPUT_ERR
  OUTPUT_VARIABLE MODULE_ANALYZER_OUTPUT
)

if(MODULE_ANALYZER_RETURN_CODE)
  message(FATAL_ERROR
    "Error during module analyzer execution (${MODULE_ANALYZER_RETURN_CODE}): "
    "${MODULE_ANALYZER_OUTPUT}"
    "${MODULE_ANALYZER_OUTPUT_ERR}")
endif()

if(VERBOSE)
  message("Module analyzer:\n${MODULE_ANALYZER_OUTPUT}")
endif()

function(get_variable_value OUTPUT_VAR VAR_NAME STRING_DATA)
  string(REGEX MATCHALL "${VAR_NAME}=[a-zA-Z;_0-9-]+" LINE "${STRING_DATA}")
  string(REPLACE "${VAR_NAME}=" "" VAR_VALUE "${LINE}")
  string(STRIP "${VAR_VALUE}" VAR_VALUE)
  separate_arguments(VAR_VALUE)
  set(${OUTPUT_VAR} ${VAR_VALUE} PARENT_SCOPE)
endfunction(get_variable_value)

get_variable_value(IOTJS_NATIVE_MODULES
  "IOTJS_NATIVE_MODULES" "${MODULE_ANALYZER_OUTPUT}")
get_variable_value(IOTJS_JS_MODULES
  "IOTJS_JS_MODULES" "${MODULE_ANALYZER_OUTPUT}")

# Run js2c
set(JS2C_RUN_MODE "release")
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  set(JS2C_RUN_MODE "debug")
endif()

if(ENABLE_SNAPSHOT)
  set(JS2C_SNAPSHOT_ARG --snapshot-generator=${JERRY_HOST})
  set(IOTJS_CFLAGS ${IOTJS_CFLAGS} -DENABLE_SNAPSHOT)
endif()

add_custom_command(
  OUTPUT ${IOTJS_SOURCE_DIR}/iotjs_js.c ${IOTJS_SOURCE_DIR}/iotjs_js.h
  COMMAND python ${ROOT_DIR}/tools/js2c.py
  ARGS --buildtype=${JS2C_RUN_MODE}
       --modules '${IOTJS_JS_MODULES}'
       ${JS2C_SNAPSHOT_ARG}
  DEPENDS ${ROOT_DIR}/tools/js2c.py
          jerry
          ${IOTJS_SOURCE_DIR}/js/*.js
)

# Module Configuration - listup all possible native C modules
set(IOTJS_MODULES_ENABLED)
set(IOTJS_MODULES_DISABLED)
# List all modules and mark them as disabled by default
file(GLOB IOTJS_MODULES_ALL_SRC ${IOTJS_SOURCE_DIR}/modules/*.c)
foreach(module ${IOTJS_MODULES_ALL_SRC})
  ## iotjs_module_adc.c -> ADC
  get_filename_component(IOTJS_MODULENAME ${module} NAME_WE)
  string(SUBSTRING ${IOTJS_MODULENAME} 13 -1 IOTJS_MODULENAME)
  string(TOUPPER ${IOTJS_MODULENAME} IOTJS_MODULENAME)
  list(APPEND IOTJS_MODULES_DISABLED ${IOTJS_MODULENAME})
endforeach()

# Module Configuration - enable only selected modules and add board support
set(IOTJS_PLATFORM_SUPPORT)
set(IOTJS_BOARD_SUPPORT)
set(IOTJS_MODULES_SRC)
set(PLATFORM_SRC
  ${IOTJS_SOURCE_DIR}/platform/${PLATFORM_DESCRIPTOR}/iotjs_module)
foreach(module ${IOTJS_NATIVE_MODULES})
  string(TOUPPER ${module} MODULE)
  # check if there is a native file for the module
  set(BASE_MODULE_SRC ${IOTJS_SOURCE_DIR}/modules/iotjs_module_${module}.c)
  if(EXISTS "${BASE_MODULE_SRC}")
    list(APPEND IOTJS_MODULE_SRC ${BASE_MODULE_SRC})
  endif()

  # first, check if there is the module in <os>/<board>
  set(ADD_MODULE_RESULT FALSE)
  if(NOT "${TARGET_BOARD}" STREQUAL "None")
    set(PLATFORM_MODULE_SRC ${PLATFORM_BOARD_DIR}/iotjs_module_${module})
    set(PLATFORM_MODULE_SRC
        ${PLATFORM_MODULE_SRC}-${IOTJS_SYSTEM_OS}-${TARGET_BOARD}.c)
    if(EXISTS "${PLATFORM_MODULE_SRC}")
      list(APPEND IOTJS_MODULE_SRC ${PLATFORM_MODULE_SRC})
      list(APPEND IOTJS_BOARD_SUPPORT ${MODULE})
      set(${ADD_MODULE_RESULT} TRUE)
    else()
      set(${ADD_MODULE_RESULT} FALSE)
    endif()
  endif()

  # if the module is not in <os>/<board>, look in <os>
  if(NOT ${ADD_MODULE_RESULT})
    set(PLATFORM_MODULE_SRC
        ${PLATFORM_OS_DIR}/iotjs_module_${module}-${IOTJS_SYSTEM_OS}.c)
    if(EXISTS "${PLATFORM_MODULE_SRC}")
      list(APPEND IOTJS_MODULE_SRC ${PLATFORM_MODULE_SRC})
      list(APPEND IOTJS_PLATFORM_SUPPORT ${MODULE})
    endif()
  endif()

  list(APPEND IOTJS_MODULES_ENABLED ${MODULE})
  list(REMOVE_ITEM IOTJS_MODULES_DISABLED ${MODULE})
endforeach()
# Build the module enable defines and print out the module configurations
message("Native module configuration:")
set(IOTJS_MODULES_ALL ${IOTJS_MODULES_ENABLED} ${IOTJS_MODULES_DISABLED})
list(SORT IOTJS_MODULES_ALL)
foreach(module ${IOTJS_MODULES_ALL})
  find_value(MODULE_ENABLED "${module}" 1 0 ${IOTJS_MODULES_ENABLED})
  list(APPEND IOTJS_CFLAGS "-DENABLE_MODULE_${module}=${MODULE_ENABLED}")

  if(MODULE_ENABLED)
    find_value(PLATFORM_SUPPORT "${module}" "found" "NOT found"
               ${IOTJS_PLATFORM_SUPPORT})
    if(DEFINED TARGET_BOARD)
      find_value(BOARD_SUPPORT "${module}" "found" "NOT found"
        ${IOTJS_BOARD_SUPPORT})
      set(BOARD_SUPPORT_STR "[Board support: ${BOARD_SUPPORT}]")
    else()
      set(BOARD_SUPPORT_STR "")
    endif()

    message(STATUS "${module}: ON "
            "[Platform support: ${PLATFORM_SUPPORT}]"
            "${BOARD_SUPPORT_STR}")
  else()
    message(STATUS "${module}: OFF")
  endif()
endforeach()

# List the enabled js modules
message("Enabled JS modules:")
foreach(module ${IOTJS_JS_MODULES})
  message(STATUS "${module}")
endforeach()

# Print out some configs
message("IoT.js configured with:")
message(STATUS "CMAKE_BUILD_TYPE         ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_C_FLAGS            ${CMAKE_C_FLAGS}")
message(STATUS "PLATFORM_DESCRIPTOR      ${PLATFORM_DESCRIPTOR}")
message(STATUS "TARGET_OS                ${TARGET_OS}")
message(STATUS "TARGET_SYSTEMROOT        ${TARGET_SYSTEMROOT}")
message(STATUS "TARGET_BOARD             ${TARGET_BOARD}")
message(STATUS "BUILD_LIB_ONLY           ${BUILD_LIB_ONLY}")
message(STATUS "ENABLE_LTO               ${ENABLE_LTO}")
message(STATUS "ENABLE_SNAPSHOT          ${ENABLE_SNAPSHOT}")
message(STATUS "ENABLE_MINIMAL           ${ENABLE_MINIMAL}")
message(STATUS "IOTJS_INCLUDE_MODULE     ${IOTJS_INCLUDE_MODULE}")
message(STATUS "IOTJS_EXCLUDE_MODULE     ${IOTJS_EXCLUDE_MODULE}")
message(STATUS "IOTJS_C_FLAGS            ${IOTJS_C_FLAGS}")
message(STATUS "IOTJS_LINK_FLAGS         ${IOTJS_LINK_FLAGS}")

# Collect all sources into LIB_IOTJS_SRC
file(GLOB LIB_IOTJS_SRC ${IOTJS_SOURCE_DIR}/*.c)
list(APPEND LIB_IOTJS_SRC
  ${IOTJS_SOURCE_DIR}/iotjs_js.c
  ${IOTJS_SOURCE_DIR}/iotjs_js.h
  ${IOTJS_MODULE_SRC}
  ${IOTJS_PLATFORM_SRC}
)

separate_arguments(EXTERNAL_INCLUDE_DIR)
separate_arguments(EXTERNAL_STATIC_LIB)
separate_arguments(EXTERNAL_SHARED_LIB)

set(IOTJS_INCLUDE_DIRS
  ${EXTERNAL_INCLUDE_DIR}
  ${ROOT_DIR}/include
  ${IOTJS_SOURCE_DIR}
  ${JERRY_PORT_DIR}/include
  ${JERRY_INCLUDE_DIR}
  ${HTTPPARSER_INCLUDE_DIR}
  ${TUV_INCLUDE_DIR}
)

set(IOTJS_CFLAGS ${IOTJS_CFLAGS} ${CFLAGS_COMMON})

# Configure the libiotjs.a
set(TARGET_LIB_IOTJS libiotjs)
add_library(${TARGET_LIB_IOTJS} STATIC ${LIB_IOTJS_SRC})
set_target_properties(${TARGET_LIB_IOTJS} PROPERTIES
  COMPILE_OPTIONS "${IOTJS_CFLAGS}"
  OUTPUT_NAME iotjs
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)
target_include_directories(${TARGET_LIB_IOTJS} PRIVATE ${IOTJS_INCLUDE_DIRS})
target_link_libraries(${TARGET_LIB_IOTJS}
  ${JERRY_LIBS}
  ${TUV_LIBS}
  libhttp-parser
  ${EXTERNAL_STATIC_LIB}
  ${EXTERNAL_SHARED_LIB}
)

if("${LIB_INSTALL_DIR}" STREQUAL "")
  set(LIB_INSTALL_DIR "lib")
endif()

if("${BIN_INSTALL_DIR}" STREQUAL "")
  set(BIN_INSTALL_DIR "bin")
endif()

install(TARGETS ${TARGET_LIB_IOTJS} DESTINATION ${LIB_INSTALL_DIR})

if(NOT BUILD_LIB_ONLY)

  if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(IOTJS_LINK_FLAGS "-Xlinker -map -Xlinker iotjs.map")
  else()
    set(IOTJS_LINK_FLAGS "-Xlinker -Map -Xlinker iotjs.map")
  endif()

  # Configure the iotjs executable
  set(TARGET_IOTJS iotjs)
  add_executable(${TARGET_IOTJS} ${ROOT_DIR}/iotjs_linux.c)
  set_target_properties(${TARGET_IOTJS} PROPERTIES
    COMPILE_OPTIONS "${IOTJS_CFLAGS}"
    LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${IOTJS_LINK_FLAGS}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )
  target_include_directories(${TARGET_IOTJS} PRIVATE ${IOTJS_INCLUDE_DIRS})
  target_link_libraries(${TARGET_IOTJS} ${TARGET_LIB_IOTJS})
  install(TARGETS ${TARGET_IOTJS} DESTINATION ${BIN_INSTALL_DIR})
endif()
