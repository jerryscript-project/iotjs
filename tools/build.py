#!/usr/bin/env python

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


import argparse
import json
import os
import shutil
import subprocess
import sys
from os import path
from check_tidy import check_tidy


TERM_RED = "\033[1;31m"
TERM_YELLOW = "\033[1;33m"
TERM_GREEN = "\033[1;32m"
TERM_BLUE = "\033[1;34m"
TERM_EMPTY = "\033[0m"


def join_path(pathes):
    return path.abspath(reduce(lambda x, y: path.join(x, y), pathes))


# Path for this script file.
 # should be <project_home>/tools.
SCRIPT_PATH = path.dirname(path.abspath(__file__))

# Home directory for the project.
PROJECT_HOME = join_path([SCRIPT_PATH, '../'])

# Build directory.
BUILD_HOME = join_path([PROJECT_HOME, 'build/'])

# Root directory for dependencies.
DEPS_ROOT = join_path([PROJECT_HOME, 'deps/'])

# Root directory for jerry script submodule.
JERRY_ROOT = join_path([DEPS_ROOT, 'jerry/'])

# Root directory for libtuv submodule.
TUV_ROOT = join_path([DEPS_ROOT, 'libtuv/'])

# Root directory for http-parser submodule.
HTTPPARSER_ROOT = join_path([DEPS_ROOT, 'http-parser/'])

# checktest
CHECKTEST = join_path([SCRIPT_PATH, 'check_test.py'])

# Default build configuration file path.
DEFAULT_CONFIG_PATH = join_path([PROJECT_HOME, '.build.default.config'])
WORKING_CONFIG_PATH = join_path([PROJECT_HOME, 'build.config'])

def mkdir(path):
    if not os.path.exists(path):
        os.makedirs(path)


def cmd_line(cmd, args = []):
    return ' '.join([cmd] + args)


def print_cmd_line(cmd, args):
    print "%s%s%s" % (TERM_BLUE, cmd_line(cmd, args), TERM_EMPTY)
    print


def fail(msg):
    print
    print "%s%s%s" % (TERM_RED, msg, TERM_EMPTY)
    print
    exit(1)


def run_cmd(cmd, args = []):
    print_cmd_line(cmd, args)
    return subprocess.call([cmd] + args)


def check_run_cmd(cmd, args = []):
    retcode = run_cmd(cmd, args)
    if retcode != 0:
        fail("[Failed - %d] %s" % (retcode, cmd_line(cmd, args)))


# Retrieve host OS name.
def sys_os():
    os_name, _, _, _, _ = os.uname()
    return os_name.lower()


# Retrieve host arch name.
def sys_arch():
    _, _, _, _, arch = os.uname()
    return arch.lower()


