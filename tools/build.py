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

import os
import shutil
import subprocess
import sys
from os import path
from check_tidy import check_tidy

config_echo = True


def join_path(pathes):
    return path.abspath(reduce(lambda x, y: path.join(x, y), pathes))


def check_path(path):
    if not os.path.exists(path):
        return False
    return True


# Path for this script file.
 # should be <project_home>/tools.
SCRIPT_PATH = path.dirname(path.abspath(__file__))

# Home directory for the project.
ROOT = join_path([SCRIPT_PATH, '../'])

# Root directory for dependencies.
DEPS_ROOT = join_path([ROOT, 'deps'])

# Root directory for jerry script submodule.
JERRY_ROOT = join_path([DEPS_ROOT, 'jerry'])

# Root directory for libuv submodule.
LIBUV_ROOT = join_path([DEPS_ROOT, 'libuv'])

# Build directory suffix for jerry build.
JERRY_BUILD_SUFFIX = 'deps/jerry'

# Build direcrtory suffix for libuv build.
LIBUV_BUILD_SUFFIX = 'deps/libuv'

# Root directory for the source files.
SRC_ROOT = join_path([ROOT, 'src'])

# Root directory for the include files.
INT_ROOT = join_path([ROOT, 'inc'])


def mkdir(path):
    run_cmd('mkdir', ['-p', path])


def copy(src, dst):
    run_cmd('cp', [src, dst])


def run_cmd(cmd, args = []):
    cmd_line = cmd
    if len(args) > 0:
        cmd_line = cmd_line + " " + reduce(lambda x, y: x + " " + y, args)
    if config_echo:
        print cmd_line
    os.system(cmd_line)


def get_git_hash(path):
    os.chdir(path)
    return subprocess.check_output(['git', 'rev-parse', 'HEAD']).strip()


def sys_name():
    name, _, _, _, _ = os.uname()
    return name.lower()

def sys_machine():
    _, _, _, _, machine = os.uname()
    return machine.lower()


options = {
    'buildtype': 'release',
    'builddir': 'build',
    'buildlib': False,
    'target-arch': sys_machine(),
    'target-os': sys_name(),
    'make-flags': '-j',
    'nuttx-home': '',
    'init-submodule': True,
    'tidy': True,
}

boolean_opts = ['buildlib', 'init-submodule', 'tidy']

def opt_build_type():
    return options['buildtype']

def opt_build_lib():
    return options['buildlib']

def opt_build_root():
    return join_path([ROOT,
                      options['builddir'],
                      opt_target_tuple(),
                      opt_build_type()])

def opt_build_libs():
    return join_path([opt_build_root(), 'libs'])

def opt_target_arch():
    return options['target-arch']

def opt_target_os():
    return options['target-os']

def opt_target_tuple():
    return opt_target_arch() + '-' + opt_target_os()

def opt_make_flags():
    return options['make-flags']

def opt_nuttx_home():
    return join_path([ROOT, options['nuttx-home']])

def opt_cmake_toolchain_file():
    return join_path([ROOT, 'cmake/config', opt_target_tuple() + '.cmake'])

def opt_init_submodule():
    return options['init-submodule']

def opt_tidy():
    return options['tidy']

def parse_boolean_opt(name, arg):
    if arg.endswith(name):
        options[name] = False if arg.startswith('no') else True
        return True
    return False

def parse_args():
    for arg in sys.argv:
        optpair = arg.split('=', 1)
        opt = optpair[0][2:].lower()
        val = optpair[1] if len(optpair) == 2 else ""
        if opt == 'buildtype':
            if val.lower() == 'release':
                options[opt] = val.lower()
            elif val.lower() == 'debug':
                options[opt] = val.lower()
        elif opt == 'builddir':
            options[opt] = val
        elif opt == 'target-arch':
            if val.lower() in ['x86_64', 'i686', 'arm']:
                options[opt] = val.lower()
        elif opt == 'target-os':
            if val.lower() in ['linux', 'darwin', 'nuttx']:
                options[opt] = val.lower()
        elif opt == 'make-flags':
            options[opt] = val
        elif opt == 'nuttx-home':
            options[opt] = val
        else:
            for opt_name in boolean_opts:
                if parse_boolean_opt(opt_name, opt):
                    break

def init_submodule():
    run_cmd('git', ['submodule', 'init'])
    run_cmd('git', ['submodule', 'update'])


def libuv_output_path():
    return join_path([opt_build_libs(), "libuv.a"])


def build_libuv():
    # check libuv submodule directory.
    if not check_path(LIBUV_ROOT):
        print '* libuv build failed - submodule not exists.'
        return False

    # get libuv get hash.
    git_hash = get_git_hash(LIBUV_ROOT)

    # libuv build directory.
    build_home = join_path([opt_build_root(), LIBUV_BUILD_SUFFIX])

    # libuv cached library.
    build_cache_dir = join_path([build_home, 'cache'])
    build_cache_name = join_path([build_cache_dir, 'libuv.' + git_hash])

    # check if cache exists.
    if not check_path(build_cache_name):
        # build libuv.

        # make build directory.
        mkdir(build_home)

        # change current directory to libuv.
        os.chdir(LIBUV_ROOT)


        # libuv is using gyp. run the system according to build target.
        if opt_target_arch() == 'arm' and opt_target_os() =='nuttx':
            run_cmd('./nuttx-configure', [opt_nuttx_home()])
        else:
            run_cmd('./gyp_uv.py', ['-f', 'make'])


        # set build type.
        build_type = 'Release' if opt_build_type() == 'release' else 'Debug'

        # make libuv.
        run_cmd('make', ['-C',
                         'out',
                         'BUILDTYPE=' + build_type,
                         opt_make_flags()])

        # output: libuv.a
        output = join_path([LIBUV_ROOT, 'out', build_type, 'libuv.a'])

        # check if target is created.
        if not check_path(output):
            print '* libuv build failed - target not produced.'
            return False

        # copy output to cache
        mkdir(build_cache_dir)
        copy(output, build_cache_name)

    # copy cache to libs directory
    mkdir(opt_build_libs())
    copy(build_cache_name, libuv_output_path())

    return True



