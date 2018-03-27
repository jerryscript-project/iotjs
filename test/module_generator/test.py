#!/usr/bin/env python

# Copyright 2019-present Samsung Electronics Co., Ltd. and other contributors
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

import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..', 'tools'))
from common_py import path
from common_py.system.executor import Executor as ex
from common_py.system.filesystem import FileSystem as fs

module_generator_dir = fs.join(path.TOOLS_ROOT, 'module_generator')
generator_script = fs.join(path.TOOLS_ROOT, 'iotjs-generate-module.py')
build_script = fs.join(path.TOOLS_ROOT, 'build.py')

def print_green(msg):
    print ('\033[1;32m{}\033[00m'.format(msg))

def print_blue(msg):
    print ('\033[1;34m{}\033[00m'.format(msg))

def test_c():
    test_dir = fs.join(os.path.dirname(__file__), 'test_c')
    test_c = fs.join(test_dir, 'test.c')

    # Compile test.c and make a static library
    print_blue('Compile C test module.')
    ex.check_run_cmd_output('cc', ['-c', test_c, '-o', test_dir + '/test.o'])
    ex.check_run_cmd_output('ar', ['-cr', test_dir + '/libtest.a',
                     test_dir + '/test.o'])

    # Generate test_module
    print_blue('Generate binding for C test module.')
    ex.check_run_cmd_output(generator_script, [test_dir, 'c'])

    # Build iotjs
    print_blue('Build IoT.js.')
    module_dir = fs.join(module_generator_dir, 'output', 'test_c_module')
    args = [
    '--external-module=' + module_dir,
    '--cmake-param=-DENABLE_MODULE_TEST_C_MODULE=ON',
    '--jerry-profile=es2015-subset',
    '--clean'
    ]
    ex.check_run_cmd_output(build_script, args)

    run_test_js(test_dir)

    print_green('C test succeeded.')

def test_cpp():
    test_dir = fs.join(os.path.dirname(__file__), 'test_cpp')
    test_cpp = fs.join(test_dir, 'test.cpp')

    # Compile test.c and make a static library
    print_blue('Compile C++ test module.')
    ex.check_run_cmd_output('c++', ['-c', test_cpp, '-o', test_dir + '/test.o'])
    ex.check_run_cmd_output('ar', ['-cr', test_dir + '/libtest.a',
                     test_dir + '/test.o'])

    # Generate test_module
    print_blue('Generate binding for C++ test module.')
    ex.check_run_cmd_output(generator_script, [test_dir, 'c++'])

    # Build iotjs
    print_blue('Build IoT.js.')
    module_dir = fs.join(module_generator_dir, 'output', 'test_cpp_module')
    args = [
    '--external-module=' + module_dir,
    '--cmake-param=-DENABLE_MODULE_TEST_CPP_MODULE=ON',
    '--jerry-profile=es2015-subset',
    '--clean'
    ]
    ex.check_run_cmd_output(build_script, args)

    run_test_js(test_dir)

    print_green('C++ test succeeded.')

def run_test_js(test_dir):
    # Run test.js
    print_blue('Run test.js file.')
    binary = fs.join(path.BUILD_ROOT, 'x86_64-linux', 'debug', 'bin', 'iotjs')
    test_js = fs.join(test_dir, 'test.js')
    ex.check_run_cmd_output(binary, [test_js])


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('-x', choices=['c', 'c++'], action='append',
        default=[], help='Specify language.')
    args = parser.parse_args()

    if not args.x:
        test_c()
        test_cpp()
    if 'c' in args.x:
        test_c()
    if 'c++' in args.x:
        test_cpp()
