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

import sys
import codecs

def setup_stdio():
    (out_stream, err_stream) = (sys.stdout, sys.stderr)
    if sys.version_info.major >= 3:
        (out_stream, err_stream) = (sys.stdout.buffer, sys.stderr.buffer)
    # For tty using native encoding, otherwise (pipe) use 'utf-8'
    encoding = sys.stdout.encoding if sys.stdout.isatty() else 'utf-8'
    # Always override it to anvoid encode error
    sys.stdout = codecs.getwriter(encoding)(out_stream, 'xmlcharrefreplace')
    sys.stderr = codecs.getwriter(encoding)(err_stream, 'xmlcharrefreplace')
