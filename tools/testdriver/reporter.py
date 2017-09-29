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

from common_py.system.executor import Executor as ex


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
    def report_configuration(options):
        Reporter.message()
        Reporter.message("Test configuration:")
        Reporter.message("  target:        %s" % options.target)

        if options.target == "host":
            Reporter.message("  bin path:      %s" % options.bin_path)
            Reporter.message("  coverage:      %s" % options.coverage)
            Reporter.message("  valgrind:      %s" % options.valgrind)
        elif options.target == "rpi2":
            Reporter.message("  address:       %s" % options.address)
            Reporter.message("  username:      %s" % options.username)
            Reporter.message("  remote path:   %s" % options.remote_path)
        elif options.target in ["stm32f4dis", "artik053"]:
            Reporter.message("  port:          %s" % options.port)
            Reporter.message("  baud:          %d" % options.baud)

        Reporter.message("  quiet:         %s" % options.quiet)
        Reporter.message("  timeout:       %s" % options.timeout)
        Reporter.message("  skip-modules:  %s" % repr(options.skip_modules))

    @staticmethod
    def report_final(results):
        Reporter.message()
        Reporter.message("Finished with all tests:", ex._TERM_BLUE)
        Reporter.message("  PASS:    %d" % results["pass"], ex._TERM_GREEN)
        Reporter.message("  FAIL:    %d" % results["fail"], ex._TERM_RED)
        Reporter.message("  TIMEOUT: %d" % results["timeout"], ex._TERM_RED)
        Reporter.message("  SKIP:    %d" % results["skip"], ex._TERM_YELLOW)
