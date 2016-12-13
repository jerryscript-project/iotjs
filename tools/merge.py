#!/usr/bin/env python

# Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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


import sys
import subprocess
import getpass

from common_py.system.filesystem import FileSystem as fs
from common_py.system.executor import Executor as ex

SCRIPT_PATH = fs.dirname(fs.abspath(__file__))

BUILD_SCRIPT_DEBUG = fs.join(SCRIPT_PATH,
    'build.py --clean --config=.build.default.config --buildtype=debug')

BUILD_SCRIPT_RELEASE = fs.join(SCRIPT_PATH,
    'build.py --clean --config=.build.default.config --buildtype=release')

GIT_REPO_FORMAT = 'https://github.com/%s/iotjs.git'


def get_repo_url(fork_name):
    return GIT_REPO_FORMAT % fork_name


def get_merge_branch_name(fork_name, branch_name):
    return '%s-%s' % (fork_name, branch_name)


def get_merge_remote_name(fork_name, branch_name):
    return 'remote-%s-%s' % (fork_name, branch_name)


def git_cache_credential():
    return ex.run_cmd_output('git config --global credential.helper cache')


def git_current_branch():
    return ex.run_cmd_output('git rev-parse --abbrev-ref HEAD')


def git_fetch_origin():
    return ex.run_cmd('git fetch origin')


def git_rebase_origin_master():
    return ex.run_cmd('git rebase origin/master')


def git_check_master():
    return not ex.run_cmd_output('git diff master origin/master')


def git_add_remote(fork_name, branch_name):
    remote_name = get_merge_remote_name(fork_name, branch_name)
    remote_url = get_repo_url(fork_name)
    return ex.run_cmd('git remote add %s %s' % (remote_name, remote_url))


def git_fetch_remote(fork_name, branch_name):
    remote_name = get_merge_remote_name(fork_name, branch_name)
    return ex.run_cmd('git fetch %s' % remote_name)


def git_checkout_for_merge(fork_name, branch_name):
    merge_branch = get_merge_branch_name(fork_name, branch_name)
    remote_name = get_merge_remote_name(fork_name, branch_name)
    return ex.run_cmd('git checkout -b %s %s/%s'
                      % (merge_branch, remote_name, branch_name))


def git_rebase_on_master():
    return ex.run_cmd('git rebase master')


def git_checkout_master():
    return ex.run_cmd('git checkout master')


def git_merge(merge_branch):
    return ex.run_cmd('git merge %s' % merge_branch)


def git_remove_merge(merge_branch):
    return ex.run_cmd('git branch -D %s' % merge_branch)


def git_remove_remote(fork_name, branch_name):
    remote_name = get_merge_remote_name(fork_name, branch_name)
    return ex.run_cmd('git remote remove %s' % remote_name)


def check_build():
    return (ex.run_cmd(BUILD_SCRIPT_DEBUG) or
            ex.run_cmd(BUILD_SCRIPT_RELEASE))


def git_push():
    return ex.run_cmd('git push origin master')


if len(sys.argv) < 3:
    ex.fail(
        'usage: <merge.py> <fork_name> <branch_name> [<id>]')

fork_name = sys.argv[1]
branch_name = sys.argv[2]
git_id = ""
git_passwd = ""

if len(sys.argv) >= 4:
    git_id = sys.argv[3]
    git_passwd = getpass.getpass()


merge_branch = get_merge_branch_name(fork_name, branch_name)


# check if current branch is master.
if git_current_branch() != 'master':
    ex.fail('You should run merge script on master branch')


git_cache_credential()


# rebase on top of origin/master
if git_fetch_origin() != 0:
    ex.fail('Failed to fetch origin')

if git_rebase_origin_master() != 0:
    ex.fail('Failed to rebase origin/master')

if not git_check_master():
    ex.fail('master branch is different from origin/master')


# checkout the branch we want to merge.
git_add_remote(fork_name, branch_name)

if git_fetch_remote(fork_name, branch_name) != 0:
    ex.fail('Failed to fetch remote')

if git_checkout_for_merge(fork_name, branch_name) != 0:
    ex.fail('Failed to checkout branch for merge')

if git_rebase_on_master() != 0:
    ex.fail('Failed to rebase on top of master')


# checkout master
if git_checkout_master() != 0:
    ex.fail('Failed to checkout master')


# rebase
if git_merge(merge_branch) != 0:
    ex.fail('Failed to rebase merge branch')

git_remove_merge(merge_branch)
git_remove_remote(fork_name, branch_name)


# check build
if check_build() != 0:
    ex.fail('Failed to build check')

print 'Do you want push [y/N]? '

# push
yes = set(['yes', 'y'])
choise = raw_input().lower()

if choise in yes:
    if git_push() != 0:
        ex.fail('Failed to push')
