#!/usr/bin/env python

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

import argparse

from testdriver.testrunner import TestRunner
from testdriver.target import DEVICES


def parse_options():
    """
    Parse the given options.
    """
    parser = argparse.ArgumentParser()

    parser.add_argument("--build", default=False, action="store_true",
                        help="create build for the selected target")
    parser.add_argument("--quiet", default=False, action="store_true",
                        help="hide the output of the test execution")
    parser.add_argument("--skip-modules", metavar="LIST", default="",
                        help="""specify the built-in modules that sholuld be
                                skipped (e.g. fs,net,process)""")
    parser.add_argument("--target", default="host", choices=DEVICES.keys(),
                        help="""define the target where the testing happens
                             (default: %(default)s)""")
    parser.add_argument("--timeout", metavar="SEC", default=300, type=int,
                        help="default timeout in sec (default: %(default)s)")

    group = parser.add_argument_group("Local testing")

    group.add_argument("--bin-path", metavar="PATH",
                       help="path to the iotjs binary file")
    group.add_argument("--coverage", default=False, action="store_true",
                       help="""measure JavaScript coverage
                            (only for the meausre_coverage.sh script)""")
    group.add_argument("--valgrind", action="store_true", default=False,
                       help="check memory management by Valgrind")

    group = parser.add_argument_group("Remote testing (SSH communication)")

    group.add_argument("--address", metavar="IPADDR",
                       help="IP address of the device")
    group.add_argument("--remote-path", metavar="PATH",
                       help="""define the folder (absolute path) on the device
                            where iotjs and tests are located""")
    group.add_argument("--username", metavar="USER",
                       help="username to login")

    group = parser.add_argument_group("Remote testing (Serial communication)")

    group.add_argument("--baud", default=115200, type=int,
                       help="baud rate (default: %(default)s)")
    group.add_argument("--port",
                       help="serial port name (e.g. /dev/ttyACM0)")

    return parser.parse_args()


def main():
    options = parse_options()

    testrunner = TestRunner(options)
    testrunner.run()


if __name__ == '__main__':
    main()
