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

cd ..

echo "******************************************************************"
echo "*                       Tizen GBS build                          *"
echo "*                                                                *"
echo "* Please input user, passwd of http://Tizen.org on '~/.gbs.conf' *"
echo "* For more information, please read Guide Docs. folder           *"
echo "* ~/.gbs.conf sample is at 'config/tizen/sample.gbs.conf'.       *"
echo "*                                                                *"
echo "******************************************************************"
read -p "[Warning] This working folder will be copied to ../iotjs_tizen_gbs \
Are you sure to continue? (y/n) " -n 1 -r

echo ""
if [[ $REPLY =~ ^[Yy]$ ]]
then
  echo copy from $OLDPWD to ../iotjs_tizen_gbs
  cp -ra $OLDPWD iotjs_tizen_gbs
  cd iotjs_tizen_gbs
  echo -e "\n(1) Now, cloning submodules. "
  git submodule init
  echo -e "\n(2) Update submodules... "
  git submodule update
  echo -e "\n(3) Remove compiler specified lines on CMake"
  rm ./cmake/config/arm-tizen.cmake
  echo "include(CMakeForceCompiler)

  set(CMAKE_SYSTEM_NAME Tizen)
  set(CMAKE_SYSTEM_PROCESSOR armv7l)"\
   >> ./cmake/config/arm-tizen.cmake

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

  echo -e "\n(4) 3 files have been modified."

  find ./ -name '.git' | xargs rm -rf
  # Initialize Git repositoryㅣ
  if [ ! -d .git ]
  then
    git init ./
    git checkout -b tizen_gbs
    git add ./
    git commit -m "Initial commit"
  fi

  echo -e "\n(5) Calling core gbs build command"
  gbsconf="config/tizen/sample.gbs.conf"
  gbscommand="gbs -c $gbsconf build -A armv7l --include-all --clean"
  echo $gbscommand
  if eval $gbscommand
  then
    echo "========================================================"
    echo "1. GBS Build is successful."
    echo "2. Please move to new working folder ../iotjs_tizen_gbs "
    echo "   cd ../iotjs_tizen_gbs"
    echo "3. From now, you can build with this command on new directory"
    echo "   gbs build -A armv7l --include"
    echo "4. Your new branch 'tizen_gbs' is added."
    echo "5. 'iotjs origin' repository is added."
    git remote add origin https://github.com/samsung/iotjs
    echo "(You cant fetch origin repository with this command)"
    echo "   git fetch --all"
    echo "========================================================"
    # git remote add origin
    # https://review.tizen.org/git/platform/upstream/iotjs
    git branch -a
    git status
  else
    echo "GBS Build failed!"
    exit 1
  fi
fi
