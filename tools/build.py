#!/usr/bin/env python

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

from __future__ import print_function
try:
    basestring
except:
    # in Python 3.x there is no basestring just str
    basestring = str

import argparse
import json
import sys
import re
import os

from js2c import js2c
from module_analyzer import resolve_modules, analyze_module_dependency
from common_py import path
from common_py.system.filesystem import FileSystem as fs
from common_py.system.executor import Executor as ex
from common_py.system.platform import Platform

platform = Platform()


# Initialize build options.
def init_options():
    # Check config options.
    arg_config = list(filter(lambda x: x.startswith('--config='), sys.argv))
    config_path = path.BUILD_CONFIG_PATH

    if arg_config:
        config_path = arg_config[-1].split('=', 1)[1]

    # Read config file and apply it to argv.
    argv = []
    with open(config_path, 'rb') as f:
        config = json.loads(f.read().decode('ascii'))
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
    argv = argv + sys.argv[1:]

    # Prepare argument parser.
    parser = argparse.ArgumentParser()

    parser.add_argument('--buildtype',
        choices=['debug', 'release'], default='debug',
        help='Specify the build type: %(choices)s (default: %(default)s)')

    parser.add_argument('--builddir', default=path.BUILD_ROOT,
        help='Specify the build directory (default: %(default)s)')
    parser.add_argument('--buildlib', action='store_true', default=False,
        help='Build IoT.js library only (default: %(default)s)')

    parser.add_argument('--clean', action='store_true', default=False,
        help='Clean build directory before build (default: %(default)s)')

    parser.add_argument('--config', default=path.BUILD_CONFIG_PATH,
        help='Specify the config file (default: %(default)s)',
        dest='config_path')

    parser.add_argument('--target-arch',
        choices=['arm', 'x86', 'i686', 'x86_64', 'x64'],
        default=platform.arch(),
        help='Specify the target architecture: '
             '%(choices)s (default: %(default)s)')
    parser.add_argument('--target-os',
        choices=['linux', 'darwin', 'osx', 'nuttx', 'tizen', 'tizenrt'],
        default=platform.os(),
        help='Specify the target os: %(choices)s (default: %(default)s)')

    parser.add_argument('--target-board',
        choices=['none', 'artik10', 'stm32f4dis', 'rpi2', 'artik05x'],
        default='none', help='Specify the targeted board (if needed): '
             '%(choices)s (default: %(default)s)')
    parser.add_argument('--nuttx-home', default=None, dest='sysroot',
        help='Specify the NuttX base directory (required for NuttX build)')

    parser.add_argument('--cross-compile', dest='cross_compile',
        action='store', help='Specify the cross compilation toolkit prefix.')
    parser.add_argument('--sysroot', action='store',
        help='The location of the development tree root directory (sysroot).'
        'Must be compatible with used toolchain.')

    parser.add_argument('--cmake-param',
        action='append', default=[],
        help='Specify additional cmake parameters '
             '(can be used multiple times)')
    parser.add_argument('--compile-flag',
        action='append', default=[],
        help='Specify additional compile flags (can be used multiple times)')
    parser.add_argument('--link-flag',
        action='append', default=[],
        help='Specify additional linker flags (can be used multiple times)')

    parser.add_argument('--external-include-dir',
        action='append', default=[],
        help='Specify additional external include directory '
             '(can be used multiple times)')
    parser.add_argument('--external-static-lib',
        action='append', default=[],
        help='Specify additional external static library '
             '(can be used multiple times)')
    parser.add_argument('--external-shared-lib',
        action='append', default=[],
        help='Specify additional external shared library '
             '(can be used multiple times)')

    parser.add_argument('--iotjs-include-module',
        action='store', default=set(), type=lambda x: set(x.split(',')),
        help='Specify iotjs modules which should be included '
             '(format: module_1,module_2,...)')
    parser.add_argument('--iotjs-exclude-module',
        action='store', default=set(), type=lambda x: set(x.split(',')),
        help='Specify iotjs modules which should be excluded '
             '(format: module_1,module_2,...)')

    parser.add_argument('--iotjs-minimal-profile',
        action='store_true', default=False,
        help='Build IoT.js with minimal profile')

    parser.add_argument('--jerry-cmake-param',
        action='append', default=[],
        help='Specify additional cmake parameters for JerryScript '
        '(can be used multiple times')
    parser.add_argument('--jerry-compile-flag',
        action='append', default=[],
        help='Specify additional compile flags for JerryScript '
             '(can be used multiple times')
    parser.add_argument('--jerry-link-flag',
        action='append', default=[],
        help='Specify additional linker flags for JerryScript '
             '(can be used multiple times')
    parser.add_argument('--jerry-lto',
        action='store_true', default=False,
        help='Build JerryScript with LTO enabled')

    parser.add_argument('--jerry-heap-section',
        action='store', default=None,
        help='Specify the name of the JerryScript heap section')
    parser.add_argument('--jerry-heaplimit',
        type=int, default=config['build_option']['jerry-heaplimit'],
        help='Sepcify the size of the JerryScript max heap size '
             '(default: %(default)s)')

    parser.add_argument('--jerry-memstat',
        action='store_true', default=False,
        help='Enable JerryScript heap statistics')

    parser.add_argument('--no-init-submodule',
        action='store_true', default=False,
        help='Disable initialization of git submodules')
    parser.add_argument('--no-check-valgrind',
        action='store_true', default=False,
        help='Disable test execution with valgrind after build')
    parser.add_argument('--no-check-test',
        action='store_true', default=False,
        help='Disable test exection after build')
    parser.add_argument('--no-parallel-build',
        action='store_true', default=False,
        help='Disable parallel build')
    parser.add_argument('--no-snapshot',
        action='store_true', default=False,
        help='Disable snapshot generation for IoT.js')
    parser.add_argument('-e', '--experimental',
        action='store_true', default=False,
        help='Enable to build experimental features')

    options = parser.parse_args(argv)
    options.config = config

    return options


