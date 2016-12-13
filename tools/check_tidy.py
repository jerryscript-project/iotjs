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

import sys
import os
import fileinput

from check_license import CheckLicenser
from common_py import path
from common_py.system.filesystem import FileSystem as fs
from common_py.system.executor import Executor as ex

TERM_RED = "\033[1;31m"
TERM_YELLOW = "\033[1;33m"
TERM_GREEN = "\033[1;32m"
TERM_BLUE = "\033[1;34m"
TERM_EMPTY = "\033[0m"


column_limit = 80

count_err = 0

interesting_exts = ['.c', '.h', '.js', '.py', '.sh', '.cmake']
clang_format_exts = ['.c', '.h']
skip_dirs = ['deps', 'build']
skip_files = ['check_signed_off.sh', '__init__.py',
              'iotjs_js.c', 'iotjs_js.h', 'iotjs_string_ext.inl.h']


def report_error_name_line(name, line, msg):
    global count_err
    if line is None:
        print("%s: %s" % (name, msg))
    else:
        print("%s:%d: %s" % (name, line, msg))
    count_err += 1


def report_error(msg):
    report_error_name_line(fileinput.filename(), fileinput.filelineno(), msg)


def is_interesting(file):
    _, ext = fs.splitext(file)
    return ext in interesting_exts and file not in skip_files


def is_checked_by_clang(file):
    _, ext = fs.splitext(file)
    return ext in clang_format_exts and file not in skip_files


def check_tidy(src_dir):
    count_lines = 0
    count_empty_lines = 0

    for (dirpath, dirnames, filenames) in os.walk(src_dir):
        if any(d in fs.relpath(dirpath, src_dir) for d in skip_dirs):
            continue

        files = [fs.join(dirpath, name) for name in filenames
                 if is_interesting(name)]

        if not files:
            continue

        for file in files:
            if is_checked_by_clang(file):
                formatted = ex.run_cmd_output(' '.join(['clang-format-3.8',
                                                        '-style=file', file]),
                                              quiet=True)
                f = open(file + '.formatted', 'w')
                f.write(formatted)
                f.close()
                ex.check_run_cmd('diff', [file, file+'.formatted'], quiet=True)
                fs.remove(file + '.formatted')

        for line in fileinput.input(files):
            if '\t' in line:
                report_error('TAB character')
            if '\r' in line:
                report_error('CR character')
            if line.endswith(' \n') or line.endswith('\t\n'):
                report_error('trailing whitespace')
            if not line.endswith('\n'):
                report_error('line ends without NEW LINE character')

            if len(line) - 1 > column_limit:
                report_error('line exceeds %d characters' % column_limit)

            if fileinput.isfirstline():
                if not CheckLicenser.check(fileinput.filename()):
                    report_error_name_line(fileinput.filename(),
                                           None,
                                           'incorrect license')

            count_lines += 1
            if not line.strip():
                count_empty_lines += 1

    print "* total lines of code: %d" % count_lines
    print ("* total non-blank lines of code: %d"
           % (count_lines - count_empty_lines))
    print "%s* total errors: %d%s" % (TERM_RED if count_err > 0 else TERM_GREEN,
                                      count_err,
                                      TERM_EMPTY)
    print

    return count_err == 0

if __name__ == '__main__':
    check_tidy(path.PROJECT_ROOT)
