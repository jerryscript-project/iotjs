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

if [[ "${TRAVIS_REPO_SLUG}" == "jerryscript-project/iotjs"
          && ${TRAVIS_BRANCH} == "master"
          && ${TRAVIS_EVENT_TYPE} == "push" ]]
then
  git fetch --unshallow
  build-wrapper-linux-x86-64 --out-dir bw-output ./tools/build.py
  sonar-scanner -Dsonar.projectVersion="${TRAVIS_COMMIT}"
else
  echo "Skip: The pull request from ${TRAVIS_PULL_REQUEST_SLUG} is an \
  external one. It's not supported yet in Travis-CI"
fi
