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

import re

from common_py import path
from common_py.system.filesystem import FileSystem as fs

EXCLUDE_DIRS = [
    'test/napi'
]

class CheckLicenser(object):
    _license = re.compile(
        r'((#|//|\*) Copyright .*\n'
        r')+\s?\2\n'
        r'\s?\2 Licensed under the Apache License, Version 2.0 \(the "License"\);\n'
        r'\s?\2 you may not use this file except in compliance with the License.\n'
        r'\s?\2 You may obtain a copy of the License at\n'
        r'\s?\2\n'
        r'\s?\2     http://www.apache.org/licenses/LICENSE-2.0\n'
        r'\s?\2\n'
        r'\s?\2 Unless required by applicable law or agreed to in writing, software\n'
        r'\s?\2 distributed under the License is distributed on an "AS IS" BASIS\n'
        r'\s?\2 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n'
        r'\s?\2 See the License for the specific language governing permissions and\n'
        r'\s?\2 limitations under the License.\n'
    )

    @staticmethod
    def check(filename):
        if any(fs.relpath(filename).startswith(exclude) for exclude in EXCLUDE_DIRS):
            return True

        with open(filename, 'r') as f:
            contents = f.read()
            return bool(CheckLicenser._license.search(contents))
