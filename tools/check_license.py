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

import re;

license = re.compile(
u"""(#|//|\*) Copyright 2015 Samsung Electronics Co., Ltd.
\s?\\1
\s?\\1 Licensed under the Apache License, Version 2.0 \(the "License"\);
\s?\\1 you may not use this file except in compliance with the License.
\s?\\1 You may obtain a copy of the License at
\s?\\1
\s?\\1     http://www.apache.org/licenses/LICENSE-2.0
\s?\\1
\s?\\1 Unless required by applicable law or agreed to in writing, software
\s?\\1 distributed under the License is distributed on an "AS IS" BASIS
\s?\\1 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
\s?\\1 See the License for the specific language governing permissions and
\s?\\1 limitations under the License.""")


def check_license(contents):
    return bool(license.search(contents))
