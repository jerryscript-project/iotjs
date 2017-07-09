#!/bin/bash

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
if [ ! -d "iotivity" ]; then
  git clone https://github.com/iotivity/iotivity.git iotivity;
fi

cd iotivity;
git checkout 1.2-rel;

git clone https://github.com/01org/tinycbor.git \
extlibs/tinycbor/tinycbor -b v0.4.1;
git clone https://github.com/ARMmbed/mbedtls.git \
extlibs/mbedtls/mbedtls -b mbedtls-2.4.2;

architecture=$(uname -m)

scons TARGET_ARCH=$architecture octbstack
mkdir out/linux/$architecture/release/include
cp resource/csdk/stack/include/*.h out/linux/$architecture/release/include/
cp resource/c_common/*.h out/linux/$architecture/release/include/
cd ..
./tools/build.py --config=build.iotivity.config
