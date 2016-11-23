#!/usr/bin/env python

# Copyright 2015-2016 Samsung Electronics Co., Ltd.
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


import argparse
import json
import sys
import re
import os

from js2c import js2c
from common_py import path
from common_py.system.filesystem import FileSystem as fs
from common_py.system.executor import Executor as ex
from common_py.system.platform import Platform

platform = Platform()


# Initialize build option.
def init_option():
    # Check config option.
    arg_config = filter(lambda x: x.startswith('--config='), sys.argv)

    config_path = path.BUILD_CONFIG_PATH

    if len(arg_config) != 0:
        config_path = arg_config[-1].split('=', 1)[1]

    # Read config file and apply it to argv.
    argv = []
    config = {}
    with open(config_path, 'r') as f:
        config = json.loads(f.read().encode('ascii'))
        config_option = config['build_option']
        for opt_key in config_option:
            opt_val = config_option[opt_key]
            if isinstance(opt_val, basestring) and opt_val != '':
                argv.append('--%s=%s' % (opt_key, opt_val))
            elif isinstance(opt_val, bool):
                if opt_val:
                    argv.append('--%s' % opt_key)
            elif isinstance(opt_val, int):
                argv.append('--%s=%s' % (opt_key, opt_val))
            elif isinstance(opt_val, list):
                for val in opt_val:
                    argv.append('--%s=%s' % (opt_key, val))

    # Apply command line argument to argv.
    argv = argv + ([arg for arg in sys.argv[1:]
                    if not arg.startswith('--config=')])

    # Prepare argument parser.
    parser = argparse.ArgumentParser()

    parser.add_argument('--buildtype',
                        choices=['debug', 'release'],
                        default='debug')

    parser.add_argument('--builddir', default=path.BUILD_ROOT)

    parser.add_argument('--clean', action='store_true')

    parser.add_argument('--buildlib', action='store_true')

    parser.add_argument('--target-arch',
                        choices=['arm', 'x86', 'i686', 'x86_64', 'x64'],
                        default=platform.arch())

    parser.add_argument('--target-os',
                        choices=['linux', 'darwin', 'osx', 'nuttx'],
                        default=platform.os())

    parser.add_argument('--target-board', default='')

    parser.add_argument('--cmake-param', action='append')

    parser.add_argument('--compile-flag', action='append')

    parser.add_argument('--link-flag', action='append')

    parser.add_argument('--external-include-dir', action='append')

    parser.add_argument('--external-static-lib', action='append')

    parser.add_argument('--external-shared-lib', action='append')

    parser.add_argument('--iotjs-include-module', action='append')

    parser.add_argument('--iotjs-exclude-module', action='append')

    parser.add_argument('--iotjs-minimal-profile', action='store_true')

    parser.add_argument('--jerry-cmake-param', action='append')

    parser.add_argument('--jerry-compile-flag', action='append')

    parser.add_argument('--jerry-link-flag', action='append')

    parser.add_argument('--jerry-lto', action='store_true')

    parser.add_argument('--jerry-heap-section', action='store')

    parser.add_argument('--jerry-heaplimit', type=int)

    parser.add_argument('--jerry-memstat', action='store_true')

    parser.add_argument('--no-init-submodule', action='store_true')

    parser.add_argument('--no-check-valgrind', action='store_true')

    parser.add_argument('--no-check-test', action='store_true')

    parser.add_argument('--no-parallel-build', action='store_true')

    parser.add_argument('--nuttx-home', default='')

    parser.add_argument('--no-snapshot', action='store_true')

    # parse argument.
    option = parser.parse_args(argv)
    option.config = config

    return option


