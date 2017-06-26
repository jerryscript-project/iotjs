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

from distutils import spawn

from check_license import CheckLicenser
from common_py.system.filesystem import FileSystem as fs
from common_py.system.executor import Executor as ex


def parse_option():
    parser = argparse.ArgumentParser()
    parser.add_argument('--autoedit', action='store_true', default=False,
        help='Automatically edit the detected clang format errors.'
        'No diffs will be displayed')

    option = parser.parse_args()
    return option


class StyleChecker(object):

    column_limit = 80

    def __init__(self):
        self.count_lines = 0
        self.count_empty_lines = 0
        self.errors = []

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

    def check(self, files):
        for line in fileinput.input(files):
            if '\t' in line:
                self.report_error('TAB character')
            if '\r' in line:
                self.report_error('CR character')
            if line.endswith(' \n') or line.endswith('\t\n'):
                self.report_error('trailing whitespace')
            if not line.endswith('\n'):
                self.report_error('line ends without NEW LINE character')

            if len(line) - 1 > StyleChecker.column_limit:
                self.report_error('line exceeds %d characters'
                                  % StyleChecker.column_limit)

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
        self._check_clang_format("clang-format-3.8")

    def _check_clang_format(self, base):
        clang_format = spawn.find_executable(base)

        if not clang_format:
            clang_format = spawn.find_executable("clang-format")
            if clang_format:
                print("%sUsing %s instead of %s%s"
                      % (ex._TERM_YELLOW, clang_format, base, ex._TERM_EMPTY))
            else:
                print("%sNo %s found, skipping checks!%s"
                      % (ex._TERM_RED, base, ex._TERM_EMPTY))

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
            output = ex.run_cmd_output(self._clang_format,
                                       args,
                                       quiet=True)

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
    skip_dirs = ['deps', 'build', '.git', 'node_modules', 'coverage']
    skip_files = ['check_signed_off.sh', '__init__.py',
                  'iotjs_js.c', 'iotjs_js.h', 'iotjs_string_ext.inl.h',
                  'ble.js',
                  'ble_hci_socket_acl_stream.js',
                  'ble_hci_socket_smp.js',
                  'ble_hci_socket_hci.js',
                  'ble_hci_socket_gap.js',
                  'ble_hci_socket_gatt.js',
                  'ble_hci_socket_mgmt.js',
                  'ble_hci_socket_bindings.js',
                  'ble_characteristic.js',
                  'test_ble_setservices.js',
                  ]

    style = StyleChecker()
    clang = ClangFormat(clang_format_exts, skip_files, options)

    file_filter = FileFilter(allowed_exts, allowed_files, skip_files)
    files = fs.files_under(src_dir, skip_dirs, file_filter)

    clang.check(files)
    style.check(files)

    if clang.error_count:
        print("Detected clang-format problems:")
        print("".join(clang.diffs))
        print()

    if style.error_count:
        print("Detected style problems:")
        print("\n".join(style.errors))
        print()

    total_errors = style.error_count + clang.error_count
    print("* total lines of code: %d" % style.count_lines)
    print("* total non-blank lines of code: %d" % style.count_valid_lines)
    print("* style errors: %d" % style.error_count)
    print("* clang-format errors: %d" % clang.error_count)

    msg_color = ex._TERM_RED if total_errors > 0 else ex._TERM_GREEN
    print("%s* total errors: %d%s" % (msg_color, total_errors, ex._TERM_EMPTY))
    print()

    return total_errors == 0



if __name__ == '__main__':
    from common_py import path
    options = parse_option()
    check_tidy(path.PROJECT_ROOT, options)
