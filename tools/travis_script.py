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

from common_py.system.filesystem import FileSystem as fs
from common_py.system.executor import Executor as ex
from common_py.system.sys_platform import Platform
from check_tidy import check_tidy

platform = Platform()

DOCKER_ROOT_PATH = fs.join('/root')

# IoT.js path in travis
TRAVIS_BUILD_PATH = fs.join(os.environ['TRAVIS_BUILD_DIR'])

# IoT.js path in docker
DOCKER_IOTJS_PATH = fs.join(DOCKER_ROOT_PATH, 'work_space/iotjs')

# Node server path in docker
DOCKER_NODE_SERVER_PATH = fs.join(DOCKER_ROOT_PATH, 'work_space/node_server')

DOCKER_TIZENRT_PATH = fs.join(DOCKER_ROOT_PATH, 'TizenRT')
DOCKER_TIZENRT_OS_PATH = fs.join(DOCKER_TIZENRT_PATH, 'os')
DOCKER_TIZENRT_OS_TOOLS_PATH = fs.join(DOCKER_TIZENRT_OS_PATH, 'tools')

DOCKER_NUTTX_PATH = fs.join(DOCKER_ROOT_PATH, 'nuttx')
DOCKER_NUTTX_TOOLS_PATH = fs.join(DOCKER_NUTTX_PATH, 'tools')
DOCKER_NUTTX_APPS_PATH = fs.join(DOCKER_ROOT_PATH, 'apps')

DOCKER_NAME = 'iotjs_docker'
DOCKER_TAG = 'iotjs/ubuntu:0.10'
BUILDTYPES = ['debug', 'release']
TIZENRT_TAG = '2.0_Public_M2'

# Common buildoptions for sanitizer jobs.
BUILDOPTIONS_SANITIZER = [
    '--buildtype=debug',
    '--clean',
    '--compile-flag=-fno-common',
    '--compile-flag=-fno-omit-frame-pointer',
    '--jerry-cmake-param=-DJERRY_SYSTEM_ALLOCATOR=ON',
    '--no-check-valgrind',
    '--no-snapshot',
    '--profile=test/profiles/host-linux.profile',
    '--run-test=full',
    '--target-arch=i686'
]

def start_container():
    run_docker()
    start_mosquitto_server()
    start_node_server()

def run_docker():
    ex.check_run_cmd('docker', ['pull', DOCKER_TAG])
    ex.check_run_cmd('docker', ['run', '-dit', '--privileged',
                     '--name', DOCKER_NAME, '-v',
                     '%s:%s' % (TRAVIS_BUILD_PATH, DOCKER_IOTJS_PATH),
                     '--add-host', 'test.mosquitto.org:127.0.0.1',
                     '--add-host', 'echo.websocket.org:127.0.0.1',
                     '--add-host', 'httpbin.org:127.0.0.1',
                     DOCKER_TAG])

def exec_docker(cwd, cmd, env=[], is_background=False):
    exec_cmd = 'cd %s && ' % cwd + ' '.join(cmd)
    if is_background:
        docker_args = ['exec', '-dit']
    else:
        docker_args = ['exec', '-it']

    for e in env:
        docker_args.append('-e')
        docker_args.append(e)

    docker_args += [DOCKER_NAME, 'bash', '-c', exec_cmd]
    ex.check_run_cmd('docker', docker_args)

def start_mosquitto_server():
    exec_docker(DOCKER_ROOT_PATH, ['mosquitto', '-d'])

def start_node_server():
    exec_docker(DOCKER_NODE_SERVER_PATH, ['node', 'server.js'], [], True)

def set_config_tizenrt(buildtype):
    exec_docker(DOCKER_ROOT_PATH, [
                'cp',
                fs.join(DOCKER_IOTJS_PATH,
                        'config/tizenrt/artik05x/configs/',
                        buildtype, 'defconfig'),
                fs.join(DOCKER_TIZENRT_OS_PATH, '.config')])

def build_iotjs(buildtype, args=[], env=[]):
    exec_docker(DOCKER_IOTJS_PATH, [
                './tools/build.py',
                '--clean',
                '--buildtype=' + buildtype] + args, env)

JOBS = dict()

class job(object):
    def __init__(self, name):
        self.name = name
    def __call__(self, fn):
        JOBS[self.name] = fn

@job('host-linux')
def job_host_linux():
    start_container()

    for buildtype in BUILDTYPES:
        build_iotjs(buildtype, [
                    '--cmake-param=-DENABLE_MODULE_ASSERT=ON',
                    '--run-test=full',
                    '--profile=profiles/minimal.profile'])

    for buildtype in BUILDTYPES:
        build_iotjs(buildtype, [
                    '--run-test=full',
                    '--profile=test/profiles/host-linux.profile'])

@job('n-api')
def job_n_api():
    start_container()

    # N-API should work with both ES5.1 and ES2015-subset JerryScript profiles
    for buildtype in BUILDTYPES:
        build_iotjs(buildtype, [
                    '--run-test=full',
                    '--n-api'])

    for buildtype in BUILDTYPES:
        build_iotjs(buildtype, [
                    '--run-test=full',
                    '--n-api',
                    '--jerry-profile=es2015-subset'])

