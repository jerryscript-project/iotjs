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
import os
import re
import subprocess
import sys

from common_py import path
from common_py.system.filesystem import FileSystem as fs

def get_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument('--base', required=True,
        help='Path to the base IoT.js binary')
    parser.add_argument('--new',  required=True,
        help='Path to the new IoT.js binary')

    script_args = parser.parse_args()

    return script_args

def run_iotjs(cmd):
    pattern = re.compile(r'Peak allocated = (\d+) bytes')

    try:
        output = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as err:
        return ""

    match = pattern.search(str(output))

    if match:
        return match.group(1)
    else:
        return ""


if __name__ == "__main__":
    script_args = get_arguments()
    print("**JS heap peak (bytes)**\n")
    print("| {0:^40} | {1:^10} | {2:^10} |".format("Test file", "base", "new"))
    print("| {0} | {1} | {2} |".format("-"*40, "-"*10, "-"*10))

    for test_file in os.listdir(path.RUN_PASS_DIR):
        if test_file.endswith(".js"):
            line = "| " + test_file + " | "
            cmd = [script_args.base, '--memstat',
                os.path.join(path.RUN_PASS_DIR, test_file)
            ]
            base_out = run_iotjs(cmd)

            cmd = [script_args.new, '--memstat',
                os.path.join(path.RUN_PASS_DIR, test_file)
            ]
            new_out = run_iotjs(cmd)

            if base_out or new_out:
                print("| {0:40} | {1:^10} | {2:^10} |"
                    .format(test_file, base_out, new_out))
