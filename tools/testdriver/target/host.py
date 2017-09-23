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

import time
import signal
import subprocess

from common_py import path
from common_py.system.filesystem import FileSystem as fs
from common_py.system.executor import Executor as ex
from common_py.system.executor import TimeoutException


# Defines the folder that will contain the coverage info.
# The path must be consistent with the measure_coverage.sh script.
JS_COVERAGE_FOLDER = fs.join(path.PROJECT_ROOT, '.coverage_output')

# This code should be applied to each testfile.
JS_COVERAGE_CODE = (
"""
process.on('exit', function() {{
  if (typeof __coverage__ == 'undefined')
    return;

  if (typeof fs == 'undefined')
    var fs = require('fs');

  if (!fs.existsSync('{folder}'))
    fs.mkdirSync('{folder}');

  var filename = '{folder}/{file}';
  fs.writeFileSync(filename, Buffer(JSON.stringify(__coverage__)));
}})
"""
)


# Append coverage source to the appropriate test.
def append_coverage_code(testfile, coverage):
    if not coverage:
        return

    with open(testfile, 'r') as file_p:
        content = file_p.read()

    with open(testfile, 'w') as file_p:
        file_p.write(JS_COVERAGE_CODE.format(
            folder=JS_COVERAGE_FOLDER, file=fs.basename(testfile)))
        file_p.write(content)


# Remove coverage source from the appropriate test.
def remove_coverage_code(testfile, coverage):
    if not coverage:
        return

    with open(testfile, 'r') as file_p:
        content = file_p.read()
        index = content.find('/* Copyright')

    with open(testfile, 'w') as file_p:
        file_p.write(content[index:])


# Use alarm handler to handle timeout.
def alarm_handler(signum, frame):
    raise TimeoutException


class Device(object):
    """
    Device of the Host target.
    """
    def __init__(self, options):
        if not options.bin_path:
            ex.fail("Path to the iotjs binary is not defined. " +
                    "Use --bin-path to define it.")

        self.iotjs = fs.abspath(options.bin_path)
        self.timeout = options.timeout
        self.coverage = options.coverage
        self.valgrind = options.valgrind

        signal.signal(signal.SIGALRM, alarm_handler)

    def run_test(self, testset, test):
        testfile = fs.join(path.TEST_ROOT, testset, test["name"])
        timeout = test.get("timeout", self.timeout)
        command = [self.iotjs, testfile]

        if self.valgrind:
            valgrind_options = [
                "--leak-check=full",
                "--error-exitcode=5",
                "--undef-value-errors=no"
            ]

            command = ["valgrind"] + valgrind_options + command

        append_coverage_code(testfile, self.coverage)

        signal.alarm(timeout)

        try:
            start_time = time.time()
            process = subprocess.Popen(args=command,
                                       cwd=path.TEST_ROOT,
                                       stdout=subprocess.PIPE,
                                       stderr=subprocess.STDOUT)

            stdout = process.communicate()[0]
            retval = process.returncode
            end_time = time.time()

            signal.alarm(0)

        except TimeoutException:
            process.kill()
            return { "exitcode": -1 }

        finally:
            remove_coverage_code(testfile, self.coverage)

        return {
            "exitcode": retval,
            "stdout" : stdout,
            "runtime": round(end_time - start_time, 2),
            "mempeak" :'n/a'
        }
