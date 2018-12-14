#!/bin/bash
ROOT=`pwd`

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
echo "*                       Tizen release script                     *"
echo "******************************************************************"

repo=$1

if [ "$repo" == "../iotjs_tizen" -o "$repo" == "../iotjs_tizen/" ]; then
    echo "Syncing with: tizen iotjs"
else
    echo "Usage: $0 [ ../iotjs_tizen ]"
    exit 0
fi

if [ ! -d ../iotjs_tizen ]; then
    # echo "cloning..."
    echo "Error: $repo not exist"
    exit 0
fi

cd ..
echo copy from $OLDPWD to ../iotjs_tizen_org
cp -ra $OLDPWD iotjs_tizen_org
cd iotjs_tizen_org

echo -e "\n(1) Now, cloning submodules. "
git submodule init

echo -e "\n(2) Update submodules... "
git submodule update

echo -e "\n(3) Modify version... "
hash=`git log | head -1 | cut -f2 -d' ' | cut -c 1-7`
today=`date +%y%m%d`
sed -i "s/\(IOTJS_VERSION \".*\"\)/\1 \"$today\_$hash\"/g" src/iotjs_def.h

echo -e "\n(4) Patch for tizen.org... "
patch -p1 < config/tizen/iotjs_tizen.patch
cp -ra config/tizen/packaging .

merge_filter="merge config/tizen/filter.txt"
rsync -av --delete --delete-excluded --filter="$merge_filter" . $repo

cd $repo

git add -A
echo "======================================="
echo git commit -m "IOTJS_Release_$today""_$hash"
echo "======================================="
msg="IOTJS_Release_$today""_$hash"
git commit -m "$msg"
cd $ROOT

rm -rf ../iotjs_tizen_org
