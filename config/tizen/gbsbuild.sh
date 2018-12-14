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

function print_usage {
  echo "USAGE: $0 [--debug|--clean]"
  echo ""
  echo "Optional arguments:"
  echo "--debug: Build IoT.js in debug mode. The default is release mode."
  echo "--clean: Make a clean gbs build by deleting the old build root."
  echo ""
}

buildtype="release"
unset cleanbuild

while [ -n "$1" ]; do
  case $1 in
    --debug )
      buildtype="debug"
    ;;
    --clean )
      cleanbuild=true
    ;;
    * )
      print_usage
      exit 1;
    ;;
  esac
  shift
done

echo "******************************************************************"
echo "*                       Tizen GBS build                          *"
echo "* ~/.gbs.conf sample is at 'config/tizen/sample.gbs.conf'.       *"
echo "******************************************************************"
echo ""
echo "This working folder will be copied to ../iotjs_tizen_gbs"

cd ..
echo copy from $OLDPWD to ../iotjs_tizen_gbs
cp -ra $OLDPWD iotjs_tizen_gbs
cd iotjs_tizen_gbs
echo -e "\n(1) Now, cloning submodules. "
git submodule init
echo -e "\n(2) Update submodules... "
git submodule update

find ./ -name '.git' | xargs rm -rf
# Initialize Git repositoryㅣ
if [ ! -d .git ]
then
  git init ./
  git checkout -b tizen_gbs
  git add ./
  git commit -m "Initial commit"
fi

echo -e "\n(3) Calling core gbs build command"
gbsconf="config/tizen/sample.gbs.conf"
gbscommand="gbs -c $gbsconf build -A armv7l --include-all"
gbscommand+=" ${cleanbuild:+--clean} --define='build_mode $buildtype'"
gbscommand+=" --define='external_build_options $IOTJS_BUILD_OPTION'"

ret=0
echo $gbscommand
if eval $gbscommand
then
  echo "========================================================"
  echo "1. GBS Build is successful."
  echo "2. You can find rpm packages in below folder"
  echo "   ~/GBS-ROOT/local/repos/tizen50m2/armv7l/RPMS"
else
  echo "GBS Build failed!"
  ret=1
fi
cd ..
rm -rf iotjs_tizen_gbs
cd iotjs
exit $ret
