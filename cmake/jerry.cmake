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

set(JERRY_ROOT ${DEP_ROOT}/jerry)
set(JERRY_CORE_ROOT ${JERRY_ROOT}/jerry-core)
set(JERRY_PORT_ROOT ${JERRY_ROOT}/targets/default)
set(JERRY_INCDIR ${JERRY_CORE_ROOT} ${JERRY_PORT_ROOT})
set(JERRY_BIN ${BIN_ROOT}/deps/jerry)
set(JERRY_LIB ${LIB_ROOT}/libjerrycore.a)
