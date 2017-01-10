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

import subprocess


class Executor(object):
    _TERM_RED = "\033[1;31m"
    _TERM_YELLOW = "\033[1;33m"
    _TERM_GREEN = "\033[1;32m"
    _TERM_BLUE = "\033[1;34m"
    _TERM_EMPTY = "\033[0m"

    @staticmethod
    def cmd_line(cmd, args=[]):
        return ' '.join([cmd] + args)

    @staticmethod
    def print_cmd_line(cmd, args=[]):
        print("%s%s%s" % (Executor._TERM_BLUE, Executor.cmd_line(cmd, args),
                          Executor._TERM_EMPTY))
        print()

    @staticmethod
    def fail(msg):
        print()
        print("%s%s%s" % (Executor._TERM_RED, msg, Executor._TERM_EMPTY))
        print()
        exit(1)

    @staticmethod
    def run_cmd(cmd, args=[], quiet=False):
        if not quiet:
            Executor.print_cmd_line(cmd, args)
        try:
            return subprocess.call([cmd] + args)
        except OSError as e:
            Executor.fail("[Failed - %s] %s" % (cmd, e.strerror))

    @staticmethod
    def run_cmd_output(cmd, quiet=False):
        if not quiet:
            Executor.print_cmd_line(cmd)
        cmd_list = cmd.split()
        try:
            return subprocess.check_output(cmd_list)
        except OSError as e:
            Executor.fail("[Failed - %s] %s" % (cmd, e.strerror))

    @staticmethod
    def check_run_cmd(cmd, args=[], quiet=False):
        retcode = Executor.run_cmd(cmd, args, quiet)
        if retcode != 0:
            Executor.fail("[Failed - %d] %s" % (retcode,
                                                Executor.cmd_line(cmd, args)))