def adjust_options(options):
    # First fix some option inconsistencies
    if options.target_os in ['nuttx', 'tizenrt']:
        options.buildlib = True
        if not options.sysroot:
            ex.fail('--sysroot needed for nuttx target')

        options.sysroot = fs.abspath(options.sysroot)
        if not fs.exists(options.sysroot):
            ex.fail('Nuttx sysroot %s does not exist' % options.sysroot)

    if options.target_arch == 'x86':
        options.target_arch = 'i686'
    if options.target_arch == 'x64':
        options.target_arch = 'x86_64'

    if options.target_os == 'darwin':
        options.no_check_valgrind = True

    if options.target_board in ['rpi2', 'artik10', 'artik05x']:
        options.no_check_valgrind = True
    elif options.target_board == 'none':
        options.target_board = None

    if options.iotjs_minimal_profile:
        options.no_check_test = True

    # Then add calculated options
    options.host_tuple = '%s-%s' % (platform.arch(), platform.os())
    options.target_tuple = '%s-%s' % (options.target_arch, options.target_os)
    arch_for_iotjs = 'arm' if options.target_arch[0:3] == 'arm' else \
        options.target_arch
    os_for_iotjs = 'linux' if options.target_os == 'tizen' else \
        options.target_os
    options.target_tuple_for_iotjs = '%s-%s' % (arch_for_iotjs, os_for_iotjs)

    options.host_build_root = fs.join(path.PROJECT_ROOT,
                                     options.builddir,
                                     'host',
                                     options.host_tuple,
                                     options.buildtype)
    options.host_build_bins = fs.join(options.host_build_root, 'bin')

    options.build_root = fs.join(path.PROJECT_ROOT,
                                 options.builddir,
                                 options.target_tuple,
                                 options.buildtype)
    options.build_bins = fs.join(options.build_root, 'bin')
    options.build_libs = fs.join(options.build_root, 'lib')

    cmake_path = fs.join(path.PROJECT_ROOT, 'cmake', 'config', '%s.cmake')
    options.cmake_toolchain_file = cmake_path % options.target_tuple
    options.host_cmake_toolchain_file = cmake_path % options.host_tuple


