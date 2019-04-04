#!/usr/bin/env python

# Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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
import fileinput
import functools
import os
import subprocess
import tempfile
import re

from distutils import spawn

from check_license import CheckLicenser
from common_py.system.filesystem import FileSystem as fs
from common_py.system.executor import Executor as ex
from common_py.system.executor import Terminal


def parse_option():
    parser = argparse.ArgumentParser()
    parser.add_argument('--autoedit', action='store_true', default=False,
        help='Automatically edit the detected clang format and eslint errors.'
        'No diffs will be displayed')

    option = parser.parse_args()
    return option


class StyleChecker(object):

    column_limit = 80

    def __init__(self):
        self.count_lines = 0
        self.count_empty_lines = 0
        self.errors = []
        self.rules = []
        self.err_msgs = []

    @property
    def error_count(self):
        return len(self.errors)

    @property
    def count_valid_lines(self):
        return self.count_lines - self.count_empty_lines

    def report_error(self, msg):
        name = fileinput.filename()
        line = fileinput.filelineno()
        self.errors.append("%s:%d: %s" % (name, line, msg))

    def set_rules(self):
        limit = StyleChecker.column_limit
        self.rules.append(re.compile(r"[\t]"))
        self.err_msgs.append("TAB character")
        self.rules.append(re.compile(r"[\r]"))
        self.err_msgs.append("CR character")
        self.rules.append(re.compile(r"[ \t]+[\n]$"))
        self.err_msgs.append("Trailing Whitespace")
        self.rules.append(re.compile(r"[^\n]\Z"))
        self.err_msgs.append("Line ends without NEW LINE character")
        self.rules.append(re.compile("^.{" + str(limit+1) + ",}"))
        self.err_msgs.append("Line exceeds %d characters" % limit)
        # append additional rules

    def check(self, files):
        for line in fileinput.input(files):
            for i, rule in enumerate(self.rules):
                mc = rule.search(line)
                if mc:
                    self.report_error(self.err_msgs[i])

            if fileinput.isfirstline():
                if not CheckLicenser.check(fileinput.filename()):
                    self.report_error('incorrect license')

            self.count_lines += 1
            if not line.strip():
                self.count_empty_lines += 1


class ClangFormat(object):

    def __init__(self, extensions, skip_files=None, options=None):
        self.diffs = []
        self._extensions = extensions
        self._skip_files = skip_files
        self._options = options
        self._check_clang_format("clang-format-3.9")

    def _check_clang_format(self, base):
        clang_format = spawn.find_executable(base)

        if not clang_format:
            clang_format = spawn.find_executable("clang-format")
            if clang_format:
                Terminal.pprint(
                    "Using %s instead of %s" % (clang_format, base),
                    Terminal.yellow)
            else:
                Terminal.pprint("No %s found, skipping checks!" % base,
                    Terminal.red)

        self._clang_format = clang_format

    @property
    def error_count(self):
        return len(self.diffs)

    def is_checked_by_clang(self, file):
        _, ext = fs.splitext(file)
        return ext in self._extensions and file not in self._skip_files

    def check(self, files):
        if not self._clang_format:
            return

        for file in filter(self.is_checked_by_clang, files):
            args = ['-style=file', file]
            if self._options and self._options.autoedit:
                args.append('-i')
            output = ex.check_run_cmd_output(self._clang_format,
                                       args, quiet=True)

            if output:
                with tempfile.NamedTemporaryFile() as temp:
                    temp.write(output)
                    temp.flush() # just to be really safe
                    self._diff(file, temp.name)

    def _diff(self, original, formatted):
        try:
            subprocess.check_output(['diff', '-u', original, formatted])
        except subprocess.CalledProcessError as error:
            # if there is a difference between the two files
            # this error will be generated and we can extract
            # the diff from that it. Otherwise nothing to do.
            self.diffs.append(error.output.decode())

