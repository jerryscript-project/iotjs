#!/bin/bash

# Copyright 2017 Samsung Electronics Co., Ltd. and other contributors
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



IOTPATH="$( cd "$(dirname "$0")/.." ; pwd -P )"
TEST='which npm'
if [ "$?" -ne  0 ] ; then
  echo "NPM is required in order to run ESLint!\nPlease install it."
  exit 2
fi


if ! [ -x "$IOTPATH/node_modules/.bin/eslint" ] ; then
  echo "ESLint not installed!\nYou can install it using:"
  echo "$IOTPATH\$ npm install eslint"
  exit 3
fi

echo "Linting..."
$IOTPATH/node_modules/.bin/eslint -c $IOTPATH/.eslintrc.js $IOTPATH/src/js
