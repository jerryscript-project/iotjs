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

from __future__ import print_function

import argparse
import json
import multiprocessing
import os
import subprocess
import sys
import time

try:
    import queue
except ImportError:
    # Backwards compatibility
    import Queue as queue


from collections import OrderedDict
from common_py import path
from common_py.system.filesystem import FileSystem as fs
from common_py.system.executor import Executor
from common_py.system.executor import Terminal
from common_py.system.sys_platform import Platform

# Defines the folder that will contain the coverage info.
# The path must be consistent with the measure_coverage.sh script.
JS_COVERAGE_FOLDER = fs.join(path.PROJECT_ROOT, '.coverage_output')

# This code should be applied to each testfile.
JS_COVERAGE_CODE = (
"""
process.on('exit', function() {{
  if (typeof __coverage__ == 'undefined')
    return;

  if (typeof fs == 'undefined')
    var fs = require('fs');

  if (!fs.existsSync('{folder}'))
    fs.mkdirSync('{folder}');

  var filename = '{folder}/{file}';
  fs.writeFileSync(filename, Buffer(JSON.stringify(__coverage__)));
}})
"""
)

# Append coverage source to the appropriate test.
def append_coverage_code(testfile, coverage):
    if not coverage:
        return

    with open(testfile, 'r') as file_p:
        content = file_p.read()

    with open(testfile, 'w') as file_p:
        file_p.write(JS_COVERAGE_CODE.format(
            folder=JS_COVERAGE_FOLDER, file=fs.basename(testfile)))
        file_p.write(content)


# Remove coverage source from the appropriate test.
def remove_coverage_code(testfile, coverage):
    if not coverage:
        return

    with open(testfile, 'r') as file_p:
        content = file_p.read()
        index = content.find('/* Copyright')

    with open(testfile, 'w') as file_p:
        file_p.write(content[index:])


class Reporter(object):
    @staticmethod
    def message(msg="", color=Terminal.empty):
        print("%s%s%s" % (color, msg, Terminal.empty))

    @staticmethod
    def report_testset(testset):
        Reporter.message()
        Reporter.message("Testset: %s" % testset, Terminal.blue)

    @staticmethod
    def report_pass(test, time):
        Reporter.message("  PASS: %s (%ss)" % (test, time), Terminal.green)

    @staticmethod
    def report_fail(test, time):
        Reporter.message("  FAIL: %s (%ss)" % (test, time), Terminal.red)

    @staticmethod
    def report_timeout(test):
        Reporter.message("  TIMEOUT: %s" % test, Terminal.red)

    @staticmethod
    def report_skip(test, reason):
        skip_message = "  SKIP: %s" % test

        if reason:
            skip_message += "   (Reason: %s)" % reason

        Reporter.message(skip_message, Terminal.yellow)

    @staticmethod
    def report_configuration(testrunner):
        Reporter.message()
        Reporter.message("Test configuration:")
        Reporter.message("  iotjs:        %s" % testrunner.iotjs)
        Reporter.message("  quiet:        %s" % testrunner.quiet)
        Reporter.message("  timeout:      %d sec" % testrunner.timeout)
        Reporter.message("  valgrind:     %s" % testrunner.valgrind)
        Reporter.message("  skip-modules: %s" % testrunner.skip_modules)

    @staticmethod
    def report_final(results):
        Reporter.message()
        Reporter.message("Finished with all tests:", Terminal.blue)
        Reporter.message("  PASS:    %d" % results["pass"], Terminal.green)
        Reporter.message("  FAIL:    %d" % results["fail"], Terminal.red)
        Reporter.message("  TIMEOUT: %d" % results["timeout"], Terminal.red)
        Reporter.message("  SKIP:    %d" % results["skip"], Terminal.yellow)