@job('mock-linux')
def job_mock_linux():
    start_container()
    for buildtype in BUILDTYPES:
        build_iotjs(buildtype, [
                    '--run-test=full',
                    '--target-os=mock',
                    '--profile=test/profiles/mock-linux.profile'])

@job('rpi2')
def job_rpi2():
    start_container()
    for buildtype in BUILDTYPES:
        build_iotjs(buildtype, [
                    '--target-arch=arm',
                    '--target-board=rpi2',
                    '--profile=test/profiles/rpi2-linux.profile'])
@job('stm32f4dis')
def job_stm32f4dis():
    start_container()

    # Copy the application files to apps/system/iotjs.
    exec_docker(DOCKER_ROOT_PATH, [
                'cp', '-r',
                fs.join(DOCKER_IOTJS_PATH,'config/nuttx/stm32f4dis/app/'),
                fs.join(DOCKER_NUTTX_APPS_PATH, 'system/iotjs/')])

    exec_docker(DOCKER_ROOT_PATH, [
                'cp', '-r',
                fs.join(DOCKER_IOTJS_PATH,
                        'config/nuttx/stm32f4dis/config.travis'),
                fs.join(DOCKER_NUTTX_PATH,
                        'configs/stm32f4discovery/usbnsh/defconfig')])

    for buildtype in BUILDTYPES:
        exec_docker(DOCKER_NUTTX_PATH, ['make', 'distclean'])
        exec_docker(DOCKER_NUTTX_TOOLS_PATH,
                    ['./configure.sh', 'stm32f4discovery/usbnsh'])
        exec_docker(DOCKER_NUTTX_PATH, ['make', 'clean'])
        exec_docker(DOCKER_NUTTX_PATH, ['make', 'context'])
        # Build IoT.js
        build_iotjs(buildtype, [
                    '--target-arch=arm',
                    '--target-os=nuttx',
                    '--nuttx-home=' + DOCKER_NUTTX_PATH,
                    '--target-board=stm32f4dis',
                    '--jerry-heaplimit=78',
                    '--profile=test/profiles/nuttx.profile'])
        # Build Nuttx
        if buildtype == "release":
            rflag = 'R=1'
        else:
            rflag = 'R=0'
        exec_docker(DOCKER_NUTTX_PATH, [
                    'make', 'all',
                    'IOTJS_ROOT_DIR=' + DOCKER_IOTJS_PATH, rflag])

@job('tizen')
def job_tizen():
    start_container()
    for buildtype in BUILDTYPES:
        if buildtype == "debug":
            exec_docker(DOCKER_IOTJS_PATH, [
                        'config/tizen/gbsbuild.sh',
                        '--debug', '--clean'])
        else:
            exec_docker(DOCKER_IOTJS_PATH, ['config/tizen/gbsbuild.sh',
                        '--clean'])

@job('misc')
def job_misc():
    ex.check_run_cmd('tools/check_signed_off.sh', ['--travis'])
    ex.check_run_cmd('tools/check_tidy.py')

@job('external-modules')
def job_external_modules():
    start_container()
    for buildtype in BUILDTYPES:
        build_iotjs(buildtype, [
                    '--run-test=full',
                    '--profile=test/profiles/host-linux.profile',
                    '--external-modules=test/external_modules/'
                    'mymodule1,test/external_modules/mymodule2',
                    '--cmake-param=-DENABLE_MODULE_MYMODULE1=ON',
                    '--cmake-param=-DENABLE_MODULE_MYMODULE2=ON'])

@job('es2015')
def job_es2015():
    start_container()
    for buildtype in BUILDTYPES:
        build_iotjs(buildtype, [
                    '--run-test=full',
                    '--jerry-profile=es2015-subset'])

@job('no-snapshot')
def job_no_snapshot():
    start_container()
    for buildtype in BUILDTYPES:
        build_iotjs(buildtype, ['--run-test=full', '--no-snapshot',
                                '--jerry-lto'])

@job('host-darwin')
def job_host_darwin():
    for buildtype in BUILDTYPES:
        ex.check_run_cmd('./tools/build.py', [
                         '--run-test=full',
                         '--buildtype=' + buildtype,
                         '--clean',
                         '--profile=test/profiles/host-darwin.profile'])

@job('asan')
def job_asan():
    start_container()
    build_iotjs('debug', [
                '--compile-flag=-fsanitize=address',
                '--compile-flag=-O2'
                ] + BUILDOPTIONS_SANITIZER,
                ['ASAN_OPTIONS=detect_stack_use_after_return=1:'
                'check_initialization_order=true:strict_init_order=true',
                'TIMEOUT=600'])

@job('ubsan')
def job_ubsan():
    start_container()
    build_iotjs('debug', [
                '--compile-flag=-fsanitize=undefined'
                ] + BUILDOPTIONS_SANITIZER,
                ['UBSAN_OPTIONS=print_stacktrace=1', 'TIMEOUT=600'])

@job('coverity')
def job_coverity():
    ex.check_run_cmd('./tools/build.py', ['--clean'])

if __name__ == '__main__':
    JOBS[os.getenv('OPTS')]()
