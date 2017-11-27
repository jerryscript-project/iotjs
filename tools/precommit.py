#!/usr/bin/env python

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

from __future__ import print_function

import argparse
import sys
import os
import json
from common_py import path
from common_py.system.filesystem import FileSystem as fs
from common_py.system.executor import Executor as ex
from common_py.system.platform import Platform
from check_tidy import check_tidy

TESTS=['host-linux', 'host-darwin', 'rpi2', 'nuttx', 'misc', 'no-snapshot',
       'artik10', 'artik053', 'coverity']
BUILDTYPES=['debug', 'release']
NUTTXTAG = 'nuttx-7.19'

# This is latest tested TizenRT commit working for IoT.js
# Title: Merge pull request #496 from sunghan-chang/iotivity
TIZENRT_COMMIT='0f47277170972bb33b51996a374c483e4ff2c26a'


def parse_option():
    parser = argparse.ArgumentParser(
         description='IoT.js pre-commit script.',
         epilog='If no arguments are given, runs full test.')
    parser.add_argument('--test', choices=TESTS, action='append')
    parser.add_argument('--buildtype', choices=BUILDTYPES, action='append')
    parser.add_argument('--buildoptions', action='store', default='',
                        help='A comma separated list of extra buildoptions')
    parser.add_argument("--enable-testsuite", action='store_true',
                        default=False, help="Append testsuite onto the device")
    parser.add_argument("--flash", action='store_true', default=False,
                        help="Flash binary onto the device")

    option = parser.parse_args(sys.argv[1:])
    if option.test is None:
        option.test = TESTS
    if option.buildtype is None:
        option.buildtype = BUILDTYPES
    return option


def setup_nuttx_root(nuttx_root):
    # Step 1
    fs.maybe_make_directory(nuttx_root)
    fs.chdir(nuttx_root)
    if not fs.exists('nuttx'):
        ex.check_run_cmd('git', ['clone',
                                 'https://bitbucket.org/nuttx/nuttx.git'])
    fs.chdir('nuttx')
    ex.check_run_cmd('git', ['checkout', NUTTXTAG])
    fs.chdir('..')

    if not fs.exists('apps'):
        ex.check_run_cmd('git', ['clone',
                                 'https://bitbucket.org/nuttx/apps.git'])
    fs.chdir('apps')
    ex.check_run_cmd('git', ['checkout', NUTTXTAG])
    fs.chdir('..')

    # Step 2
    fs.maybe_make_directory(fs.join(nuttx_root, 'apps', 'system', 'iotjs'))
    for file in fs.listdir(fs.join(path.PROJECT_ROOT,
                                   'config', 'nuttx', 'stm32f4dis','app')):
        fs.copy(fs.join(path.PROJECT_ROOT, 'config',
                        'nuttx', 'stm32f4dis', 'app', file),
                fs.join(nuttx_root, 'apps', 'system', 'iotjs'))

    # Step 3
    fs.chdir(fs.join(nuttx_root, 'nuttx', 'tools'))
    ex.check_run_cmd('./configure.sh', ['stm32f4discovery/usbnsh'])
    fs.chdir('..')
    fs.copy(fs.join(path.PROJECT_ROOT,
                    'config',
                    'nuttx',
                    'stm32f4dis',
                    'config.travis'),
            '.config')


def setup_stlink():
    '''
    Setup the stlink dependency.
    '''
    fs.chdir(path.DEPS_ROOT)

    if not fs.exists('stlink'):
        ex.check_run_cmd('git', ['clone',
                                 'https://github.com/texane/stlink.git'])

    if not fs.exists(fs.join(path.DEPS_ROOT, 'stlink/build/Release/st-flash')):
        fs.chdir(fs.join(path.DEPS_ROOT, 'stlink'))
        ex.check_run_cmd('make', ['release'])

    fs.chdir(path.PROJECT_ROOT)


def flash_nuttx(nuttx_root):
    '''
    Flash the NuttX binary onto the board.
    '''
    setup_stlink()
    nuttx_bin = fs.join(nuttx_root, 'nuttx/nuttx.bin')

    if fs.exists(nuttx_bin):
        fs.chdir(fs.join(path.DEPS_ROOT, 'stlink/build/Release'))
        options = ['write', nuttx_bin, '0x8000000']
        ex.check_run_cmd('./st-flash', options)
        fs.chdir(path.PROJECT_ROOT)


