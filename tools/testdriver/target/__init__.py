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

import host
import stm32f4dis

from common_py import path
from common_py.system.filesystem import FileSystem as fs
from common_py.system.executor import Executor as ex
from common_py.system.platform import Platform


DEVICES = {
    "host": host.Device,
    "stm32f4dis": stm32f4dis.Device
}


def create_build(options):
    if options.target == "host":
        ex.run_cmd(fs.join(path.TOOLS_ROOT, "build.py"), ["--no-check-test"])

        # Append the build path to the options.
        target = "%s-%s" % (Platform().arch(), Platform().os())
        options.bin_path = fs.join(path.BUILD_ROOT, target, "debug/bin/iotjs")

    elif options.target == "stm32f4dis":
        build_options = [
            "--test=nuttx",
            "--buildtype=release",
            "--buildoptions=--jerry-memstat",
            "--enable-testsuite",
            "--flash"
        ]

        ex.run_cmd(fs.join(path.TOOLS_ROOT, "precommit.py"), build_options)

    else:
        ex.fail("Unimplemented case for building iotjs to the target.")


def create_device(options):
    if options.build:
        create_build(options)

    device_class = DEVICES[options.target]

    return device_class(options)