def adjust_option(option):
    if option.target_os.lower() == 'nuttx':
        option.buildlib = True
        if option.nuttx_home == '':
            ex.fail('--nuttx-home needed for nuttx target')
        else:
            option.nuttx_home = fs.abspath(option.nuttx_home)
            if not fs.exists(option.nuttx_home):
                ex.fail('--nuttx-home %s not exists' % option.nuttx_home)
    if option.target_arch == 'x86':
        option.target_arch = 'i686'
    if option.target_arch == 'x64':
        option.target_arch = 'x86_64'
    if option.target_board == 'rpi2':
        option.no_check_valgrind = True
    if option.cmake_param is None:
        option.cmake_param = []
    if option.compile_flag is None:
        option.compile_flag = []
    if option.link_flag is None:
        option.link_flag = []
    if option.external_include_dir is None:
        option.external_include_dir = []
    if option.external_static_lib is None:
        option.external_static_lib = []
    if option.external_shared_lib is None:
        option.external_shared_lib = []
    if option.iotjs_include_module is None:
        option.iotjs_include_module = []
    if option.iotjs_exclude_module is None:
        option.iotjs_exclude_module = []
    if option.iotjs_minimal_profile:
        option.no_check_test = True
    if option.jerry_cmake_param is None:
        option.jerry_cmake_param = []
    if option.jerry_compile_flag is None:
        option.jerry_compile_flag = []
    if option.jerry_link_flag is None:
        option.jerry_link_flag = []


def print_build_option(option):
    print '================================================='
    option_vars = vars(option)
    for opt in option_vars:
        print ' --%s: %s ' % (opt, option_vars[opt])
    print


def set_global_vars(option):
    global host_tuple
    host_tuple = '%s-%s' % (platform.arch(), platform.os())

    global host_build_root
    host_build_root = fs.join(path.PROJECT_ROOT,
                              option.builddir,
                              'host',
                              host_tuple,
                              option.buildtype)

    global host_build_bins
    host_build_bins = fs.join(host_build_root, 'bin')

    global target_tuple
    target_tuple = '%s-%s' % (option.target_arch, option.target_os)

    global build_root
    build_root = fs.join(path.PROJECT_ROOT,
                         option.builddir,
                         target_tuple,
                         option.buildtype)

    global build_bins
    build_bins = fs.join(build_root, 'bin')

    global build_libs
    build_libs = fs.join(build_root, 'lib')

    global build_jerry_deps
    build_jerry_deps = fs.join(build_root, 'deps', 'jerry')

    global build_jerry_deps_libs
    build_jerry_deps_libs = fs.join(build_jerry_deps, "lib")

    global libtuv_output_path
    libtuv_output_path = fs.join(build_libs, 'libtuv.a')

    global libhttpparser_output_path
    libhttpparser_output_path = fs.join(build_libs, 'libhttpparser.a')

    global jerry_output_path
    jerry_output_path = fs.join(host_build_bins, 'jerry')

    global libjerry_output_path
    libjerry_output_path = fs.join(build_libs, 'libjerrycore.a')

    global iotjs_output_path
    iotjs_output_path = fs.join(build_bins, 'iotjs')

    global libiotjs_output_path
    libiotjs_output_path = fs.join(build_libs, 'libiotjs.a')

    global libjerry_libc_output_path
    libjerry_libc_output_path = fs.join(build_jerry_deps_libs,
                                        'libjerry-libc.a')

    global libjerry_libm_output_path
    libjerry_libm_output_path = fs.join(build_jerry_deps_libs,
                                        'libjerry-libm.a')

    global cmake_toolchain_file
    cmake_toolchain_file = fs.join(path.PROJECT_ROOT, 'cmake', 'config',
                                   target_tuple + '.cmake')

    global host_cmake_toolchain_file
    host_cmake_toolchain_file = fs.join(path.PROJECT_ROOT, 'cmake', 'config',
                                        host_tuple + '.cmake')

    global platform_descriptor
    platform_descriptor = '%s-%s' % (option.target_arch, option.target_os)


def create_build_directories(option):
    fs.maybe_make_directory(build_root)
    fs.maybe_make_directory(build_bins)
    fs.maybe_make_directory(build_libs)
    fs.maybe_make_directory(host_build_root)
    fs.maybe_make_directory(host_build_bins)


def print_progress(msg):
    print '==> %s' % msg
    print


def init_submodule():
    ex.check_run_cmd('git', ['submodule', 'init'])
    ex.check_run_cmd('git', ['submodule', 'update'])


