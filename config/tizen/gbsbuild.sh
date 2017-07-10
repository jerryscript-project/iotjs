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

read -p "The '.git' folder will be removed. \
Are you sure to continue? (y/n) " -n 1 -r
echo    # (optional) move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then

echo `pwd`
git submodule init
git submodule update

echo "Modify cmake, remote compiler specified lines"
rm ./cmake/config/arm-tizen.cmake
echo"include(CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR armv7l)" >> ./cmake/config/arm-tizen.cmake

rm ./deps/libtuv/cmake/config/config_arm-tizen.cmake
echo "include(CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)"\
 >> ./deps/libtuv/cmake/config/config_arm-tizen.cmake

rm ./deps/jerry/cmake/toolchain_linux_armv7l-el.cmake
echo "set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR armv7l-el)
set(FLAGS_COMMON_ARCH -mlittle-endian -mthumb)" \
>> ./deps/jerry/cmake/toolchain_linux_armv7l-el.cmake

echo "3 files have been modified."

rm -rf .git/
# Initialize Git repositoryㅣ
if [ ! -d .git ]; then
   git init ./
   git checkout -b tizen_gbs
   git add ./
   git commit -m "Initial commit"
fi

echo "Calling core gbs build command"
gbscommand="gbs build -A armv7l --include-all"
echo $gbscommand
if eval $gbscommand; then
    echo "GBS Build is successful"
    #git remote add origin https://review.tizen.org/git/platform/upstream/iotjs
    #git remote add iotjs https://github.com/samsung/iotjs
    #git fetch --all
else
    echo "GBS Build failed!"
    exit 1
fi

fi
