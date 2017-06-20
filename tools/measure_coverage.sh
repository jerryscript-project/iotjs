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

print_dep()
{
  echo "The following dependencies are required:"
  echo "    sudo apt install lcov gcc-multilib"
  echo ""
}

print_nvm_dep()
{
  echo "The nvm (node version manager) is required to install node and npm."
  echo "Use the following command to install nvm:"
  echo "    curl \
https://raw.githubusercontent.com/creationix/nvm/v0.33.1/install.sh | bash"
  echo ""
}

print_npm_dep()
{
  echo "The following node dependencies are required: "
  echo "    npm install babel-cli nyc babel-plugin-istanbul merge-source-map"
  echo ""
}

check_architecture()
{
  architecture=$(uname -m)
  case $architecture in
    i386|i686|x86_32)
      ;;
    *)
      echo "Error: You can measure test coverage only on x86 32-bit."
      exit 1
  esac
}

if [ "$#" -gt "0" ] && ( [ "$1" == "-h" ] || [ "$1" == "--help" ] ); then
    echo "Measure JavaScript and C coverage and create a html report"
    echo "out of the results"
    echo ""
    echo "Usage: $0 [NODE_MODULES_DIR]"
    echo ""
    echo "Optional Arguments:"
    echo "  NODE_MODULES_DIR    Specifies the node_module directory, where"
    echo "                      the nodejs dependencies are installed."
    echo ""
    echo "The created html reports can be found in the 'coverage' directory,"
    echo "which will be created in the IoT.js project source dir. The C and"
    echo "JavaScript coverage reports are in the 'c' and 'js' subdirectories"
    echo "respectively. The reports can be viewed by opening the 'index.html'"
    echo "file in a web browser of your choice."
    echo ""
    echo "Running the script will require some additional dependencies."
    echo ""
    print_dep
    print_nvm_dep
    print_npm_dep
    exit 0
fi

# Don't run this script on x86_64 architecture, only on x86_32
check_architecture

tools_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
iotjs_root=$(readlink -f "$tools_dir/..")

test -e ~/.nvm

if [ "$?" -ne "0" ]; then
    print_nvm_dep
    exit 1
fi

# Make available the nvm command.
. ~/.nvm/nvm.sh
. ~/.profile

# Istanbul and babel require node version > 4.0.
nvm install 6.11.0

dpkg -l lcov >> /dev/null 2>&1 && \
dpkg -l gcc-multilib >> /dev/null 2>&1

if [ "$?" -ne "0" ]; then
    print_dep
    exit 1
fi

modules_dir=$(readlink -f "$(npm bin)/..")

if [ "$#" -gt "0" ]; then
    path=$(readlink -f $1)
    if [ ! -d "$path" ] || [ $(basename "$path") != "node_modules" ]; then
        echo "'$1' is not a node_modules directory"
        exit 1
    fi

    test -e $path/.bin/nyc && \
    test -e $path/.bin/babel

    if [ "$?" -ne "0" ]; then
        print_npm_dep
        exit 1
    fi

    modules_dir="$path"
else
    test -e $modules_dir/.bin/nyc && \
    test -e $modules_dir/.bin/babel

    if [ "$?" -ne "0" ]; then
        print_npm_dep
        exit 1
    fi
fi

cd $iotjs_root

# Transpile JS files to provide line counters
$modules_dir/.bin/babel --plugins="babel-plugin-istanbul" \
    src/js/ --out-dir src/cover_js/

# Backup original module files, and replace them with the transpiled files
mv src/js src/orig_js
mv src/cover_js src/js

# Build iot.js
# We need to use the system allocator to have enough memory, for now this can
# only be done with a 32-bit build
tools/build.py --jerry-cmake-param="-DFEATURE_SYSTEM_ALLOCATOR=ON" \
    --target-arch=x86 --compile-flag="-coverage" --no-snapshot --no-check-test

# Run tests
build/i686-linux/debug/bin/iotjs tools/check_test.js -- output-coverage=yes

# Revert to original module files
rm -rf src/js
mv src/orig_js src/js

# Generate js coverage report
mkdir -p .coverage_output
$modules_dir/.bin/nyc report --reporter=lcov \
    --report-dir=coverage --temp-directory=.coverage_output
rm -rf .coverage_output

rm -rf coverage/js
mv coverage/lcov-report coverage/js

# Generate c coverage report
lcov -t "c_coverage" -o ".c-coverage.info" -c -d build/i686-linux/debug/
lcov --remove ".c-coverage.info" 'iotjs/deps/*' -o ".c-coverage.info"
genhtml -o coverage/c .c-coverage.info
rm .c-coverage.info

cd -
