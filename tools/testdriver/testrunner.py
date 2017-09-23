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

import json
import target
from reporter import Reporter

from collections import OrderedDict
from common_py import path
from common_py.system.filesystem import FileSystem as fs
from common_py.system.executor import Executor as ex
from common_py.system.platform import Platform


class TestRunner(object):
    def __init__(self, options):
        #Create target specific build if neccessary.
        self.options = options
        self.quiet = options.quiet
        self.skip_modules = options.skip_modules
        self.device = target.create_device(options)
        self.results = {}

        # Select the appropriate os.
        if options.target == 'host':
            self.os = Platform().os()
        elif options.target == 'stm32f4dis':
            self.os = 'nuttx'

        # Process the iotjs build information.
        iotjs_build_info = { "name": "iotjs_build_info.js" }
        iotjs_output = self.device.run_test(".", iotjs_build_info)
        build_info = json.loads(iotjs_output["stdout"])

        self.builtins = build_info["builtins"]
        self.stability = build_info["stability"]

    def run(self):
        Reporter.report_configuration(self.options)

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
            if self.skip_test(test):
                Reporter.report_skip(test["name"], test.get("reason"))
                self.results["skip"] += 1
                continue

            # Run the test on the device.
            result = self.device.run_test(testset, test)
            expected_failure = test.get("expected-failure", False)

            # Timeout happened.
            if result["exitcode"] == -1:
                Reporter.report_timeout(test["name"])
                self.results["timeout"] += 1
                continue

            # Show the output.
            if not self.quiet:
                print(result["stdout"], end="")

            if (bool(result["exitcode"]) == expected_failure):
                Reporter.report_pass(test["name"], result["runtime"])
                self.results["pass"] += 1
            else:
                Reporter.report_fail(test["name"], result["runtime"])
                self.results["fail"] += 1

    def skip_test(self, test):
        skip_list = test.get("skip", [])

        # Skip by the `skip` attribute in testsets.json file.
        for i in ["all", self.os, self.stability]:
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
