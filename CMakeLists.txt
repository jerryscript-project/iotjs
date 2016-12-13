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

include(cmake/config.cmake)
include(cmake/libtuv.cmake)
include(cmake/jerry.cmake)
include(cmake/http-parser.cmake)
include(cmake/iotjs.cmake)