# Initialize build option.
def init_option():
    # Check config option.
    arg_config = filter(lambda x: x.startswith('--config='), sys.argv)

    if not os.path.exists(WORKING_CONFIG_PATH):
        shutil.copy(DEFAULT_CONFIG_PATH, WORKING_CONFIG_PATH)

    config_path = WORKING_CONFIG_PATH

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

    # Prepare aguemnt parser.
    parser = argparse.ArgumentParser()

    parser.add_argument('--buildtype',
                        choices=['debug', 'release'],
                        default='debug')

    parser.add_argument('--builddir', default=BUILD_HOME)

    parser.add_argument('--clean', action='store_true')

    parser.add_argument('--buildlib', action='store_true')

    parser.add_argument('--target-arch',
                        choices=['arm', 'mips', 'x86', 'i686', 'x86_64', 'x64'],
                        default=sys_arch())

    parser.add_argument('--target-os',
                        choices=['linux', 'darwin', 'osx', 'nuttx', 'openwrt'],
                        default=sys_os())

    parser.add_argument('--target-board', default='')

    parser.add_argument('--cmake-param', action='append')

    parser.add_argument('--compile-flag', action='append')

    parser.add_argument('--link-flag', action='append')

    parser.add_argument('--external-include-dir', action='append')

    parser.add_argument('--external-static-lib', action='append')

    parser.add_argument('--external-shared-lib', action='append')

    parser.add_argument('--jerry-cmake-param', action='append')

    parser.add_argument('--jerry-compile-flag', action='append')

    parser.add_argument('--jerry-link-flag', action='append')

    parser.add_argument('--jerry-lto', action='store_true')

    parser.add_argument('--jerry-heaplimit', type=int)

    parser.add_argument('--jerry-memstat', action='store_true')

    parser.add_argument('--no-init-submodule', action='store_true')

    parser.add_argument('--no-check-tidy', action='store_true')

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
        option.buildlib = True;
        if option.nuttx_home == '':
            fail('--nuttx-home needed for nuttx target')
        else:
            option.nuttx_home = path.abspath(option.nuttx_home)
            if not path.exists(option.nuttx_home):
                fail('--nuttx-home %s not exists' % option.nuttx_home)
    if option.target_arch == 'x86':
        option.target_arch = 'i686'
    if option.target_arch == 'x64':
        option.target_arch = 'x86_64'
    if option.cmake_param == None:
        option.cmake_param = []
    if option.compile_flag == None:
        option.compile_flag = []
    if option.link_flag == None:
        option.link_flag = []
    if option.external_include_dir == None:
        option.external_include_dir = []
    if option.external_static_lib == None:
        option.external_static_lib = []
    if option.external_shared_lib == None:
        option.external_shared_lib = []
    if option.jerry_cmake_param == None:
        option.jerry_cmake_param = []
    if option.jerry_compile_flag == None:
        option.jerry_compile_flag = []
    if option.jerry_link_flag == None:
        option.jerry_link_flag = []


def print_build_option(option):
    print '================================================='
    option_vars = vars(option)
    for opt in option_vars:
        print ' --%s: %s '% (opt, option_vars[opt])
    print


def set_global_vars(option):
    global host_tuple
    host_tuple = '%s-%s' % (sys_arch(), sys_os())

    global host_build_root
    host_build_root = join_path([PROJECT_HOME,
                                 option.builddir,
                                 host_tuple,
                                 option.buildtype])

    global host_build_bins
    host_build_bins = join_path([host_build_root, 'bin'])

    global target_tuple
    target_tuple = '%s-%s' % (option.target_arch, option.target_os)

    global build_root
    build_root = join_path([PROJECT_HOME,
                            option.builddir,
                            target_tuple,
                            option.buildtype])

    global build_bins
    build_bins = join_path([build_root, 'bin'])

    global build_libs
    build_libs = join_path([build_root, 'lib'])

    global libtuv_output_path
    libtuv_output_path = join_path([build_libs, 'libtuv.a'])

    global libhttpparser_output_path
    libhttpparser_output_path = join_path([build_libs, 'libhttpparser.a'])

    global jerry_output_path
    jerry_output_path = join_path([host_build_bins, 'jerry'])

    global libjerry_output_path
    libjerry_output_path = join_path([build_libs, 'libjerrycore.a'])

    global libfdlibm_output_path
    libfdlibm_output_path = join_path([build_libs, 'libfdlibm.a'])

    global iotjs_output_path
    iotjs_output_path = join_path([build_bins, 'iotjs'])

    global libiotjs_output_path
    libiotjs_output_path = join_path([build_libs, 'libiotjs.a'])

    global cmake_toolchain_file
    cmake_toolchain_file = join_path([PROJECT_HOME, 'cmake', 'config',
                                      target_tuple + '.cmake'])

    global host_cmake_toolchain_file
    host_cmake_toolchain_file = join_path([PROJECT_HOME, 'cmake', 'config',
                                           host_tuple + '.cmake'])

    global platform_descriptor
    platform_descriptor = '%s-%s' % (option.target_arch, option.target_os)

def create_build_directories(option):
    mkdir(build_root)
    mkdir(build_bins)
    mkdir(build_libs)
    mkdir(host_build_root)
    mkdir(host_build_bins)


def print_progress(msg):
    print '==> %s' % msg
    print


def init_submodule():
    check_run_cmd('git', ['submodule', 'init']);
    check_run_cmd('git', ['submodule', 'update']);


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
    cmake_opt.append('-DCMAKE_CXX_FLAGS=' + ' '.join(compile_flags))


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
        include_dirs.append('%s/include/cxx' % option.nuttx_home)
    include_dirs.extend(option.external_include_dir)
    cmake_opt.append('-DEXTERNAL_INCLUDE_DIR=' + ' '.join(include_dirs))


