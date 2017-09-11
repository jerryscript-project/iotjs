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
import signal
import subprocess
import time

from common_py import path
from common_py.system.filesystem import FileSystem as fs
from common_py.system.executor import Executor as ex
from common_py.system.platform import Platform


class Reporter(object):
    @staticmethod
    def message(msg="", color=ex._TERM_EMPTY):
        print("%s%s%s" % (color, msg, ex._TERM_EMPTY))

    @staticmethod
    def report_testset(testset):
        Reporter.message()
        Reporter.message("Testset: %s" % testset, ex._TERM_BLUE)

    @staticmethod
    def report_pass(test, time):
        Reporter.message("  PASS: %s (%ss)" % (test, time), ex._TERM_GREEN)

    @staticmethod
    def report_fail(test, time):
        Reporter.message("  FAIL: %s (%ss)" % (test, time), ex._TERM_RED)

    @staticmethod
    def report_timeout(test):
        Reporter.message("  TIMEOUT: %s" % test, ex._TERM_RED)

    @staticmethod
    def report_skip(test, reason):
        skip_message = "  SKIP: %s" % test

        if reason:
            skip_message += "   (Reason: %s)" % reason

        Reporter.message(skip_message, ex._TERM_YELLOW)

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
        Reporter.message("Finished with all tests:", ex._TERM_BLUE)
        Reporter.message("  PASS:    %d" % results["pass"], ex._TERM_GREEN)
        Reporter.message("  FAIL:    %d" % results["fail"], ex._TERM_RED)
        Reporter.message("  TIMEOUT: %d" % results["timeout"], ex._TERM_RED)
        Reporter.message("  SKIP:    %d" % results["skip"], ex._TERM_YELLOW)


class TimeoutException(Exception):
    pass


def alarm_handler(signum, frame):
    raise TimeoutException


class TestRunner(object):
    def __init__(self, options):
        self.iotjs = options.iotjs
        self.quiet = options.quiet
        self.timeout = options.timeout
        self.valgrind = options.valgrind
        self.skip_modules = []
        self.results = {}

        if options.skip_modules:
            self.skip_modules = options.skip_modules.split(",")

        # Process the iotjs build information.
        iotjs_output = ex.run_cmd_output(self.iotjs, [path.BUILD_INFO_PATH])
        build_info = json.loads(iotjs_output)

        self.builtins = build_info["builtins"]
        self.stability = build_info["stability"]

        # Define own alarm handler to handle timeout.
        signal.signal(signal.SIGALRM, alarm_handler)

    def run(self):
        Reporter.report_configuration(self)

        self.results = {
            "pass": 0,
            "fail": 0,
            "skip": 0,
            "timeout": 0
        }

        with open(fs.join(path.TEST_ROOT, "testsets.json")) as testsets_file:
            testsets = json.load(testsets_file)

        for testset, tests in testsets.items():
            self.run_testset(testset, tests)

        Reporter.report_final(self.results)

    def run_testset(self, testset, tests):
        Reporter.report_testset(testset)

        for test in tests:
            if self.skip_test(test):
                Reporter.report_skip(test["name"], test.get("reason"))
                self.results["skip"] += 1
                continue

            exitcode, output, runtime = self.run_test(testset, test)
            expected_failure = test.get("expected-failure", False)

            # Timeout happened.
            if exitcode == -1:
                Reporter.report_timeout(test["name"])
                self.results["timeout"] += 1
                continue

            if (bool(exitcode) == expected_failure):
                Reporter.report_pass(test["name"], runtime)
                self.results["pass"] += 1
            else:
                Reporter.report_fail(test["name"], runtime)
                self.results["fail"] += 1

            # Show the output.
            if not self.quiet:
                print(output, end="")

    def run_test(self, testset, test):
        timeout = test.get("timeout", self.timeout)
        command = [self.iotjs, fs.join(testset, test["name"])]

        if self.valgrind:
            valgrind_options = [
                "--leak-check=full",
                "--error-exitcode=5",
                "--undef-value-errors=no"
            ]

            command = ["valgrind"] + valgrind_options + command

        signal.alarm(timeout)

        try:
            start = time.time()
            process = subprocess.Popen(args=command,
                                       cwd=path.TEST_ROOT,
                                       stdout=subprocess.PIPE,
                                       stderr=subprocess.STDOUT)

            stdout = process.communicate()[0]
            exitcode = process.returncode
            runtime = round((time.time() - start), 2)

            signal.alarm(0)

        except TimeoutException:
            process.kill()
            return -1, None, None

        return exitcode, stdout, runtime

    def skip_test(self, test):
        skip_list = test.get("skip", [])

        # Skip by the `skip` attribute in testsets.json file.
        for i in ["all", Platform().os(), self.stability]:
            if i in skip_list:
                return True

        name_parts = test["name"][0:-3].split('_')

        # Test filename does not start with 'test_' so we'll just
        # assume we support it.
        if name_parts[0] != 'test':
            return False

        tested_module = name_parts[1]

        # Skip the test if it requires a module that is defined by
        # the `--skip-modules` flag.
        if tested_module in self.skip_modules:
            test["reason"] = "the required module is skipped by testrunner"
            return True

        # Skip the test if it requires a module that is not
        # compiled into the binary.
        if tested_module not in self.builtins:
            test["reason"] = "unsupported module by iotjs build"
            return True

        return False


def get_args():
    parser = argparse.ArgumentParser()

    parser.add_argument("iotjs", action="store",
                        help="path to the iotjs binary file")
    parser.add_argument("--quiet", action="store_true", default=False,
                        help="show or hide the output of the tests")
    parser.add_argument("--skip-modules", action="store", metavar='list',
                        help="module list to skip test of specific modules")
    parser.add_argument("--timeout", action="store", default=300, type=int,
                        help="default timeout for the tests in seconds")
    parser.add_argument("--valgrind", action="store_true", default=False,
                        help="check tests with Valgrind")

    return parser.parse_args()


def main():
    options = get_args()

    testrunner = TestRunner(options)
    testrunner.run()


if __name__ == "__main__":
    main()
