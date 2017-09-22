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


DOCKER_ROOT_PATH = \
    fs.abspath(fs.join(fs.dirname(__file__), fs.pardir, fs.pardir))
IOTJS_PATH = fs.join(DOCKER_ROOT_PATH, 'iotjs')
BUILD_MODULE_PATH = fs.join(IOTJS_PATH, 'build.module')

DOCKER_NAME = 'iotjs_docker'
BUILDTYPES=['debug', 'release']

def get_config():
    with open(BUILD_MODULE_PATH, 'r') as f:
        config = json.loads(f.read().encode('ascii'))
    return config

def run_docker():
    ex.check_run_cmd('docker', ['run', '-dit', '--name', DOCKER_NAME, '-v',
                     '%s:%s' % (os.environ['TRAVIS_BUILD_DIR'], IOTJS_PATH),
                     'iotjs/ubuntu:0.1'])

def exec_docker(cwd, cmd):
    exec_cmd = 'cd %s && ' % cwd + ' '.join(cmd)
    ex.check_run_cmd('docker', ['exec', '-it', DOCKER_NAME,
                                'bash', '-c', exec_cmd])

if __name__ == '__main__':
    config = get_config()
    os_dependency_module = {}
    extend_module = config['module']['supported']['extended']
    for os_name in extend_module.keys():
        os_dependency_module[os_name] = \
            '--iotjs-include-module=' + ','.join(extend_module[os_name])

    test = os.environ['OPTS']
    if test == 'host-linux':
        run_docker()
        for buildtype in BUILDTYPES:
            exec_docker(IOTJS_PATH, ['./tools/build.py',
                                     '--buildtype=%s' % buildtype])
    elif test == 'rpi2':
        run_docker()
        build_options = ['--clean', '--target-arch=arm', '--target-board=rpi2',
                         os_dependency_module['linux']]
        for buildtype in BUILDTYPES:
            exec_docker(IOTJS_PATH, ['./tools/build.py',
                                     '--buildtype=%s' % buildtype] +
                                     build_options)