class TestRunner(object):
    def __init__(self, options):
        self._process_pool = multiprocessing.Pool(processes=1)
        self.iotjs = fs.abspath(options.iotjs)
        self.quiet = options.quiet
        self.platform = options.platform
        self.timeout = options.timeout
        self.valgrind = options.valgrind
        self.coverage = options.coverage
        self.skip_modules = []
        self.results = {}
        self._msg_queue = multiprocessing.Queue(1)

        if options.skip_modules:
            self.skip_modules = options.skip_modules.split(",")

        # Process the iotjs build information.
        iotjs_output = Executor.check_run_cmd_output(self.iotjs,
                                                     [path.BUILD_INFO_PATH])
        build_info = json.loads(iotjs_output)

        self.builtins = set(build_info["builtins"])
        self.features = set(build_info["features"])
        self.stability = build_info["stability"]
        self.debug = build_info["debug"]
        self.arch = build_info["arch"]
        if "napi" in self.builtins:
            self.build_napi_test_module()

    def build_napi_test_module(self):
        node_gyp = fs.join(path.PROJECT_ROOT,
                           'node_modules',
                           '.bin',
                           'node-gyp')

        print('==> Build N-API test module with node-gyp\n')

        project_root = fs.join(path.PROJECT_ROOT, 'test', 'napi')
        cmd = ['--debug'] if self.debug else ['--release']
        if self.platform == 'windows':
            node_gyp += '.cmd'
            cmd.append('--arch=x64' if self.arch == 'x64' else '--arch=ia32')
        Executor.check_run_cmd(node_gyp, ['rebuild'] + cmd,
                               cwd=project_root)

    def run(self):
        Reporter.report_configuration(self)

        self.results = {
            "pass": 0,
            "fail": 0,
            "skip": 0,
            "timeout": 0
        }

        with open(fs.join(path.TEST_ROOT, "testsets.json")) as testsets_file:
            testsets = json.load(testsets_file, object_pairs_hook=OrderedDict)

        for testset, tests in testsets.items():
            self.run_testset(testset, tests)

        Reporter.report_final(self.results)

    def run_testset(self, testset, tests):
        Reporter.report_testset(testset)

        for test in tests:
            testfile = fs.join(path.TEST_ROOT, testset, test["name"])
            timeout = test.get("timeout", self.timeout)

            if self.skip_test(test):
                Reporter.report_skip(test["name"], test.get("reason"))
                self.results["skip"] += 1
                continue

            append_coverage_code(testfile, self.coverage)

            exitcode, output, runtime = self.run_test(testfile, timeout)
            expected_failure = test.get("expected-failure", False)

            remove_coverage_code(testfile, self.coverage)

            # Timeout happened.
            if exitcode == -1:
                Reporter.report_timeout(test["name"])
                self.results["timeout"] += 1
                continue

            # Show the output.
            if not self.quiet and output:
                print(output.decode("utf8"), end="")

            is_normal_run = (not expected_failure and exitcode == 0)
            is_expected_fail = (expected_failure and exitcode in [1, 2])
            if is_normal_run or is_expected_fail:
                Reporter.report_pass(test["name"], runtime)
                self.results["pass"] += 1
            else:
                Reporter.report_fail(test["name"], runtime)
                self.results["fail"] += 1

    def run_test(self, testfile, timeout):
        command = [self.iotjs, testfile]

        if self.valgrind:
            valgrind_options = [
                "--leak-check=full",
                "--error-exitcode=5",
                "--undef-value-errors=no"
            ]

            command = ["valgrind"] + valgrind_options + command

        try:
            process = multiprocessing.Process(target=run_subprocess,
                                              args=(self._msg_queue, command))
            start = time.time()
            process.start()
            process.join(timeout)
            runtime = round((time.time() - start), 2)

            if process.is_alive():
                raise multiprocessing.TimeoutError("Test still running")

            # At this point the queue must have data!
            # If not then it is also a timeout event
            exitcode, stdout = self._msg_queue.get_nowait()

        except (multiprocessing.TimeoutError, queue.Full):
            process.terminate()
            return -1, None, None

        return exitcode, stdout, runtime

    def skip_test(self, test):
        skip_list = set(test.get("skip", []))

        # Skip by the `skip` attribute in testsets.json file.
        for i in ["all", self.platform, self.stability]:
            if i in skip_list:
                return True

        required_modules = set(test.get("required-modules", []))
        required_features = set(test.get("required-features", []))

        unsupported_modules = required_modules - self.builtins
        unsupported_features = required_features - self.features
        skipped_modules = required_modules.intersection(skip_list)

        # Skip the test if the tested module requires a module
        # which is not compiled into the binary
        if unsupported_modules:
            test["reason"] = "Required module(s) unsupported by iotjs build: "
            test["reason"] += ', '.join(sorted(unsupported_modules))
            return True

        # Skip the test if it requires a module that is skipped by the
        # testrunner
        if skipped_modules:
            test["reason"] = "Required module(s) skipped by testrunner: "
            test["reason"] += ', '.join(sorted(skipped_modules))
            return True

        # Skip the test if it uses features which are
        # unavailable in the current iotjs build
        if unsupported_features:
            test["reason"] = "Required feature(s) unsupported by iotjs build: "
            test["reason"] += ', '.join(sorted(unsupported_features))
            return True

        return False


def run_subprocess(parent_queue, command):
    process = subprocess.Popen(args=command,
                               cwd=path.TEST_ROOT,
                               stdout=subprocess.PIPE,
                               stderr=subprocess.STDOUT)

    stdout = process.communicate()[0]
    exitcode = process.returncode

    parent_queue.put_nowait([exitcode, stdout])

def get_args():
    parser = argparse.ArgumentParser()

    parser.add_argument("iotjs", action="store",
                        help="path to the iotjs binary file")
    parser.add_argument('--platform', default=Platform().os(),
                        help='Specify the platform (default: %(default)s)')
    parser.add_argument("--quiet", action="store_true", default=False,
                        help="show or hide the output of the tests")
    parser.add_argument("--skip-modules", action="store", metavar='list',
                        help="module list to skip test of specific modules")
    parser.add_argument("--timeout", action="store", default=300, type=int,
                        help="default timeout for the tests in seconds")
    parser.add_argument("--valgrind", action="store_true", default=False,
                        help="check tests with Valgrind")
    parser.add_argument("--coverage", action="store_true", default=False,
                        help="measure JavaScript coverage")

    return parser.parse_args()


def main():
    options = get_args()

    testrunner = TestRunner(options)
    testrunner.run()
    if testrunner.results["fail"]:
        sys.exit(1)

if __name__ == "__main__":
    main()
