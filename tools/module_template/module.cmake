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

# General variables usable from IoT.js cmake:
# - TARGET_ARCH - the target architecture (as specified during cmake step)
# - TARGET_BOARD - the target board(/device)
# - TARGET_OS - the target operating system
#
# Module related variables usable from IoT.js cmake:
# - MODULE_DIR - the modules root directory
# - MODULE_BINARY_DIR - the build directory for the current module
# - MODULE_LIBS - list of libraries to use during linking (set this)
set(MODULE_NAME "$MODULE_NAME$")

# DO NOT include the source files which are already in the modules.json file.

# If the module builds its own files into a lib please use the line below.
# Note: the subdir 'lib' should contain the CMakeLists.txt describing how the
#  module should be built.
#add_subdirectory(${MODULE_DIR}/lib/ ${MODULE_BINARY_DIR}/${MODULE_NAME})

# If you wish to link external libraries please add it to
# the MODULE_LIBS list.
#
# IMPORTANT!
#  if the module builds its own library that should also be specified!
#
# Example (to add the 'demo' library for linking):
#
#  list(APPEND MODULE_LIBS demo)