def print_build_option(options):
    print('=================================================')
    option_vars = vars(options)
    for opt in option_vars:
        print(' --%s: %s' % (opt, option_vars[opt]))
    print()


def print_progress(msg):
    print('==> %s\n' % msg)


def create_build_directories(options):
    print_progress('Create build directories')

    fs.maybe_make_directory(options.build_root)
    fs.maybe_make_directory(options.build_bins)
    fs.maybe_make_directory(options.build_libs)
    fs.maybe_make_directory(options.host_build_root)
    fs.maybe_make_directory(options.host_build_bins)


def init_submodule():
    ex.check_run_cmd('git', ['submodule', 'init'])
    ex.check_run_cmd('git', ['submodule', 'update'])


def build_cmake_args(options, for_jerry=False):
    cmake_args = []
    # compile flags
    compile_flags = []

    config_compile_flags = options.config['compile_flags']
    compile_flags += config_compile_flags['os'][options.target_os]
    compile_flags += config_compile_flags['arch'][options.target_arch]
    compile_flags += config_compile_flags['buildtype'][options.buildtype]
    if options.target_board:
        compile_flags += config_compile_flags['board'][options.target_board]

    compile_flags += options.compile_flag
    compile_flags += options.jerry_compile_flag if for_jerry else []

    cmake_args.append("-DCMAKE_C_FLAGS='%s'" % (' '.join(compile_flags)))

    # link flags
    link_flags = []

    config_link_flags = options.config['link_flags']
    link_flags += config_link_flags['os'][options.target_os]

    link_flags += options.link_flag
    link_flags += options.jerry_link_flag if for_jerry else []

    if options.jerry_lto:
        link_flags.append('-flto')

    cmake_args.append("-DCMAKE_EXE_LINKER_FLAGS='%s'" % (' '.join(link_flags)))

    # external include dir
    include_dirs = []
    if options.target_os in ['nuttx', 'tizenrt'] and options.sysroot:
        include_dirs.append('%s/include' % options.sysroot)
        if options.target_board == 'stm32f4dis':
            include_dirs.append('%s/arch/arm/src/stm32' % options.sysroot)
    include_dirs.extend(options.external_include_dir)
    cmake_args.append("-DEXTERNAL_INCLUDE_DIR='%s'" % (' '.join(include_dirs)))

    # set target board
    if options.target_board == 'stm32f4dis':
        cmake_args.append('-DCMAKE_TARGET_BOARD=STM32')

    return cmake_args


def run_make(options, build_home, *args):
    make_opt = ['-C', build_home]
    make_opt.extend(args)
    if not options.no_parallel_build:
        make_opt.append('-j')

    ex.check_run_cmd('make', make_opt)


def copy_build_target(target_name, src_dir, dst_dir, new_target_name=None):
    build_output = fs.join(src_dir, target_name)
    if not fs.exists(build_output):
        ex.fail('%s build failed - target not produced.' % target_name)

    if new_target_name:
        copy_target_name = new_target_name
    else:
        copy_target_name = target_name

    copy_target_path = fs.join(dst_dir, copy_target_name)
    fs.copy(build_output, copy_target_path)


def build_tuv(options):
    print_progress('Build libtuv')

    # Check if libtuv submodule exists.
    if not fs.exists(path.TUV_ROOT):
        ex.fail('libtuv submodule does not exists!')

    # Move working directory to libtuv build directory.
    build_home = fs.join(options.build_root, 'deps', 'libtuv')

    # Set tuv cmake options.
    cmake_opt = [
        '-B%s' % build_home,
        '-H%s' % path.TUV_ROOT,
        "-DCMAKE_TOOLCHAIN_FILE='%s'" %
                     fs.join(path.TUV_ROOT, 'cmake', 'config',
                             'config_%s.cmake' % options.target_tuple),
        "-DCMAKE_BUILD_TYPE='%s'" % options.buildtype,
        "-DTARGET_PLATFORM='%s'" % options.target_tuple,
        "-DLIBTUV_CUSTOM_LIB_OUT='%s'" % build_home,
        '-DBUILDTESTER=NO',
        '-DBUILDAPIEMULTESTER=NO',
    ]

    if options.cross_compile:
        cmake_opt.append("-DCROSS_COMPILE='%s'" % options.cross_compile)

    if options.sysroot:
        cmake_opt.append("-DTARGET_SYSTEMROOT='%s'" % options.sysroot)
        cmake_opt.append("-DSYSROOT='%s'" % options.sysroot)

    if options.target_board:
        cmake_opt.append("-DTARGET_BOARD='%s'" % options.target_board)

    # Add common cmake options.
    cmake_opt.extend(build_cmake_args(options))

    # Run cmake.
    ex.check_run_cmd('cmake', cmake_opt)

    run_make(options, build_home)
    copy_build_target('libtuv.a', build_home, options.build_libs)


