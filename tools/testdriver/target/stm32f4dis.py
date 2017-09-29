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

import re
import time

from connection import serialcom

from common_py import path
from common_py.system.filesystem import FileSystem as fs
from common_py.system.executor import Executor as ex


class Device(object):
    """
    Device of the STM32F4-Discovery target.
    """
    def __init__(self, options):
        if not options.port:
            ex.fail("Serial port is not defined. Use --port to define it.")

        self.serial = serialcom.Connection(options.port, options.baud,
                                           options.timeout, prompt="nsh> ")

    def reset(self):
        ex.run_cmd(fs.join(path.STLINK_ROOT,
                    "build/Release/st-flash"), ["reset"], quiet=True)

        # Wait a moment to boot the device.
        time.sleep(5)

    def login(self):
        try:
            self.serial.open()

            # Press enters to start the serial communication and
            # go to the test folder because some tests require resources.
            self.serial.exec_command("\n\n")
            self.serial.exec_command("cd /test")

        except Exception as e:
            ex.fail(str(e))

    def logout(self):
        self.serial.close()

    def run_test(self, testset, test):
        self.reset()
        self.login()

        command = "iotjs --memstat /test/%s/%s" % (testset, test["name"])

        start_time = time.time()
        stdout = self.serial.exec_command(command.encode("utf8"))
        retval = self.serial.exec_command("echo $?")
        end_time = time.time()

        self.logout()

        # Process the stdout of the execution.
        if stdout.rfind("Heap stat") != -1:
            stdout, heap = stdout.rsplit("Heap stats", 1)

            match = re.search(r"Peak allocated = (\d+) bytes", str(heap))

            if match:
                mempeak = match.group(1)
            else:
                mempeak = "n/a"
        else:
            mempeak = "n/a"

        return {
            "exitcode": int(retval),
            "stdout" : stdout,
            "runtime": round(end_time - start_time, 2),
            "mempeak" : mempeak
        }