def build_nuttx(nuttx_root, buildtype, maketarget):
    fs.chdir(fs.join(nuttx_root, 'nuttx'))
    if buildtype == "release":
        rflag = 'R=1'
    else:
        rflag = 'R=0'
    ex.check_run_cmd('make',
                     [maketarget, 'IOTJS_ROOT_DIR=' + path.PROJECT_ROOT, rflag])


def setup_tizen_root(tizen_root):
    if fs.exists(tizen_root):
        fs.chdir(tizen_root)
        ex.check_run_cmd('git', ['pull'])
        fs.chdir(path.PROJECT_ROOT)
    else:
        ex.check_run_cmd('git', ['clone',
            'https://github.com/pmarcinkiew/tizen3.0_rootstrap.git',
            tizen_root])

def copy_tiznert_stuff(tizenrt_root, iotjs_dir):
    tizenrt_iotjsapp_dir = fs.join(tizenrt_root, 'apps/system/iotjs')
    tizenrt_config_dir = fs.join(tizenrt_root, 'build/configs/artik053/iotjs')
    iotjs_tizenrt_appdir = fs.join(iotjs_dir,
                                  'config/tizenrt/artik05x/app')
    iotjs_config_dir = \
        fs.join(iotjs_dir, 'config/tizenrt/artik05x/configs')

    ex.check_run_cmd('cp',
                    ['-rfu', iotjs_tizenrt_appdir, tizenrt_iotjsapp_dir])

    ex.check_run_cmd('cp',
                    ['-rfu', iotjs_config_dir, tizenrt_config_dir])

def setup_tizenrt_repo(tizenrt_root):
    if fs.exists(tizenrt_root):
        fs.chdir(tizenrt_root)
        ex.check_run_cmd('git', ['fetch', 'origin'])
        fs.chdir(path.PROJECT_ROOT)
    else:
        ex.check_run_cmd('git', ['clone',
            'https://github.com/Samsung/TizenRT.git',
            tizenrt_root])
    ex.check_run_cmd('git', ['--git-dir', tizenrt_root + '/.git/',
                             '--work-tree', tizenrt_root,
                             'checkout', TIZENRT_COMMIT])
    copy_tiznert_stuff(tizenrt_root, path.PROJECT_ROOT)

def configure_trizenrt(tizenrt_root, buildtype):
    # TODO: handle buildtype (build vs release) for tizenrt build
    tizenrt_tools = fs.join(tizenrt_root, 'os/tools')
    fs.chdir(tizenrt_tools)
    ex.check_run_cmd('./configure.sh', ['artik053/iotjs'])
    fs.chdir('..')
    ex.check_run_cmd('make', ['context'])

def build_tizenrt(tizenrt_root, iotjs_rootdir, buildtype):
    fs.chdir(fs.join(tizenrt_root, 'os'))
    iotjs_libdir = iotjs_rootdir + '/build/arm-tizenrt/' + buildtype + '/lib'
    ex.check_run_cmd('make', ['IOTJS_ROOT_DIR=' + iotjs_rootdir,
                              'IOTJS_LIB_DIR=' + iotjs_libdir])

def build(buildtype, args=[]):
    fs.chdir(path.PROJECT_ROOT)
    ex.check_run_cmd('./tools/build.py', ['--buildtype=' + buildtype] + args)


def apply_nuttx_patches(nuttx_root, use_patches=True):
    '''
    Apply memstat patches to measure the memory consumption of IoT.js.
    '''
    fs.chdir(path.PROJECT_ROOT)

    options = ['apply']
    if not use_patches:
        options.append('-R')
    else:
        ex.check_run_cmd('git', ['submodule', 'init'])
        ex.check_run_cmd('git', ['submodule', 'update'])

    patch_dir = fs.join(path.PROJECT_ROOT, 'config', 'nuttx', 'stm32f4dis')
    ex.check_run_cmd('git', options + [fs.join(patch_dir,
                                               'iotjs-memstat.diff')])
    fs.chdir(path.TUV_ROOT)
    ex.check_run_cmd('git', options + [fs.join(patch_dir,
                                               'libtuv-memstat.diff')])
    fs.chdir(path.JERRY_ROOT)
    ex.check_run_cmd('git', options + [fs.join(patch_dir,
                                               'jerry-memstat.diff')])
    fs.chdir(fs.join(nuttx_root, 'nuttx'))
    ex.check_run_cmd('git', options + [fs.join(patch_dir,
                                               'nuttx-7.19.diff')])
    fs.chdir(path.PROJECT_ROOT)


