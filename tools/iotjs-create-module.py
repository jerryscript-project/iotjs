#!/usr/bin/env python

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

from __future__ import print_function

import os
import re

IOTJS_BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
TEMPLATE_BASE_DIR = os.path.join(os.path.dirname(__file__), 'module_templates')
MODULE_NAME_RE = "^[a-z0-9][a-z0-9\._]*$"

def load_templates(template_dir):
    for root, dirs, files in os.walk(template_dir):
        for fp in files:
            yield os.path.relpath(os.path.join(root, fp), template_dir)


def replace_contents(input_file, module_name):
    with open(input_file) as fp:
        data = fp.read()
        data = data.replace("$MODULE_NAME$", module_name)
        data = data.replace("$IOTJS_PATH$", IOTJS_BASE_DIR)

    return data

def create_module(output_dir, module_name, template_dir, template_files):
    module_path = os.path.join(output_dir, module_name)
    print("Creating module in {}".format(module_path))

    if os.path.exists(module_path):
        print("Module path ({}) already exists! Exiting".format(module_path))
        return False

    for file_name in template_files:
        file_path = os.path.join(template_dir, file_name)
        print("loading template file: {}".format(file_path))
        contents = replace_contents(file_path, module_name)
        output_path = os.path.join(module_path, file_name)

        # create sub-dir if required
        base_dir = os.path.dirname(output_path)
        if not os.path.exists(base_dir):
            os.mkdir(base_dir)

        with open(output_path, "w") as fp:
            fp.write(contents)

    return True

def valid_module_name(value):
    if not re.match(MODULE_NAME_RE, value):
        msg = "Invalid module name, should match regexp: %s" % MODULE_NAME_RE
        raise argparse.ArgumentTypeError(msg)
    return value

if __name__ == "__main__":
    import argparse
    import sys

    desc = "Create an IoT.js external module using a template"
    parser = argparse.ArgumentParser(description=desc)
    parser.add_argument("module_name", metavar="<MODULE NAME>", nargs=1,
                        type=valid_module_name,
                        help="name of the new module ((must be in lowercase " +
                             "and should match regexp: %s)" % MODULE_NAME_RE)
    parser.add_argument("--path", default=".",
                        help="directory where the module will be created " +
                             "(default: %(default)s)")
    parser.add_argument("--template", default="basic",
                        choices=["basic", "shared"],
                        help="type of the template which should be used "
                        "(default: %(default)s)")
    args = parser.parse_args()


    template_dir = os.path.join(TEMPLATE_BASE_DIR,
                                "%s_module_template" % args.template)
    template_files = load_templates(template_dir)
    created = create_module(args.path,
                            args.module_name[0],
                            template_dir,
                            template_files)
    if created:
        module_path = os.path.join(args.path, args.module_name[0])
        print("Module created in: {}".format(os.path.abspath(module_path)))