def inflate_cmake_option(cmake_opt, option, for_jerry=False):
    # compile flags
    compile_flags = []

    config_compile_flags = option.config['compile_flags']
    compile_flags += config_compile_flags['os'][option.target_os]
    compile_flags += config_compile_flags['arch'][option.target_arch]
    compile_flags += config_compile_flags['buildtype'][option.buildtype]
    if option.target_board:
        compile_flags += config_compile_flags['board'][option.target_board]

    compile_flags += option.compile_flag
    compile_flags += option.jerry_compile_flag if for_jerry else []

    cmake_opt.append('-DCMAKE_C_FLAGS=' + ' '.join(compile_flags))

    # link flags
    link_flags = []

    config_link_flags = option.config['link_flags']
    link_flags += config_link_flags['os'][option.target_os]

    link_flags += option.link_flag
    link_flags += option.jerry_link_flag if for_jerry else []

    if option.jerry_lto:
        link_flags.append('-flto')

    cmake_opt.append('-DCMAKE_EXE_LINKER_FLAGS=' + ' '.join(link_flags))

    # external include dir
    include_dirs = []
    if option.target_os == 'nuttx' and option.nuttx_home:
        include_dirs.append('%s/include' % option.nuttx_home)
        if option.target_board == 'stm32f4dis':
            include_dirs.append('%s/arch/arm/src/stm32' % option.nuttx_home)
    include_dirs.extend(option.external_include_dir)
    cmake_opt.append('-DEXTERNAL_INCLUDE_DIR=' + ' '.join(include_dirs))

    # set target board
    if option.target_board == 'stm32f4dis':
        cmake_opt.append('-DCMAKE_TARGET_BOARD=STM32')

def build_tuv(option):
    # Check if libtuv submodule exists.
    if not fs.exists(path.TUV_ROOT):
        ex.fail('libtuv submodule not exists!')

    # Move working directory to libtuv build directory.
    build_home = fs.join(build_root, 'deps', 'libtuv')
    fs.maybe_make_directory(build_home)
    fs.chdir(build_home)

    # Set tuv cmake option.
    cmake_opt = [path.TUV_ROOT]
    cmake_opt.append('-DCMAKE_TOOLCHAIN_FILE=' +
                     fs.join(path.TUV_ROOT,
                             'cmake', 'config',
                             'config_' + target_tuple + '.cmake'))
    cmake_opt.append('-DCMAKE_BUILD_TYPE=' + option.buildtype)
    cmake_opt.append('-DTARGET_PLATFORM=' + target_tuple)
    cmake_opt.append('-DLIBTUV_CUSTOM_LIB_OUT=' + build_home)
    cmake_opt.append('-DBUILDTESTER=no')
    cmake_opt.append('-DBUILDAPIEMULTESTER=no')

    if option.target_os == 'nuttx':
        cmake_opt.append('-DTARGET_SYSTEMROOT=' + option.nuttx_home)

    if option.target_board:
        cmake_opt.append('-DTARGET_BOARD=' + option.target_board)

    # inflate cmake option.
    inflate_cmake_option(cmake_opt, option)

    # Run cmake
    ex.check_run_cmd('cmake', cmake_opt)

    # Run make
    make_opt = []
    if not option.no_parallel_build:
        make_opt.append('-j')

    ex.check_run_cmd('make', make_opt)

    # libtuv output
    output = fs.join(build_home, 'libtuv.a')
    if not fs.exists(output):
        ex.fail('libtuv build failed - target not produced.')

    # copy output to libs directory
    fs.maybe_make_directory(build_libs)
    fs.copy(output, libtuv_output_path)

    return True


# Build jerry executable to generate snapshot.
# Jerry executable must be runnable from build HOST machine.
def build_jerry(option):
    # Check if JerryScript submodule exists.
    if not fs.exists(path.JERRY_ROOT):
        ex.fail('JerryScript submodule not exists!')

    # Move working directory to JerryScript build directory.
    build_home = fs.join(host_build_root, 'deps', 'jerry')
    fs.maybe_make_directory(build_home)
    fs.chdir(build_home)

    # Set JerryScript cmake option.
    cmake_opt = [path.JERRY_ROOT]

    cmake_opt.append('-DCMAKE_TOOLCHAIN_FILE=' + host_cmake_toolchain_file)

    if option.buildtype == 'debug':
        cmake_opt.append('-DCMAKE_BUILD_TYPE=Debug')

    # Turn off LTO for jerry bin to save build time.
    cmake_opt.append('-DENABLE_LTO=OFF')

    # Turn on snapshot
    if not option.no_snapshot:
        cmake_opt.append('-DFEATURE_SNAPSHOT_SAVE=ON')

    # Run cmake.
    ex.check_run_cmd('cmake', cmake_opt)

    target_jerry = {
        'target_name': 'jerry',
        'output_path': fs.join(build_home, 'bin/jerry')
    }

    # Make option.
    make_opt = ['-C', build_home]
    if not option.no_parallel_build:
        make_opt.append('-j')

    # Run make for a target.
    ex.check_run_cmd('make', make_opt)

    # Check output
    output = target_jerry['output_path']
    if not fs.exists(output):
        print output
        ex.fail('JerryScript build failed - target not produced.')

    # copy
    fs.copy(output, jerry_output_path)

    return True


