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

import collections
import os
import subprocess

_colors = {
    "empty": "\033[0m",
    "red": "\033[1;31m",
    "green": "\033[1;32m",
    "yellow": "\033[1;33m",
    "blue": "\033[1;34m"
}

if "TERM" not in os.environ:
    # if there is no "TERM" environment variable
    # assume that we can't output colors.
    # So reset the ANSI color escapes.
    _colors = _colors.fromkeys(_colors, "")

_TerminalType = collections.namedtuple('Terminal', _colors.keys())
class _Terminal(_TerminalType):

    def pprint(self, text, color=_colors["empty"]):
        print("%s%s%s" % (color, text, self.empty))

Terminal = _Terminal(**_colors)


class Executor(object):

    @staticmethod
    def cmd_line(cmd, args=[]):
        return ' '.join([cmd] + args)

    @staticmethod
    def print_cmd_line(cmd, args=[]):
        Terminal.pprint(Executor.cmd_line(cmd, args), Terminal.blue)
        print()

    @staticmethod
    def fail(msg):
        print()
        Terminal.pprint(msg, Terminal.red)
        print()
        exit(1)

    @staticmethod
    def run_cmd(cmd, args=[], quiet=False, cwd=None):
        if not quiet:
            Executor.print_cmd_line(cmd, args)
        try:
            return subprocess.call([cmd] + args, cwd=cwd)
        except OSError as e:
            Executor.fail("[Failed - %s] %s" % (cmd, e.strerror))

    @staticmethod
    def run_cmd_output(cmd, args=[], quiet=False):
        if not quiet:
            Executor.print_cmd_line(cmd, args)
        try:
            process = subprocess.Popen([cmd] + args, stdout=subprocess.PIPE)
            output = process.communicate()[0]

            return output
        except OSError as e:
            Executor.fail("[Failed - %s] %s" % (cmd, e.strerror))

    @staticmethod
    def check_run_cmd_output(cmd, args=[], quiet=False):
        if not quiet:
            Executor.print_cmd_line(cmd, args)
        try:
            return subprocess.check_output([cmd] + args)
        except OSError as e:
            Executor.fail("[Failed - %s] %s" % (cmd, e.strerror))

    @staticmethod
    def check_run_cmd(cmd, args=[], quiet=False, cwd=None):
        retcode = Executor.run_cmd(cmd, args, quiet, cwd)
        if retcode != 0:
            Executor.fail("[Failed - %d] %s" % (retcode,
                                                Executor.cmd_line(cmd, args)))
