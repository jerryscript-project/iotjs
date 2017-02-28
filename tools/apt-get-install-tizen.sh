#!/bin/bash

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

wget https://releases.linaro.org/components/\
toolchain/binaries/4.9-2017.01/arm-linux-gnueabi/\
gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabi.tar.xz
tar Jxf gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabi.tar.xz
mv gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabi arm-linux-gnueabi
export PATH=$(pwd)/arm-linux-gnueabi/bin:$PATH
arm-linux-gnueabi-gcc --version