def build_tuv(option):
    # Check if libtuv submodule exists.
    if not os.path.exists(TUV_ROOT):
        fail('libtuv submodule not exists!')

    # Move working directory to libtuv build directory.
    build_home = join_path([build_root, 'deps', 'libtuv'])
    mkdir(build_home)
    os.chdir(build_home)

    # Set tuv cmake option.
    cmake_opt = [TUV_ROOT]
    cmake_opt.append('-DCMAKE_TOOLCHAIN_FILE=' +
                     join_path([TUV_ROOT,
                                'cmake', 'config',
                                'config_' + target_tuple + '.cmake']))
    cmake_opt.append('-DCMAKE_BUILD_TYPE=' + option.buildtype)
    cmake_opt.append('-DTARGET_PLATFORM=' + target_tuple)
    cmake_opt.append('-DLIBTUV_CUSTOM_LIB_OUT=' + build_home)

    if option.target_os == 'nuttx':
        cmake_opt.append('-DTARGET_SYSTEMROOT=' + option.nuttx_home);

    # inflate cmake option.
    inflate_cmake_option(cmake_opt, option)

    # Run cmake
    check_run_cmd('cmake', cmake_opt)

    # Run make
    make_opt = []
    if not option.no_parallel_build:
        make_opt.append('-j')

    check_run_cmd('make', make_opt)

    # libtuv output
    output = join_path([build_home, 'libtuv.a'])
    if not os.path.exists(output):
        fail('libtuv builud failed - target not produced.')

    # copy output to libs directory
    mkdir(build_libs)
    shutil.copy(output, libtuv_output_path)

    return True


# Build jerry executable to generate snapshot.
# Jerry executable must be runnable from build HOST machine.
def build_jerry(option):
    # Check if JerryScript submodule exists.
    if not os.path.exists(JERRY_ROOT):
        fail('JerryScript submodule not exists!')

    # Move working directory to JerryScript build directory.
    build_home = join_path([host_build_root, 'deps', 'jerry'])
    mkdir(build_home)
    os.chdir(build_home)

    # Set JerryScript cmake option.
    cmake_opt = [JERRY_ROOT]
    cmake_opt.append('-DCMAKE_TOOLCHAIN_FILE=' + host_cmake_toolchain_file)

    # Turn off LTO for jerry bin to save build time.
    cmake_opt.append('-DENABLE_LTO=OFF')

    # Run cmake.
    check_run_cmd('cmake', cmake_opt)

    target_jerry_name = '%s.%s' % (option.buildtype, sys_os())
    target_jerry = {
        'target_name': target_jerry_name,
        'output_path': join_path([build_home, target_jerry_name])
    }

    # Make option.
    make_opt = ['-C', build_home, target_jerry['target_name']]
    if not option.no_parallel_build:
        make_opt.append('-j')

    # Run make for a target.
    check_run_cmd('make', make_opt)

    # Check output
    output = target_jerry['output_path']
    if not os.path.exists(output):
        fail('JerryScript builud failed - target not produced.')

    # copy
    shutil.copy(output, jerry_output_path)

    return True


