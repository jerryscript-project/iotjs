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
from os import path


SCRIPT_PATH = path.dirname(path.abspath(__file__))

BUILD_SCRIPT = path.join(SCRIPT_PATH, "build.py")

GIT_REPO_FORMAT = 'https://github.com/%s/iotjs.git'


def cmdline(cmd):
    return reduce(lambda x, y: x + " " + y, cmd)


def print_cmdline(cmd):
    print
    print '$ %s' % cmdline(cmd)
    print

def run_cmd_output(cmd):
    print_cmdline(cmd)
    return subprocess.check_output(cmd).strip()


def run_cmd_code(cmd):
    print_cmdline(cmd)
    return subprocess.call(cmd)


def check_run_cmd(cmd):
    print_cmdline(cmd)
    retcode = run_cmd_code(cmd)
    if retcode != 0:
        error_and_exit("Failed - %d" % retcode)


def get_repo_url(fork_name):
    return GIT_REPO_FORMAT % fork_name


def get_merge_branch_name(fork_name, branch_name):
    return '%s-%s' % (fork_name, branch_name)


def git_current_branch():
    return run_cmd_output(['git',
                           'rev-parse',
                           '--abbrev-ref',
                           'HEAD'])

def git_fetch_origin():
    return run_cmd_code(['git', 'fetch', 'origin'])


def git_rebase_origin_master():
    return run_cmd_code(['git', 'rebase', 'origin/master'])


def git_checkout_for_merge(merge_branch):
    return run_cmd_code(['git', 'checkout', '-b', merge_branch, 'master'])


def git_pull_rebase(fork_name, branch_name):
    return run_cmd_code(['git',
                         'pull',
                         '-r',
                         get_repo_url(fork_name), branch_name])


def git_checkout_master():
    return run_cmd_code(['git', 'checkout', 'master'])


def git_rebase_merge(merge_branch):
    return run_cmd_code(['git', 'rebase', merge_branch])


def git_remove_merge(merge_branch):
    return run_cmd_code(['git', 'branch', '-d', merge_branch])


def check_build():
    return run_cmd_code(BUILD_SCRIPT)


def git_push():
    return run_cmd_code(['git', 'push', 'origin', 'master'])


def error_and_exit(msg):
    print
    print '* ' + msg
    print
    exit(1)


if len(sys.argv) < 3:
    error_and_exit('usage: <merge.py> <fork_name> <branch_name>')

fork_name = sys.argv[1]
branch_name = sys.argv[2]

merge_branch = get_merge_branch_name(fork_name, branch_name)


# check if current branch is master.
if git_current_branch() != 'master':
    error_and_exit('You should run merge script on master branch')

# rebase on top of origin/master
if git_fetch_origin() != 0:
    error_and_exit('Failed to fetch origin')

if git_rebase_origin_master() != 0:
    error_and_exit('Failed to rebase origin/master')

# checkout the branch we want to merge.
if git_checkout_for_merge(merge_branch) != 0:
    error_and_exit("Failed to checkout branch for merge")

if git_pull_rebase(fork_name, branch_name) != 0:
    error_and_exit("Failed to pull target branch")

# checkout master
if git_checkout_master() != 0:
    error_and_exit('Failed to checkout master')

# rebase
if git_rebase_merge(merge_branch) != 0:
    error_and_exit("Failed to rebase merge branch")

if git_remove_merge(merge_branch) != 0:
    error_and_exit("Failed to remove merge branch")

# check build
if check_build() != 0:
    error_and_exit("Failed to build check")

# push
if git_push() != 0:
    error_and_exit("Failed to push")


