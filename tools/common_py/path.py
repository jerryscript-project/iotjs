# Copyright 2016-present Samsung Electronics Co., Ltd. and other contributors
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

""" common path for scripts """

from common_py.system.filesystem import FileSystem as fs

# Root directory for the project.
PROJECT_ROOT = fs.abspath(fs.join(fs.dirname(__file__), fs.pardir, fs.pardir))

# Source code directory.
SRC_ROOT = fs.join(PROJECT_ROOT, 'src')

# Root Build directory.
BUILD_ROOT = fs.join(PROJECT_ROOT, 'build')

# Root Build directory.
TOOLS_ROOT = fs.join(PROJECT_ROOT, 'tools')

# Root directory for dependencies.
DEPS_ROOT = fs.join(PROJECT_ROOT, 'deps')

# Root directory for test.
TEST_ROOT = fs.join(PROJECT_ROOT, 'test')

RUN_PASS_DIR = fs.join(TEST_ROOT, 'run_pass')

RUN_FAIL_DIR = fs.join(TEST_ROOT, 'run_fail')

RESOURCE_DIR = fs.join(TEST_ROOT, 'resources')

# Root directory for jerry script submodule.
JERRY_ROOT = fs.join(DEPS_ROOT, 'jerry')

# Root directory for libtuv submodule.
TUV_ROOT = fs.join(DEPS_ROOT, 'libtuv')

# Root directory for http-parser submodule.
HTTPPARSER_ROOT = fs.join(DEPS_ROOT, 'http-parser')

# checktest
CHECKTEST_PATH = fs.join(TOOLS_ROOT, 'check_test.js')

# Build configuration file path.
BUILD_CONFIG_PATH = fs.join(PROJECT_ROOT, 'build.config')