def build_libjerry(option):
    # Check if JerryScript submodule exists.
    if not os.path.exists(JERRY_ROOT):
        fail('JerryScript submodule not exists!')

    # Move working directory to JerryScript build directory.
    build_home = join_path([build_root, 'deps', 'jerry'])
    mkdir(build_home)
    os.chdir(build_home)

    # Set JerryScript cmake option.
    cmake_opt = [JERRY_ROOT]

    cmake_opt.append('-DCMAKE_TOOLCHAIN_FILE=' + cmake_toolchain_file)

    if option.target_os == 'nuttx':
        cmake_opt.append('-DEXTERNAL_LIBC_INTERFACE=' +
                         join_path([option.nuttx_home, 'include']))
        if option.target_arch == 'arm':
            cmake_opt.append('-DEXTERNAL_CMAKE_SYSTEM_PROCESSOR=arm')

    if option.target_os == 'linux' or option.target_os == 'openwrt':
        cmake_opt.append('-DUSE_COMPILER_DEFAULT_LIBC=YES')

    # --jerry-heaplimit
    if option.jerry_heaplimit:
        cmake_opt.append('-DEXTERNAL_MEM_HEAP_SIZE_KB=' +
                         str(option.jerry_heaplimit))

    # --jerry-lto
    cmake_opt.append('-DENABLE_LTO=%s' % ('ON' if option.jerry_lto else 'OFF'))

    # --jerry-cmake-param
    cmake_opt += option.jerry_cmake_param

    # inflate cmake option.
    inflate_cmake_option(cmake_opt, option, for_jerry=True)

    # Run cmake.
    check_run_cmd('cmake', cmake_opt)

    # make target - libjerry
    target_libjerry_name = '%s.jerry-core' % option.buildtype
    target_libjerry = {
        'target_name': target_libjerry_name,
        'output_path': join_path([build_home, 'jerry-core',
                                  'lib%s.a' % target_libjerry_name]),
        'dest_path': libjerry_output_path
    }

    # make target - libjerry for mem stat
    target_libjerry_ms_name = '%s-mem_stats.jerry-core' % option.buildtype
    target_libjerry_ms = {
        'target_name': target_libjerry_ms_name,
        'output_path': join_path([build_home, 'jerry-core',
                                  'lib%s.a' % target_libjerry_ms_name]),
        'dest_path': libjerry_output_path
    }

    # make target - target_libfdlibm
    target_libfdlibm_name = '%s.jerry-fdlibm.third_party.lib' % option.buildtype
    target_libfdlibm = {
        'target_name': target_libfdlibm_name,
        'output_path': join_path([build_home, 'third-party', 'fdlibm',
                                  'lib%s.a' % target_libfdlibm_name]),
        'dest_path': libfdlibm_output_path
    }

    targets = [target_libfdlibm]
    if option.jerry_memstat:
        targets.append(target_libjerry_ms)
    else:
        targets.append(target_libjerry)

    # make the target.
    for target in targets:
        # Make option.
        make_opt = ['-C', build_home, target['target_name']]
        if not option.no_parallel_build:
            make_opt.append('-j')

        # Run make for a target.
        check_run_cmd('make', make_opt)

        # Check output
        output = target['output_path']
        if not os.path.exists(output):
            print output
            fail('JerryScript builud failed - target not produced.')

        # copy
        shutil.copy(output, target['dest_path'])

    return True


def build_libhttpparser(option):
    # Check if JerryScript submodule exists.
    if not os.path.exists(HTTPPARSER_ROOT):
        print_error('libhttpparser submodule not exists!')
        return False

    # Move working directory to JerryScript build directory.
    build_home = join_path([build_root, 'deps', 'httpparser'])
    mkdir(build_home)
    os.chdir(build_home)

    # Set JerryScript cmake option.
    cmake_opt = [HTTPPARSER_ROOT]

    cmake_opt.append('-DCMAKE_TOOLCHAIN_FILE=' + cmake_toolchain_file)
    cmake_opt.append('-DBUILDTYPE=' + option.buildtype.capitalize())

    if option.target_os == 'nuttx':
        cmake_opt.append('-DNUTTX_HOME=' + option.nuttx_home)
        cmake_opt.append('-DOS=NUTTX')
    if option.target_os =='linux' or option.target_os == 'openwrt':
        cmake_opt.append('-DOS=LINUX')

    # inflate cmake option.
    inflate_cmake_option(cmake_opt, option)

    # Run cmake.
    check_run_cmd('cmake', cmake_opt)

    # Set make option.
    make_opt = []
    if not option.no_parallel_build:
        make_opt.append('-j')

    # Run make
    check_run_cmd('make', make_opt)

    # Output
    output = join_path([build_home, 'libhttpparser.a'])
    if not os.path.exists(output):
            fail('libhttpparser builud failed - target not produced.')

    # copy
    shutil.copy(output, libhttpparser_output_path)

    return True