def build_libjerry(option):
    # Check if JerryScript submodule exists.
    if not fs.exists(path.JERRY_ROOT):
        ex.fail('JerryScript submodule not exists!')

    # Move working directory to JerryScript build directory.
    build_home = fs.join(build_root, 'deps', 'jerry')
    fs.maybe_make_directory(build_home)
    fs.chdir(build_home)

    # Set JerryScript cmake option.
    cmake_opt = [path.JERRY_ROOT]

    cmake_opt.append('-DCMAKE_TOOLCHAIN_FILE=' + cmake_toolchain_file)

    if option.buildtype == 'debug':
        cmake_opt.append('-DCMAKE_BUILD_TYPE=Debug')
        cmake_opt.append('-DFEATURE_ERROR_MESSAGES=On')

    if option.target_os == 'nuttx':
        cmake_opt.append('-DEXTERNAL_LIBC_INTERFACE=' +
                         fs.join(option.nuttx_home, 'include'))
        if option.target_arch == 'arm':
            cmake_opt.append('-DEXTERNAL_CMAKE_SYSTEM_PROCESSOR=arm')

    if option.target_os == 'linux':
        cmake_opt.append('-DJERRY_LIBC=OFF')
        cmake_opt.append('-DJERRY_LIBM=OFF')

    # --jerry-heaplimit
    if option.jerry_heaplimit:
        cmake_opt.append('-DMEM_HEAP_SIZE_KB=' +
                         str(option.jerry_heaplimit))

    # --jerry-heap-section
    if option.jerry_heap_section:
        cmake_opt.append('-DJERRY_HEAP_SECTION_ATTR=' +
                         str(option.jerry_heap_section))

    # --jerry-lto
    cmake_opt.append('-DENABLE_LTO=%s' % ('ON' if option.jerry_lto else 'OFF'))

    if option.jerry_memstat:
        cmake_opt.append('-DFEATURE_MEM_STATS=ON')

    # Turn on snapshot
    cmake_opt.append('-DFEATURE_SNAPSHOT_SAVE=OFF')
    if not option.no_snapshot:
        cmake_opt.append('-DFEATURE_SNAPSHOT_EXEC=ON')

    # --jerry-cmake-param
    cmake_opt += option.jerry_cmake_param

    # inflate cmake option.
    inflate_cmake_option(cmake_opt, option, for_jerry=True)

    # Run cmake.
    ex.check_run_cmd('cmake', cmake_opt)

    # make target - libjerry
    target_libjerry_name = 'jerry-core'
    target_libjerry = {
        'target_name': target_libjerry_name,
        'output_path': fs.join(build_home, 'lib',
                               'lib%s.a' % target_libjerry_name),
        'dest_path': libjerry_output_path
    }

    targets = []
    targets.append(target_libjerry)

    # make the target.
    for target in targets:
        # Make option.
        make_opt = ['-C', build_home, target['target_name']]
        if not option.no_parallel_build:
            make_opt.append('-j')

        # Run make for a target.
        ex.check_run_cmd('make', make_opt)

        # Check output
        output = target['output_path']
        if not fs.exists(output):
            print output
            ex.fail('JerryScript build failed - target not produced.')

        # copy
        fs.copy(output, target['dest_path'])

    return True


