#!/usr/bin/env python

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

import sys
import os
import fileinput
from check_license import check_license


TERM_RED = "\033[1;31m"
TERM_YELLOW = "\033[1;33m"
TERM_GREEN = "\033[1;32m"
TERM_BLUE = "\033[1;34m"
TERM_EMPTY = "\033[0m"


column_limit = 80

count_err = 0

interesting_exts = ['.c', '.cpp', '.cc', '.h', '.js', '.py', '.sh', '.cmake']
skip_dirs = ['deps', 'build']
skip_files = ['iotjs_js.h']


def report_error_name_line(name, line, msg):
    global count_err
    print("%s:%d: %s" % (name, line, msg))
    count_err += 1


def report_error(msg):
    report_error_name_line(fileinput.filename(), fileinput.filelineno(), msg)


def is_interesting(file):
    _, ext = os.path.splitext(file)
    return ext in interesting_exts and file not in skip_files


def check_tidy(src_dir):
    count_lines = 0
    count_empty_lines = 0

    for (dirpath, dirnames, filenames) in os.walk(src_dir):
        if any(d in dirpath for d in skip_dirs):
            continue

        files = [os.path.join(dirpath, name) for name in filenames
                 if is_interesting(name)]

        if not files:
            continue

        contents = ""
        license_checked = False

        for line in fileinput.input(files):

            if '\t' in line:
                report_error('TAB charactor')
            if '\r' in line:
                report_error('CR charactor')
            if line.endswith(' \n') or line.endswith('\t\n'):
                report_error('trailing whitespace')
            if not line.endswith('\n'):
                report_error('line end without NEW LINE charactor')

            if len(line) -1 > column_limit:
                report_error('line exceeds %d charactors' % column_limit)

            if fileinput.isfirstline():
                contents =""
                license_checked = False

            count_lines += 1
            if not line.strip():
                count_empty_lines += 1

            if len(contents) < 700:
                contents += line
            elif not license_checked:
                if not check_license(contents):
                    report_error_name_line(fileinput.filename(),
                                           1,
                                           'incorrent license')
                license_checked = True

    print "* total line of code: %d" % count_lines
    print ("* total non-black line of code: %d"
           % (count_lines - count_empty_lines))
    print "%s* total errors: %d%s" % (TERM_RED if count_err > 0 else TERM_GREEN,
                                      count_err,
                                      TERM_EMPTY)
    print

    return count_err == 0