# Build jerry executable to generate snapshot.
# Jerry executable must be runnable from build HOST machine.
def build_host_jerry(options):
    print_progress('Build JerryScript for host')

    # Check if JerryScript submodule exists.
    if not fs.exists(path.JERRY_ROOT):
        ex.fail('JerryScript submodule does not exists!')

    # Move working directory to JerryScript build directory.
    build_home = fs.join(options.host_build_root, 'deps', 'jerry')

    # Set JerryScript cmake options.
    cmake_opt = [
        '-B%s' % build_home,
        '-H%s' % path.JERRY_ROOT,
        "-DCMAKE_TOOLCHAIN_FILE='%s'" % options.host_cmake_toolchain_file,
        # Turn off LTO for jerry bin to save build time.
        '-DENABLE_LTO=OFF',

    ]

    if options.buildtype == 'debug':
        cmake_opt.append('-DCMAKE_BUILD_TYPE=Debug')

    # Turn on snapshot
    if not options.no_snapshot:
        cmake_opt.append('-DFEATURE_SNAPSHOT_SAVE=ON')

    # Run cmake.
    ex.check_run_cmd('cmake', cmake_opt)

    run_make(options, build_home)
    copy_build_target('jerry',
                      fs.join(build_home, 'bin'),
                      options.host_build_bins)


def build_libjerry(options):
    print_progress('Build JerryScript target library')

    # Check if JerryScript submodule exists.
    if not fs.exists(path.JERRY_ROOT):
        ex.fail('JerryScript submodule does not exists!')

    # Move working directory to JerryScript build directory.
    build_home = fs.join(options.build_root, 'deps', 'jerry')

    # Set JerryScript cmake options.
    cmake_opt = [
        '-B%s' % build_home,
        '-H%s' % path.JERRY_ROOT,
        '-DCMAKE_TOOLCHAIN_FILE=%s' % options.cmake_toolchain_file,
    ]

    if options.buildtype == 'debug':
        cmake_opt.append('-DCMAKE_BUILD_TYPE=Debug')
        cmake_opt.append('-DFEATURE_ERROR_MESSAGES=On')

    if options.target_os in ['nuttx', 'tizenrt']:
        cmake_opt.append("-DEXTERNAL_LIBC_INTERFACE='%s'" %
                         fs.join(options.sysroot, 'include'))
        if options.target_arch == 'arm':
            cmake_opt.append('-DEXTERNAL_CMAKE_SYSTEM_PROCESSOR=arm')

    if options.target_os in ['linux', 'tizen', 'darwin']:
        cmake_opt.append('-DJERRY_LIBC=OFF')
        cmake_opt.append('-DJERRY_LIBM=OFF')

    if options.target_os in ['tizenrt']:
        cmake_opt.append('-DJERRY_LIBC=OFF')
        cmake_opt.append('-DJERRY_LIBM=ON')

    # --jerry-heaplimit
    if options.jerry_heaplimit:
        cmake_opt.append('-DMEM_HEAP_SIZE_KB=%d' % options.jerry_heaplimit)

    # --jerry-heap-section
    if options.jerry_heap_section:
        cmake_opt.append("-DJERRY_HEAP_SECTION_ATTR='%s'" %
                         options.jerry_heap_section)

    # --jerry-lto
    cmake_opt.append('-DENABLE_LTO=%s' % ('ON' if options.jerry_lto else 'OFF'))

    if options.jerry_memstat:
        cmake_opt.append('-DFEATURE_MEM_STATS=ON')

    # Turn off snapshot save and turn on snapshot exec if requested
    cmake_opt.append('-DFEATURE_SNAPSHOT_SAVE=OFF')
    if not options.no_snapshot:
        cmake_opt.append('-DFEATURE_SNAPSHOT_EXEC=ON')

    # --jerry-cmake-param
    cmake_opt.extend(options.jerry_cmake_param)

    # Add common cmake options.
    cmake_opt.extend(build_cmake_args(options, for_jerry=True))

    # Run cmake.
    ex.check_run_cmd('cmake', cmake_opt)

    targets = [
        # make target - libjerry
        {
            'name': 'jerry-core',
            'build_name': 'libjerry-core.a',
            'build_path': fs.join(build_home, 'lib'),
            'dest_name': 'libjerrycore.a',
            'dest_path': options.build_libs,
        },
    ]

    # make the target.
    for target in targets:
        run_make(options, build_home, target['name'])
        copy_build_target(target['build_name'], target['build_path'],
                          target['dest_path'], target['dest_name'])


