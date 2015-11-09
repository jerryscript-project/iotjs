#!/usr/bin/env python

# Copyright 2015 Samsung Electronics Co., Ltd.
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
from os import path


SCRIPT_PATH = path.dirname(path.abspath(__file__))

BUILD_SCRIPT_DEBUG = path.join(SCRIPT_PATH,
    'build.py --clean --config=.build.default.config --buildtype=debug')

BUILD_SCRIPT_RELEASE = path.join(SCRIPT_PATH,
    'build.py --clean --config=.build.default.config --buildtype=release')

GIT_REPO_FORMAT = 'https://github.com/%s/iotjs.git'


def print_cmdline(cmd):
    print
    print '\033[1;93m$ %s\033[0m' % cmd
    print

def run_cmd_output(cmd):
    print_cmdline(cmd)
    cmd_list = cmd.split()
    return subprocess.check_output(cmd_list).strip()


def run_cmd_code(cmd):
    print_cmdline(cmd)
    cmd_list = cmd.split()
    return subprocess.call(cmd_list)


def check_run_cmd(cmd):
    retcode = run_cmd_code(cmd)
    if retcode != 0:
        error_and_exit("Failed - %d" % retcode)


def get_repo_url(fork_name):
    return GIT_REPO_FORMAT % fork_name


def get_merge_branch_name(fork_name, branch_name):
    return '%s-%s' % (fork_name, branch_name)


def get_merge_remote_name(fork_name, branch_name):
    return 'remote-%s-%s' % (fork_name, branch_name)


def git_cache_credential():
    return run_cmd_output('git config --global credential.helper cache')


def git_current_branch():
    return run_cmd_output('git rev-parse --abbrev-ref HEAD')


def git_fetch_origin():
    return run_cmd_code('git fetch origin')


def git_rebase_origin_master():
    return run_cmd_code('git rebase origin/master')


def git_check_master():
    return not run_cmd_output('git diff master origin/master')


def git_add_remote(fork_name, branch_name):
    remote_name = get_merge_remote_name(fork_name, branch_name)
    remote_url = get_repo_url(fork_name)
    return run_cmd_code('git remote add %s %s' % (remote_name, remote_url))


def git_fetch_remote(fork_name, branch_name):
    remote_name = get_merge_remote_name(fork_name, branch_name)
    return run_cmd_code('git fetch %s' % remote_name)


def git_checkout_for_merge(fork_name, branch_name):
    merge_branch = get_merge_branch_name(fork_name, branch_name)
    remote_name = get_merge_remote_name(fork_name, branch_name)
    return run_cmd_code('git checkout -b %s %s/%s'
                        % (merge_branch, remote_name, branch_name))


def git_rebase_on_master():
    return run_cmd_code('git rebase master')


def git_checkout_master():
    return run_cmd_code('git checkout master')


def git_merge(merge_branch):
    return run_cmd_code('git merge %s' % merge_branch)


def git_remove_merge(merge_branch):
    return run_cmd_code('git branch -D %s' % merge_branch)


def git_remove_remote(fork_name, branch_name):
    remote_name = get_merge_remote_name(fork_name, branch_name)
    return run_cmd_code('git remote remove %s' % remote_name)


def check_build():
    return (run_cmd_code(BUILD_SCRIPT_DEBUG) or
            run_cmd_code(BUILD_SCRIPT_RELEASE))


def git_push():
    return run_cmd_code('git push origin master')


def error_and_exit(msg):
    print
    print '* ' + msg
    print
    exit(1)


if len(sys.argv) < 3:
    error_and_exit(
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
    error_and_exit('You should run merge script on master branch')


git_cache_credential()


# rebase on top of origin/master
if git_fetch_origin() != 0:
    error_and_exit('Failed to fetch origin')

if git_rebase_origin_master() != 0:
    error_and_exit('Failed to rebase origin/master')

if not git_check_master():
    error_and_exit('master branch is different from origin/master')


# checkout the branch we want to merge.
git_add_remote(fork_name, branch_name)

if git_fetch_remote(fork_name, branch_name) != 0:
    error_and_exit('Failed to fetch remote')

if git_checkout_for_merge(fork_name, branch_name) != 0:
    error_and_exit('Failed to checkout branch for merge')

if git_rebase_on_master() != 0:
    error_and_exit('Failed to rebase on top of master')


# checkout master
if git_checkout_master() != 0:
    error_and_exit('Failed to checkout master')


# rebase
if git_merge(merge_branch) != 0:
    error_and_exit('Failed to rebase merge branch')

git_remove_merge(merge_branch)
git_remove_remote(fork_name, branch_name)


# check build
if check_build() != 0:
    error_and_exit('Failed to build check')

print 'Do you want push [y/N]? '

# push
yes = set(['yes', 'y'])
choise = raw_input().lower()

if choise in yes:
    if git_push() != 0:
        error_and_exit('Failed to push')