def build_libhttpparser(option):
    # Check if JerryScript submodule exists.
    if not fs.exists(path.HTTPPARSER_ROOT):
        ex.fail('libhttpparser submodule not exists!')
        return False

    # Move working directory to JerryScript build directory.
    build_home = fs.join(build_root, 'deps', 'httpparser')
    fs.maybe_make_directory(build_home)
    fs.chdir(build_home)

    # Set JerryScript cmake option.
    cmake_opt = [path.HTTPPARSER_ROOT]

    cmake_opt.append('-DCMAKE_TOOLCHAIN_FILE=' + cmake_toolchain_file)
    cmake_opt.append('-DBUILDTYPE=' + option.buildtype.capitalize())

    if option.target_os == 'nuttx':
        cmake_opt.append('-DNUTTX_HOME=' + option.nuttx_home)
        cmake_opt.append('-DOS=NUTTX')
    if option.target_os == 'linux':
        cmake_opt.append('-DOS=LINUX')

    # inflate cmake option.
    inflate_cmake_option(cmake_opt, option)

    # Run cmake.
    ex.check_run_cmd('cmake', cmake_opt)

    # Set make option.
    make_opt = []
    if not option.no_parallel_build:
        make_opt.append('-j')

    # Run make
    ex.check_run_cmd('make', make_opt)

    # Output
    output = fs.join(build_home, 'libhttpparser.a')
    if not fs.exists(output):
            ex.fail('libhttpparser build failed - target not produced.')

    # copy
    fs.copy(output, libhttpparser_output_path)

    return True

def analyze_module_dependency(option):

    def print_warn(fmt, arg):
        print fmt % arg
        ex.fail('Failed to analyze module dependency')

    for name in option.config['module']['always']:
        if name in option.iotjs_include_module:
            print_warn('Module \"%s\" is already included', name)
        else:
            option.iotjs_include_module.append(name)

    if not option.iotjs_minimal_profile:
        for name in option.config['module']['optional']:
            if name in option.iotjs_include_module:
                print_warn('Module \"%s\" is already included', name)
            else:
                option.iotjs_include_module.append(name)

    for name in option.config['module']['exclude']:
        if name in option.iotjs_exclude_module:
            print_warn('Module \"%s\" is already excluded', name)
        else:
            option.iotjs_exclude_module.append(name)

    analyze_queue = set()
    for name in option.iotjs_include_module:
        analyze_queue.add(name)
    for name in option.iotjs_exclude_module:
        if name in option.config['module']['always']:
            print_warn('Cannot exclude mandatory module \"%s\"', name)
        else:
            if name in analyze_queue:
                analyze_queue.remove(name)
            else:
                print_warn('Cannot find module \"%s\" to exclude', name)

    js_modules = { 'iotjs', 'native' }
    native_modules = { 'process' }
    while len(analyze_queue) != 0:
        item = analyze_queue.pop()
        js_modules.add(item)

        content = open(fs.join(path.PROJECT_ROOT,
                               'src', 'js', item + '.js')).read()

        re_js_module = 'require\([\'\"](.*)[\'\"]\)'
        for js_module in re.findall(re_js_module, content):
            if js_module in option.iotjs_exclude_module:
                print_warn('Cannot exclude \"%s\" since \"%s\" requires it',
                           (js_module, item))
            if js_module not in js_modules:
                analyze_queue.add(js_module)

        re_native_module = 'process.binding\(process.binding.(.*)\)'
        for native_module in re.findall(re_native_module, content):
            native_modules.add(native_module)

    js_modules.remove('native')

    option.js_modules = js_modules
    option.native_modules = native_modules

    print 'Building js modules: %s\nBuilding native modules: %s' \
          % (', '.join(js_modules), ', '.join(native_modules))
    print

    return True