def build_libhttpparser(options):
    print_progress('Build libhttpparser')

    # Check if JerryScript submodule exists.
    if not fs.exists(path.HTTPPARSER_ROOT):
        ex.fail('libhttpparser submodule does not exists!')

    # Move working directory to JerryScript build directory.
    build_home = fs.join(options.build_root, 'deps', 'httpparser')

    # Set JerryScript cmake options.
    cmake_opt = [
        '-B%s' % build_home,
        '-H%s' % path.HTTPPARSER_ROOT,
        "-DCMAKE_TOOLCHAIN_FILE='%s'" % options.cmake_toolchain_file,
        '-DBUILDTYPE=%s' % options.buildtype.capitalize(),
    ]

    if options.target_os in ['nuttx', 'tizenrt']:
        cmake_opt.append("-DNUTTX_HOME='%s'" % options.sysroot)

    if options.target_os in ['nuttx', 'tizenrt']:
        cmake_opt.append('-DOS=NUTTX')

    if options.target_os in ['linux', 'tizen']:
        cmake_opt.append('-DOS=LINUX')

    if options.target_os == 'darwin':
        cmake_opt.append('-DOS=DARWIN')

    # Add common cmake options.
    cmake_opt.extend(build_cmake_args(options))

    # Run cmake.
    ex.check_run_cmd('cmake', cmake_opt)

    run_make(options, build_home)
    copy_build_target('libhttpparser.a', build_home, options.build_libs)


def build_iotjs(options):
    print_progress('Build IoT.js')

    # Move working directory to IoT.js build directory.
    build_home = fs.join(options.build_root, 'iotjs')

    # Set JerryScript cmake options.
    cmake_opt = [
        '-B%s' % build_home,
        '-H%s' % path.PROJECT_ROOT,
        "-DCMAKE_TOOLCHAIN_FILE='%s'" % options.cmake_toolchain_file,
        '-DCMAKE_BUILD_TYPE=%s' % options.buildtype.capitalize(),
        '-DTARGET_OS=%s' % options.target_os,
        '-DPLATFORM_DESCRIPT=%s' % options.target_tuple_for_iotjs,
    ]

    # IoT.js module list
    cmake_opt.append("-DIOTJS_MODULES='%s'" %
                     (' '.join(options.native_modules)))

    if not options.no_snapshot:
        options.compile_flag.append('-DENABLE_SNAPSHOT')

    if options.target_os == 'nuttx':
        cmake_opt.append("-DNUTTX_HOME='%s'" % options.sysroot)

    # --build-lib
    if options.buildlib:
        cmake_opt.append('-DBUILD_TO_LIB=YES')

    # --cmake-param
    cmake_opt.extend(options.cmake_param)

    # --external_static_lib
    cmake_opt.append("-DEXTERNAL_STATIC_LIB='%s'" %
                     (' '.join(options.external_static_lib)))

    # --external_shared_lib
    shared_libs = []
    shared_libs.extend(options.external_shared_lib)
    shared_libs.extend(options.config['shared_libs']['os'][options.target_os])
    cmake_opt.append("-DEXTERNAL_SHARED_LIB='%s'" % (' '.join(shared_libs)))

    # --experimental
    if options.experimental:
        options.compile_flag.append('-DEXPERIMENTAL')

    # Add common cmake options.
    cmake_opt.extend(build_cmake_args(options))

    # Run cmake.
    ex.check_run_cmd('cmake', cmake_opt)

    run_make(options, build_home)

    # Copy the correct target to the correct place
    if options.buildlib:
        src_name = 'liblibiotjs.a'
        dst_dir = options.build_libs
        dst_name = 'libiotjs.a'
    else:
        dst_name = src_name = 'iotjs'
        dst_dir = options.build_bins

    copy_build_target(src_name, build_home, dst_dir, dst_name)


