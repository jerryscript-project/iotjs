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

import argparse
import json
import os
import subprocess
import sys

from subprocess import Popen, PIPE, STDOUT


BIN_IOTJS_DEBUG = os.path.join(os.getcwd(),'build/x86_64-linux/debug/bin/iotjs')
BIN_IOTJS_RELEASE = os.path.join(os.getcwd(),
                                        'build/x86_64-linux/release/bin/iotjs')
FLAG_BUILDTYPE = '--buildtype'
FLAG_CLEAN = '--clean'
FLAG_NO_TEST = '--no-check-test'
FLAG_HEAPLIMIT = '--jerry-heaplimit'
IOTJS_BUILDER = os.path.join(os.getcwd(), 'tools/build.py')
TEST_DIR = os.path.join(os.getcwd(), 'test')


def get_args():
    """ Parse input arguments. """
    parser = argparse.ArgumentParser(description='Finds the smallest possible \
                    size of JerryHeap without failing to run the given js file')
    parser.add_argument('testfile')
    parser.add_argument('--heapsize', type = int, default = 256,
                         help = 'Set the limit of the first heapsize')
    parser.add_argument('--buildtype', type = str, default = 'debug',
                                        help = 'Set the jerry buildtype')

    script_args = parser.parse_args()

    return script_args


def is_exe(fpath):
    """ Check whether the file is exist. """
    return os.path.isfile(fpath)


def init(opts):
    if not is_exe(IOTJS_BUILDER):
        sys.exit("Builder script isn't a file!")
    if not is_exe(opts.testfile):
        sys.exit("Testfile isn't a file!")


def build_bin(heapsize, opts):
    """ Run tools/build.py script """
    buildtype = opts.buildtype
    hsize = str(heapsize)
    subprocess.call([IOTJS_BUILDER, FLAG_CLEAN, FLAG_NO_TEST,
                FLAG_BUILDTYPE, buildtype, FLAG_HEAPLIMIT, hsize])


def get_output(cwd, cmd):
    """ Run the given command and return with the output. """
    process = Popen(cmd, stdout=PIPE, stderr=STDOUT, cwd=cwd)

    output = process.communicate()[0]
    exitcode = process.returncode

    return output, exitcode


def run_test(opts):
    """ Run the testfile for output and exitcode. """
    try:
        testfile = os.path.abspath(opts.testfile)
        if opts.buildtype == 'debug':
            run_cmd = [BIN_IOTJS_DEBUG, testfile]
        else:
            run_cmd = [BIN_IOTJS_RELEASE, testfile]
        output, ret = get_output(TEST_DIR, run_cmd)
    except subprocess.CalledProcessError as err:
        output = err.output
        ret = err.returncode

    return opts.testfile, output, ret


def heap_limit(opts):
    """ Find the minimal size of jerryheap to pass """
    goodheap = opts.heapsize
    lowheap = 0
    hiheap = opts.heapsize

    while lowheap < hiheap:
        build_bin(hiheap, opts)
        if opts.buildtype == 'debug':
            assert is_exe(BIN_IOTJS_DEBUG), 'Binary file not exist'
        else:
            assert is_exe(BIN_IOTJS_RELEASE), 'Binary file not exist'

        testfile,  output, exitcode = run_test(opts)
        if exitcode != 0:
            lowheap = hiheap
            hiheap = (lowheap + goodheap) / 2
        else:
            goodheap = hiheap
            hiheap = (lowheap + hiheap) / 2

    return { 'testfile' : testfile, 'heaplimit to pass' : goodheap }


def main(options):
    init(options)
    result = heap_limit(options)
    print(json.dumps(result, indent = 4))


if __name__ == "__main__":
    main(get_args())