def build_iotjs(option):
    # Run js2c
    os.chdir(SCRIPT_PATH)
    check_run_cmd('python', ['js2c.py', option.buildtype,
                             ('no_snapshot'
                              if option.no_snapshot else 'snapshot'),
                             jerry_output_path])

    # Move working directory to IoT.js build directory.
    build_home = join_path([build_root, 'iotjs'])
    mkdir(build_home)
    os.chdir(build_home)

    # Set JerryScript cmake option.
    cmake_opt = [PROJECT_HOME]
    cmake_opt.append('-DCMAKE_TOOLCHAIN_FILE=' + cmake_toolchain_file)
    cmake_opt.append('-DCMAKE_BUILD_TYPE=' + option.buildtype.capitalize())
    cmake_opt.append('-DTARGET_OS=' + option.target_os)
    cmake_opt.append('-DPLATFORM_DESCRIPT=' + platform_descriptor)

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
    cmake_opt.append('-DEXTERNAL_SHARED_LIB=' +
                    ' '.join(option.external_shared_lib))

    # inflate cmake option
    inflate_cmake_option(cmake_opt, option)

    # Run cmake.
    check_run_cmd('cmake', cmake_opt)

    # Set make option.
    make_opt = []
    if not option.no_parallel_build:
        make_opt.append('-j')

    # Run make
    check_run_cmd('make', make_opt)

    # Output
    output = join_path([build_home,
                        'liblibiotjs.a' if option.buildlib else 'iotjs'])

    if not os.path.exists(output):
            fail('IoT.js builud failed - target not produced.')

    # copy
    dest_path = libiotjs_output_path if option.buildlib else iotjs_output_path
    shutil.copy(output, dest_path)

    return True


def run_checktest():
    # iot.js executable
    iotjs = join_path([build_root, 'iotjs', 'iotjs'])
    return run_cmd(CHECKTEST, [iotjs]) == 0


def copy_libraries_for_nuttx(option):
    nuttx_lib_path = join_path([option.nuttx_home, 'lib'])

    shutil.copy(libhttpparser_output_path, nuttx_lib_path)
    shutil.copy(libtuv_output_path, nuttx_lib_path)
    shutil.copy(libjerry_output_path, nuttx_lib_path)
    shutil.copy(libfdlibm_output_path, nuttx_lib_path)
    shutil.copy(libiotjs_output_path, nuttx_lib_path)

    return True


# Initialize build option object.
option = init_option()

adjust_option(option)

print_build_option(option)

set_global_vars(option)

# clean build directory.
if option.clean:
    print_progress('Clear build directory')
    shutil.rmtree(build_root, True)
    shutil.rmtree(host_build_root, True)

create_build_directories(option)

# Perform tidy check.
print_progress('Tidy checking')
if not option.no_check_tidy:
    if not check_tidy(PROJECT_HOME):
        fail("Failed check_tidy")

# Perform init-submodule.
print_progress('Initialize submodules')
if not option.no_init_submodule:
    init_submodule()

# make build directory.
print_progress('Create build directory')
mkdir(build_root)

# build tuv.
print_progress('Build libtuv')
if not build_tuv(option):
    fail('Failed to build libtuv')

# build jerry.
print_progress('Build JerryScript')
if not build_jerry(option):
    fail('Failed to build jerry')
if not build_libjerry(option):
    fail('Failed to build libjerry')

# build httpparser
print_progress('Build libhttpparser')
if not build_libhttpparser(option):
    fail('Failed to build libhttpparser')

# build iotjs.
print_progress('Build IoT.js')
if not build_iotjs(option):
    fail('Failed to build IoT.js')

# check unit test.
if not option.no_check_test:
    print_progress('Check unit tests')
    # Run check test when target is host.
    if option.target_os != sys_os() or option.target_arch != sys_arch():
        print "Skip unit tests - target is not host"
        print
    elif option.buildlib:
        print "Skip unit tests - build target is library"
        print
    else:
        if not run_checktest():
            fail('Failed to pass unit tests')

# copy output libraries to nuttx lib directory.
if option.target_os == 'nuttx':
    print_progress('Copy libraries')
    if not copy_libraries_for_nuttx(option):
        fail("Failed to copy libraries")

print
print "%sIoT.js Build Succeeded!!%s" % (TERM_GREEN, TERM_EMPTY)
print

sys.exit(0)
