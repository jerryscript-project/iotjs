#!/usr/bin/env python

# Copyright 2015-2016 Samsung Electronics Co., Ltd.
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

import re;

license = re.compile(
u"""((#|//|\*) Copyright .*
)+\s?\\2
\s?\\2 Licensed under the Apache License, Version 2.0 \(the "License"\);
\s?\\2 you may not use this file except in compliance with the License.
\s?\\2 You may obtain a copy of the License at
\s?\\2
\s?\\2     http://www.apache.org/licenses/LICENSE-2.0
\s?\\2
\s?\\2 Unless required by applicable law or agreed to in writing, software
\s?\\2 distributed under the License is distributed on an "AS IS" BASIS
\s?\\2 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
\s?\\2 See the License for the specific language governing permissions and
\s?\\2 limitations under the License.""")


def check_license(contents):
    return bool(license.search(contents))
