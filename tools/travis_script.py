#!/usr/bin/env python

# Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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
import json

from common_py.system.filesystem import FileSystem as fs
from common_py.system.executor import Executor as ex


DOCKER_ROOT_PATH = fs.join('/root')

# IoT.js path in travis
TRAVIS_BUILD_PATH = fs.join(os.environ['TRAVIS_BUILD_DIR'])

# IoT.js path in docker
DOCKER_IOTJS_PATH = fs.join(DOCKER_ROOT_PATH, 'iotjs')

DOCKER_TIZENRT_PATH = fs.join(DOCKER_ROOT_PATH, 'TizenRT')
DOCKER_TIZENRT_OS_PATH = fs.join(DOCKER_TIZENRT_PATH, 'os')
DOCKER_TIZENRT_OS_TOOLS_PATH = fs.join(DOCKER_TIZENRT_OS_PATH, 'tools')

IOTJS_BUILD_MODULE_PATH = fs.join(TRAVIS_BUILD_PATH, 'build.module')

DOCKER_NAME = 'iotjs_docker'

BUILDTYPES = ['debug', 'release']

def get_config():
    with open(IOTJS_BUILD_MODULE_PATH, 'r') as file:
        config = json.loads(file.read().encode('ascii'))
    return config

def run_docker():
    ex.check_run_cmd('docker', ['run', '-dit', '--name', DOCKER_NAME, '-v',
                     '%s:%s' % (TRAVIS_BUILD_PATH, DOCKER_IOTJS_PATH),
                     'iotjs/ubuntu:0.3'])

def exec_docker(cwd, cmd):
    exec_cmd = 'cd %s && ' % cwd + ' '.join(cmd)
    ex.check_run_cmd('docker', ['exec', '-it', DOCKER_NAME,
                                'bash', '-c', exec_cmd])

def set_release_config_tizenrt():
    exec_docker(DOCKER_ROOT_PATH, ['cp', 'tizenrt_release_config',
                                   fs.join(DOCKER_TIZENRT_OS_PATH, '.config')])

if __name__ == '__main__':
    # Get os dependency module list
    target_os = os.environ['TARGET_OS']
    extend_module = get_config()['module']['supported']['extended']
    dependency_module_option = \
        '--iotjs-include-module=' + ','.join(extend_module[target_os])

    run_docker()

    test = os.environ['OPTS']
    if test == 'host-linux':
        for buildtype in BUILDTYPES:
            exec_docker(DOCKER_IOTJS_PATH, ['./tools/build.py',
                                     '--buildtype=%s' % buildtype])
    elif test == 'rpi2':
        build_options = ['--clean', '--target-arch=arm', '--target-board=rpi2',
                         dependency_module_option]
        for buildtype in BUILDTYPES:
            exec_docker(DOCKER_IOTJS_PATH, ['./tools/build.py',
                                            '--buildtype=%s' % buildtype] +
                                            build_options)

    elif test == 'artik053':
        # Update latest commit
        exec_docker(DOCKER_TIZENRT_PATH, ['git', 'pull'])
        # Set configure
        exec_docker(DOCKER_TIZENRT_OS_TOOLS_PATH, ['./configure.sh',
                                                   'artik053/iotjs'])

        for buildtype in BUILDTYPES:
            if buildtype == 'release':
                set_release_config_tizenrt()
            exec_docker(DOCKER_TIZENRT_OS_PATH,
                        ['make', 'IOTJS_ROOT_DIR=../../iotjs',
                        'IOTJS_BUILD_OPTION=' + dependency_module_option])