def generate_nuttx_romfs(nuttx_root):
    '''
    Create a ROMFS image from the contents of the IoT.js test's root directory.
    '''
    genromfs_flags = ['-f', 'romfs_img', '-d', path.TEST_ROOT]
    xxd_flags = ['-i', 'romfs_img', 'nsh_romfsimg.h']
    sed_flags = ['-ie', 's/unsigned/const\ unsigned/g', 'nsh_romfsimg.h']

    fs.chdir(fs.join(nuttx_root, 'apps/nshlib'))
    ex.check_run_cmd('genromfs', genromfs_flags)
    ex.check_run_cmd('xxd', xxd_flags)
    ex.check_run_cmd('sed', sed_flags)

    fs.chdir(path.PROJECT_ROOT)


if __name__ == '__main__':
    option = parse_option()

    build_args = []

    if option.buildoptions:
        build_args.extend(option.buildoptions.split(','))

    for test in option.test:
        if test == "host-linux":
            for buildtype in option.buildtype:
                build(buildtype, ['--profile=test/profiles/host-linux.profile']
                      + build_args)

        if test == "host-darwin":
            for buildtype in option.buildtype:
                build(buildtype, build_args)

        elif test == "rpi2":
            for buildtype in option.buildtype:
                build(buildtype, ['--target-arch=arm', '--target-board=rpi2',
                                  '--profile=test/profiles/host-darwin.profile']
                      + build_args)

        elif test == "artik10":
            for buildtype in option.buildtype:
                tizen_root = fs.join(path.PROJECT_ROOT, 'deps', 'tizen')
                setup_tizen_root(tizen_root)
                build(buildtype, ['--target-arch=arm',
                                '--target-os=tizen',
                                '--target-board=artik10',
                                '--compile-flag=--sysroot=' + tizen_root,
                                '--profile=test/profiles/tizen.profile']
                      + build_args)

        elif test == "artik053":
            for buildtype in option.buildtype:
                tizenrt_root = fs.join(path.PROJECT_ROOT, 'deps', 'tizenrt')
                setup_tizenrt_repo(tizenrt_root)
                configure_trizenrt(tizenrt_root, buildtype)
                build(buildtype, ['--target-arch=arm',
                                '--target-os=tizenrt',
                                '--target-board=artik05x',
                                '--sysroot=' + tizenrt_root + '/os',
                                '--jerry-heaplimit=128',
                                '--clean',
                                '--profile=test/profiles/tizenrt.profile']
                      + build_args)
                build_tizenrt(tizenrt_root, path.PROJECT_ROOT, buildtype)

        elif test == "nuttx":
            current_dir = os.getcwd()
            for buildtype in option.buildtype:
                nuttx_root=fs.join(path.DEPS_ROOT, 'nuttx')
                setup_nuttx_root(nuttx_root)

                if '--jerry-memstat' in build_args:
                    apply_nuttx_patches(nuttx_root)

                if option.enable_testsuite:
                    generate_nuttx_romfs(nuttx_root)
                    fs.chdir(fs.join(nuttx_root, 'nuttx'))
                    fs.copy(fs.join(path.PROJECT_ROOT,
                                    'config',
                                    'nuttx',
                                    'stm32f4dis',
                                    'config.alloptions'),
                            '.config')
                    fs.chdir(path.PROJECT_ROOT)

                build_nuttx(nuttx_root, buildtype, 'context')
                build(buildtype, ['--target-arch=arm',
                                '--target-os=nuttx',
                                '--nuttx-home=' + fs.join(nuttx_root, 'nuttx'),
                                '--target-board=stm32f4dis',
                                '--jerry-heaplimit=78',
                                '--profile=test/profiles/nuttx.profile']
                      + build_args)
                build_nuttx(nuttx_root, buildtype, 'all')

                # Revert memstat patches after the build.
                if '--jerry-memstat' in build_args:
                     apply_nuttx_patches(nuttx_root, False)

                if option.flash:
                    flash_nuttx(nuttx_root)

                fs.chdir(current_dir)

        elif test == "misc":
            args = []
            if os.getenv('TRAVIS') != None:
                args = ['--travis']
            ex.check_run_cmd('tools/check_signed_off.sh', args)

            if not check_tidy(path.PROJECT_ROOT):
                ex.fail("Failed tidy check")

            build("debug", build_args)
            build("debug", ['--profile=profiles/minimal.profile',
                            '--no-check-test'] + build_args)

        elif test == "no-snapshot":
            args = []
            if os.getenv('TRAVIS') != None:
                args = ['--travis']

            build("debug", ['--no-snapshot', '--jerry-lto'] + build_args)


        elif test == "coverity":
            build("debug", ['--no-check-test'] + build_args)
