#!/usr/bin/env python3

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
import multiprocessing
from os import path
from functools import reduce
from enum import Enum


TERM_RED = "\033[1;31m"
TERM_YELLOW = "\033[1;33m"
TERM_GREEN = "\033[1;32m"
TERM_BLUE = "\033[1;34m"
TERM_EMPTY = "\033[0m"


PROCESSES = max(multiprocessing.cpu_count() - 1, 1)
TEST_TIMEOUT = 10


class TestResult(Enum):
    Pass = 1
    Fail = 2
    Skip = 3
    Notest = 4


def join_path(pathes):
    return path.abspath(reduce(lambda x, y: path.join(x, y), pathes))


def check_path(path):
    if not os.path.exists(path):
        return False
    return True

# Path for this script file.
 # should be "<project_home>/tools".
SCRIPT_PATH = path.dirname(path.abspath(__file__))

# Home directory for the project.
ROOT = join_path([SCRIPT_PATH, '../'])

# Root directory for test.
TEST_ROOT = join_path([ROOT, 'test'])

RUN_PASS_DIR = join_path([TEST_ROOT, 'run_pass'])
RUN_FAIL_DIR = join_path([TEST_ROOT, 'run_fail'])
RESOURCE_DIR = join_path([TEST_ROOT, 'resources'])


def is_javascript(name):
    _, ext = os.path.splitext(name)
    return ext == '.js'


def read_file_contents(filepath):
    f = open(filepath, 'r')
    return f.read().encode().decode().strip()


def get_test_attribute(test):
    test_attr = {
      'stdout': '',
      'timeout': TEST_TIMEOUT,
      'skip': False,
      'notest': False,
      'exitcode': (False, 0)
    }

    f = open(test, 'r')
    for line in f.readlines():
        strip = line.strip()

        if strip.startswith('@STDOUT'):
            meta_stdout = strip.split('=')[1]
            if meta_stdout.startswith('FILE['):
                filename = meta_stdout[5:-1]
                test_attr['stdout'] = read_file_contents(
                        join_path([RESOURCE_DIR, filename]))
            elif meta_stdout.startswith('COMMAND['):
                command = meta_stdout[8:-1].split()
                test_attr['stdout'] = subprocess.check_output(command)
                test_attr['stdout'] = test_attr['stdout'].decode().strip()
            else:
                test_attr['stdout'] = meta_stdout

        if strip.startswith('@TIMEOUT'):
            test_attr['timeout'] = int(strip.split('=')[1])

        if strip == '@SKIP':
            test_attr['skip'] = True

        if strip == '@NOTEST':
            test_attr['notest'] = True

        if strip.startswith('@EXITCODE'):
            meta_exitcode = int(strip.split('=')[1])
            test_attr['exitcode'] = (True, meta_exitcode)

    return test_attr


def get_expected_stdout(filename):
    file_path = join_path([RESOURCE_DIR, filename])
    f = open(file_path, 'r')
    return f.read().encode()


# Run a test case
def run_test(arg):
    test_type, iotjs, test = arg
    test_dir = path.dirname(test)

    # fail test ?
    should_fail = True if test_type == 'run_fail' else False

    # working directory
    os.chdir(test_dir)

    # Check test attributes.
    test_attr = get_test_attribute(test)

    if test_attr['skip']:
        print('%s[ %s ] %s - SKIP%s' % (TERM_BLUE,
                                        test_type, os.path.basename(test),
                                        TERM_EMPTY))
        return (TestResult.Skip, test)

    if test_attr['notest']:
        return (TestResult.Notest, test)


    test_output = ''
    exitcode = 0
    attr_mismatched = False
    error_msg = ''
    timeout_expired = False

    # run the test.
    try:
        test_output = subprocess.check_output([iotjs, test],
                                              stderr=subprocess.STDOUT,
                                              timeout=test_attr['timeout'])
        test_output = test_output.decode().strip()
    except subprocess.TimeoutExpired:
        timeout_expired = True
    except Exception as e:
        exitcode = int(e.returncode)
        test_output = e.output.decode().strip()
        error_msg = str(e)

    # check attributes
    if test_attr['stdout'] != '' and test_output != test_attr['stdout']:
        attr_mismatched = True
        error_msg = 'unexpected stdout'

    if test_attr['exitcode'][0] and test_attr['exitcode'][1] != exitcode:
        attr_mismatched = True
        error_msg = ('unexpected exitcode - expected: %d, actuall: %d'
                   % (test_attr['exitcode'][1], exitcode))

    test_failed = False

    # attribute mismatched
    if attr_mismatched:
        test_failed = True

    # timeout expired
    if timeout_expired:
        test_failed = True
        error_msg = 'timed out after %d seconds' % test_attr['timeout']

    # This test should have passed but failed.
    if not should_fail and exitcode != 0:
        test_failed = True

    # This test should have failed but passed.
    if should_fail and exitcode == 0:
        test_failed = True
        if error_msg != '':
            error_msg = 'NOT FAILED'

    if test_failed:
        print('%s[ %s ] %s - %s%s' % (TERM_RED,
                                      test_type,
                                      os.path.basename(test),
                                      error_msg,
                                      TERM_EMPTY))
        print(test_output)
        return (TestResult.Fail, test)

    print('[ %s ] %s' % (test_type, os.path.basename(test)))
    return (TestResult.Pass, test)


def run_tests(testtype, testpath):
    pass_list = []
    fail_list = []
    skip_list = []

    for (dirpath, dirnames, filenames) in os.walk(testpath):
        tests = [join_path([dirpath, name]) for name in filenames
                 if is_javascript(name)]

        pool = multiprocessing.Pool(processes = PROCESSES)
        res = pool.map(run_test, ([testtype, iotjs, test] for test in tests))

        pass_list += list(map(lambda x: x[1],
                              filter(lambda x: x[0] == TestResult.Pass, res)))
        fail_list += list(map(lambda x: x[1],
                              filter(lambda x: x[0] == TestResult.Fail, res)))
        skip_list += list(map(lambda x: x[1],
                              filter(lambda x: x[0] == TestResult.Skip, res)))

    return (pass_list, fail_list, skip_list)


def run_pass_test(iotjs):
    passes, fails, skips = run_tests('run_pass', RUN_PASS_DIR)
    if len(fails) > 0:
        print()
        print('%s[ run_pass ] Test Failed%s' % (TERM_RED, TERM_EMPTY))
        for fail_test in fails:
            print('  * %s' % path.basename(fail_test))
        return False
    return True


def run_fail_test(iotjs):
    passes, fails, skips = run_tests('run_fail', RUN_FAIL_DIR)
    if len(fails) > 0:
        print()
        print('%s[ run_fail] Test Failed%s' % (TERM_RED, TERM_EMPTY))
        for fail_test in fails:
            print('  * %s' % path.basename(fail_test))
        return False
    return True


if len(sys.argv) < 2:
    print('Usage: %s <path for iotjs>' % path.basename(sys.argv[0]))
    exit(1)

iotjs = path.abspath(sys.argv[1])

if not check_path(iotjs):
    print('No iotjs executable: %s' % iotjs)
    exit(1)

tests = [
    {
      'type': 'run_pass',
      'handler': run_pass_test
    },
    {
      'type': 'run_fail',
      'handler': run_fail_test
    }
]

for test in tests:
    print()
    print('%s[ %s ] Test Run%s' % (TERM_YELLOW, test['type'], TERM_EMPTY))
    print()

    if not test['handler'](iotjs):
        exit(1)
