#!/bin/bash

# Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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

echo "******************************************************************"
echo "*                       Tizen patch for obs build                *"
echo "******************************************************************"
echo ""
echo "This working folder will be copied to ../iotjs_tizen_org"

cd ..
echo copy from $OLDPWD to ../iotjs_tizen_org
cp -ra $OLDPWD iotjs_tizen_org
cd iotjs_tizen_org

echo -e "\n(1) Now, cloning submodules. "
git submodule init

echo -e "\n(2) Update submodules... "
git submodule update

echo -e "\n(3) remove .git folders.. "
find ./ -name '.git' | xargs rm -rf

# Initialize Git repository
if [ ! -d .git ]
then
  git init ./
  git checkout -b tizen_gbs
  git add ./
  git commit -m "Initial commit"
fi


echo -e "\n(4) Patch for tizen.org... "
patch -p1 < config/tizen/iotjs_tizen.patch
cp -ra config/tizen/packaging .