def process_modules(options):
    print_progress('Analyze modules')

    includes, excludes = resolve_modules(options)
    modules = analyze_module_dependency(includes, excludes)

    print('Selected js modules: %s' % ', '.join(modules['js']))
    print('Selected native modules: %s' % ', '.join(modules['native']))

    options.js_modules = modules['js']
    options.native_modules = modules['native']
    options.iotjs_exclude_module = excludes


def run_checktest(options):
    checktest_quiet = 'yes'
    if os.getenv('TRAVIS') == "true":
        checktest_quiet = 'no'

    # iot.js executable
    iotjs = fs.join(options.build_root, 'iotjs', 'iotjs')
    build_args = ['--', 'quiet=' + checktest_quiet]
    if options.iotjs_exclude_module:
        skip_module = ','.join(options.iotjs_exclude_module)
        build_args.append('skip-module=' + skip_module)

    # experimental
    if options.experimental:
        build_args.append('experimental=' + 'yes');

    fs.chdir(path.PROJECT_ROOT)
    code = ex.run_cmd(iotjs, [path.CHECKTEST_PATH] + build_args)
    if code != 0:
        ex.fail('Failed to pass unit tests')
    if not options.no_check_valgrind:
        code = ex.run_cmd('valgrind', ['--leak-check=full',
                                       '--error-exitcode=5',
                                       '--undef-value-errors=no',
                                       iotjs,
                                       path.CHECKTEST_PATH] + build_args)
        if code == 5:
            ex.fail('Failed to pass valgrind test')
        if code != 0:
            ex.fail('Failed to pass unit tests in valgrind environment')


if __name__ == '__main__':
    # Initialize build option object.
    options = init_options()
    adjust_options(options)
    print_build_option(options)

    if options.clean:
        print_progress('Clear build directory')
        fs.rmtree(options.build_root)
        fs.rmtree(options.host_build_root)

    create_build_directories(options)

    process_modules(options)

    # Perform init-submodule.
    if not options.no_init_submodule:
        print_progress('Initialize submodule')
        init_submodule()

    build_tuv(options)
    if not options.no_snapshot:
        build_host_jerry(options)
    build_libjerry(options)
    build_libhttpparser(options)

    # Run js2c
    print("Building js library: %s" % (", ".join(options.js_modules)))
    js2c(options.buildtype, options.no_snapshot, options.js_modules,
         fs.join(options.host_build_bins, 'jerry'))

    build_iotjs(options)

    # Run tests
    if not options.no_check_test:
        print_progress('Run tests')
        if options.buildlib:
            print("Skip unit tests - build target is library\n")
        elif (options.host_tuple == options.target_tuple or
              (options.host_tuple == 'x86_64-linux' and
               options.target_tuple == 'i686-linux')):
             run_checktest(options)
        else:
            print("Skip unit tests - target-host pair is not allowed\n")

    print("\n%sIoT.js Build Succeeded!!%s\n" % (ex._TERM_GREEN, ex._TERM_EMPTY))