def libjerry_output_path():
    return join_path([opt_build_libs(), 'libjerrycore.a'])

def build_libjerry():
    # check libjerry submodule directory.
    if not check_path(JERRY_ROOT):
        print '* libjerry build failed - submodule not exists.'
        return False

    # get jerry git hash.
    git_hash = get_git_hash(JERRY_ROOT)

    # jerry build directory.
    build_home = join_path([opt_build_root(), JERRY_BUILD_SUFFIX])

    # jerry cached library.
    build_cache_dir = join_path([build_home, 'cache'])
    build_cache_name = join_path([build_cache_dir, 'libjerry-core.' + git_hash])

    # check if cache exists.
    if not check_path(build_cache_name):
        # build jerry.

        # make build directory.
        mkdir(build_home)

        # change current directory to build directory.
        os.chdir(build_home)


        # libjerry is using cmake.
        # prepare cmake command line option.
        jerry_cmake_opt = [JERRY_ROOT]

        # set lto off.
        jerry_cmake_opt.append('-DENABLE_LTO=OFF')

        # tool chain file.
        jerry_cmake_opt.append('-DCMAKE_TOOLCHAIN_FILE=' +
                               opt_cmake_toolchain_file())

        # for nuttx build.
        if opt_target_arch() == 'arm' and opt_target_os() =='nuttx':
            # nuttx include path.
            jerry_cmake_opt.append('-DEXTERNAL_LIBC_INTERFACE=' +
                                   join_path([opt_nuttx_home(), 'include']))

        # run cmake.
        # FIXME: Running cmake once cause a problem because cmake does not know
        # the system like "System is unknown to cmake". and the other settings
        # are not applied intendly, running twice solves the problem.
        run_cmd('cmake', jerry_cmake_opt)
        run_cmd('cmake', jerry_cmake_opt)


        # cmake will produce a Makefile.

        # set build target.
        jerry_build_target = opt_build_type() + '.jerry-core'

        # run make.
        run_cmd('make', ['-C',
                         build_home,
                         jerry_build_target,
                         opt_make_flags()])

        # output: libjerry-core.a
        output = join_path([build_home,
                            'jerry-core',
                            'lib' + jerry_build_target + '.a'])

        # check if target is created.
        if not check_path(output):
            print 'Jerry build failed - target not produced.'
            return False

        # copy output to cache
        mkdir(build_cache_dir)
        copy(output, build_cache_name)

    # copy cache to libs directory
    mkdir(opt_build_libs())
    copy(build_cache_name, libjerry_output_path())

    return True


def build_iotjs():
    # iot.js build directory.
    build_home = join_path([opt_build_root(), 'iotjs'])
    mkdir(build_home)

    # set build type.
    build_type = 'Release' if opt_build_type() == 'release' else 'Debug'

    # build iotjs.

    # change current directory to build home.
    os.chdir(build_home)

    # IoT.js is using cmake.
    # prepare cmake command line option.
    iotjs_cmake_opt = [ROOT, "-DCMAKE_BUILD_TYPE=" + build_type]

    # set toolchain file.
    iotjs_cmake_opt.append('-DCMAKE_TOOLCHAIN_FILE=' +
                           opt_cmake_toolchain_file())

    # for nuttx build.
    if opt_target_arch() == 'arm' and opt_target_os() =='nuttx':
        iotjs_cmake_opt.append('-DNUTTX_HOME=' + opt_nuttx_home())
        options['buildlib'] = True

    if opt_build_lib():
        iotjs_cmake_opt.append('-DBUILD_TO_LIB=YES')

    # run cmake
    # FIXME: Running cmake once cause a problem because cmake does not know the
    # system like "System is unknown to cmake". and the other settings are not
    # applied intendly, running twice solves the problem.
    run_cmd('cmake', iotjs_cmake_opt)
    run_cmd('cmake', iotjs_cmake_opt)

    # run make
    run_cmd('make', [opt_make_flags()])

    return True



# parse arguments.
parse_args()

print('=======================================')
for opt_key in options:
    print ' -', opt_key, ':', options[opt_key]
print

# tidy check.
if opt_tidy():
    if not check_tidy(ROOT):
        print 'Failed check_tidy'
        sys.exit(1)

# init submodules.
if opt_init_submodule():
    init_submodule()

# make build directory.
mkdir(opt_build_root())

# build libuv.
if not build_libuv():
    sys.exit(1)

# build jerry lib.
if not build_libjerry():
    sys.exit(1)

# build iot.js
if not build_iotjs():
    sys.exit(1)

sys.exit(0)