class EslintChecker(object):

    def __init__(self, options=None):
        self._check_eslint()
        self._options = options

    def _check_eslint(self):
        self._node = spawn.find_executable('node')
        if not self._node:
            Terminal.pprint('No node found,', Terminal.red)
            return

        self._eslint = spawn.find_executable('node_modules/.bin/eslint')
        if not self._eslint:
            self._eslint = spawn.find_executable('eslint')
            if not self._eslint:
                Terminal.pprint('No eslint found.', Terminal.red)

    def check(self):
        self.error_count = 0

        if not self._node or not self._eslint:
            return
        args = ['src', '-f', 'codeframe']
        if self._options and self._options.autoedit:
             args.append('--fix')

        output = ex.run_cmd_output(self._eslint, args, quiet=True)
        match = re.search('(\d+) error', output)
        if match:
            self.error_count = int(match.group(1))

        # Delete unnecessary error messages.
        self.errors = output.split('\n')[:-4]


class FileFilter(object):

    def __init__(self, allowed_exts, allowed_files, skip_files):
        self._allowed_exts = allowed_exts
        self._allowed_files = allowed_files
        self._skip_files = skip_files

    def __call__(self, dir_path, file):
        if file in self._allowed_files:
            return True

        if file in self._skip_files:
            return False

        _, ext = fs.splitext(file)
        return ext in self._allowed_exts


def check_tidy(src_dir, options=None):
    allowed_exts = ['.c', '.h', '.js', '.py', '.sh', '.cmake']
    allowed_files = ['CMakeLists.txt']
    clang_format_exts = ['.c', '.h']
    skip_dirs = ['deps', 'build', '.git', 'node_modules', 'coverage',
                 'iotjs_modules', 'IoTjsApp']
    skip_files = ['check_license.py', 'check_signed_off.sh', '__init__.py',
                  'iotjs_js.c', 'iotjs_js.h', 'iotjs_string_ext.inl.h',
                  "iotjs_module_inl.h",
                  'ble.js',
                  'ble_hci_socket_acl_stream.js',
                  'ble_hci_socket_smp.js',
                  'ble_hci_socket_hci.js',
                  'ble_hci_socket_gap.js',
                  'ble_hci_socket_gatt.js',
                  'ble_hci_socket_mgmt.js',
                  'ble_hci_socket_bindings.js',
                  'ble_characteristic.js',
                  'node_api.h',
                  'node_api_types.h',
                  'test_ble_setservices.js',
                  '.eslintrc.js',
                  'c_source_templates.py',
                  'cpp_source_templates.py'
                  ]

    style = StyleChecker()
    style.set_rules()
    clang = ClangFormat(clang_format_exts, skip_files, options)
    eslint = EslintChecker(options)

    file_filter = FileFilter(allowed_exts, allowed_files, skip_files)
    files = fs.files_under(src_dir, skip_dirs, file_filter)

    clang.check(files)
    style.check(files)
    eslint.check()

    if clang.error_count:
        print("Detected clang-format problems:")
        print("".join(clang.diffs))
        print()

    if style.error_count:
        print("Detected style problems:")
        print("\n".join(style.errors))
        print()

    if eslint.error_count:
        print("Detected eslint problems:")
        print("\n".join(eslint.errors))
        print()

    total_errors = style.error_count + clang.error_count + eslint.error_count
    print("* total lines of code: %d" % style.count_lines)
    print("* total non-blank lines of code: %d" % style.count_valid_lines)
    print("* style errors: %d" % style.error_count)
    print("* clang-format errors: %d" % clang.error_count)
    print("* eslint errors: %d" % eslint.error_count)

    msg_color = Terminal.red if total_errors > 0 else Terminal.green
    Terminal.pprint("* total errors: %d" % (total_errors), msg_color)
    print()

    if total_errors:
        ex.fail("Failed tidy check")



if __name__ == '__main__':
    from common_py import path
    options = parse_option()
    check_tidy(path.PROJECT_ROOT, options)