def build_iotjs(option):
    # Run js2c
    fs.chdir(path.TOOLS_ROOT)
    js2c(option.buildtype, option.no_snapshot, option.js_modules,
         jerry_output_path)

    # Move working directory to IoT.js build directory.
    build_home = fs.join(build_root, 'iotjs')
    fs.maybe_make_directory(build_home)
    fs.chdir(build_home)

    # Set JerryScript cmake option.
    cmake_opt = [path.PROJECT_ROOT]
    cmake_opt.append('-DCMAKE_TOOLCHAIN_FILE=' + cmake_toolchain_file)
    cmake_opt.append('-DCMAKE_BUILD_TYPE=' + option.buildtype.capitalize())
    cmake_opt.append('-DTARGET_OS=' + option.target_os)
    cmake_opt.append('-DPLATFORM_DESCRIPT=' + platform_descriptor)

    # IoT.js module list
    cmake_opt.append('-DIOTJS_MODULES=' + (' ').join(option.native_modules))

    if not option.no_snapshot:
        option.compile_flag.append('-DENABLE_SNAPSHOT')

    if option.target_os == 'nuttx':
        cmake_opt.append('-DNUTTX_HOME=' + option.nuttx_home)
        option.buildlib = True

    # --build-lib
    if option.buildlib:
        cmake_opt.append('-DBUILD_TO_LIB=YES')

    # --cmake-param
    cmake_opt += option.cmake_param

    # --external_static_lib
    cmake_opt.append('-DEXTERNAL_STATIC_LIB=' +
                    ' '.join(option.external_static_lib))

    # --external_shared_lib
    config_shared_libs = option.config['shared_libs']
    option.external_shared_lib += config_shared_libs['os'][option.target_os]
    cmake_opt.append('-DEXTERNAL_SHARED_LIB=' +
                    ' '.join(option.external_shared_lib))

    # inflate cmake option
    inflate_cmake_option(cmake_opt, option)

    # Run cmake.
    ex.check_run_cmd('cmake', cmake_opt)

    # Set make option.
    make_opt = []
    if not option.no_parallel_build:
        make_opt.append('-j')

    # Run make
    ex.check_run_cmd('make', make_opt)

    # Output
    output = fs.join(build_home,
                     'liblibiotjs.a' if option.buildlib else 'iotjs')

    if not fs.exists(output):
            ex.fail('IoT.js build failed - target not produced.')

    # copy
    dest_path = libiotjs_output_path if option.buildlib else iotjs_output_path
    fs.copy(output, dest_path)

    return True


def run_checktest(option):
    checktest_quiet = 'yes'
    if os.getenv('TRAVIS') == "true":
        checktest_quiet = 'no'

    # iot.js executable
    iotjs = fs.join(build_root, 'iotjs', 'iotjs')
    fs.chdir(path.PROJECT_ROOT)
    code = ex.run_cmd(iotjs, [path.CHECKTEST_PATH,
                              '--',
                              'quiet='+checktest_quiet])
    if code != 0:
        ex.fail('Failed to pass unit tests')
    if not option.no_check_valgrind:
        code = ex.run_cmd('valgrind', ['--leak-check=full',
                                       '--error-exitcode=5',
                                       '--undef-value-errors=no',
                                       iotjs,
                                       path.CHECKTEST_PATH,
                                       '--',
                                       'quiet='+checktest_quiet])
        if code == 5:
            ex.fail('Failed to pass valgrind test')
        if code != 0:
            ex.fail('Failed to pass unit tests in valgrind environment')
    return True


# Initialize build option object.
option = init_option()

adjust_option(option)

print_build_option(option)

set_global_vars(option)

# clean build directory.
if option.clean:
    print_progress('Clear build directory')
    fs.rmtree(build_root)
    fs.rmtree(host_build_root)

create_build_directories(option)

# Analyze module dependency
print_progress('Analyze module dependency')
if not analyze_module_dependency(option):
    ex.fail('Failed to analyze module dependency')

# Perform init-submodule.
print_progress('Initialize submodules')
if not option.no_init_submodule:
    init_submodule()

# make build directory.
print_progress('Create build directory')
fs.maybe_make_directory(build_root)

# build tuv.
print_progress('Build libtuv')
if not build_tuv(option):
    ex.fail('Failed to build libtuv')

# build jerry.
print_progress('Build JerryScript')
if not build_jerry(option):
    ex.fail('Failed to build jerry')
if not build_libjerry(option):
    ex.fail('Failed to build libjerry')

# build httpparser
print_progress('Build libhttpparser')
if not build_libhttpparser(option):
    ex.fail('Failed to build libhttpparser')

# build iotjs.
print_progress('Build IoT.js')
if not build_iotjs(option):
    ex.fail('Failed to build IoT.js')

# check unit test.
if not option.no_check_test:
    print_progress('Check unit tests')
    # Run check test when target is host.
    if (option.target_os != platform.os() or
            option.target_arch != platform.arch()):
        print "Skip unit tests - target is not host"
        print
    elif option.buildlib:
        print "Skip unit tests - build target is library"
        print
    else:
        if not run_checktest(option):
            ex.fail('Failed to pass unit tests')


print
print "%sIoT.js Build Succeeded!!%s" % (ex._TERM_GREEN, ex._TERM_EMPTY)
print

sys.exit(0)
