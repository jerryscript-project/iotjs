#!/usr/bin/env python

# Copyright 2019-present Samsung Electronics Co., Ltd. and other contributors
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

import os
import sys

from common_py import path
from common_py.system.filesystem import FileSystem as fs

from module_generator.source_generator import CSourceGenerator, \
    CppSourceGenerator
from module_generator.clang_translation_unit_visitor import ClangTUVisitor


def generate_c_source(header, api_headers, dirname, args):

    visit_args = []
    if args.define:
        visit_args += ['-D' + defs for defs in args.define]
    if args.defines:
        visit_args += ['-D' + defs for defs in args.defines.read().splitlines()]
    if args.include:
        visit_args += ['-I' + inc for inc in args.include]
    if args.includes:
        visit_args += ['-I' + inc for inc in args.includes.read().splitlines()]

    visitor = ClangTUVisitor(args.lang, header, api_headers, args.check_all,
                             visit_args, args.verbose)
    visitor.visit()

    if args.check or args.check_all:
        visitor.check(visitor)

    if args.lang == 'c':
        generator = CSourceGenerator()
    elif args.lang == 'c++':
        generator = CppSourceGenerator()

    generated_source = [INCLUDE.format(HEADER=dirname + '_js_binding.h')]

    if 'macros' not in args.off:
        generator.macros = visitor.macro_defs

    def visit_namespace(namespace):
        generator.create_ns_obj()
        if 'records' not in args.off:
            for record in namespace.record_decls:
                generated_source.append(generator.create_record(record))

        if 'functions' not in args.off:
            for function in namespace.function_decls:
                generated_source.append(generator.create_ext_function(function))

        if 'enums' not in args.off:
            for decl in namespace.enum_constant_decls:
                generator.enums += decl.enums

        if 'variables' not in args.off:
            for var in namespace.var_decls:
                generated_source.append(generator.create_getter_setter(var))

        generator.create_init_function_body()

        for ns in namespace.namespaces:
            generator.namespace.append(ns.name)
            visit_namespace(ns)
            generator.regist_ns_obj()
            generator.namespace.pop()

    visit_namespace(visitor)

    generated_source.append(generator.create_init_function(dirname))

    return ('\n').join(generated_source)


def generate_header(directory):
    includes = []
    api_headers = []
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith('.h'):
                api_headers.append(os.path.abspath(os.path.join(root, file)))
                includes.append('#include "' +
                                os.path.abspath(os.path.join(root, file)) +
                                '"')

    return ('\n').join(includes), api_headers


def search_for_lib(directory):
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.startswith('lib') and file.endswith('.a'):
                return (root, file)


def generate_module(args):
    directory = args.directory

    if fs.isdir(directory):
        # handle strings end with '/'
        if directory[-1] == '/':
            directory = directory[:-1]

        dirname = fs.basename(directory)
    else:
        sys.exit('Please give an existing directory.')

    if args.out_dir:
        output_dir = args.out_dir
    else:
        output_dir = fs.join(fs.join(path.TOOLS_ROOT, 'module_generator'),
                             'output')

    if not fs.isdir(output_dir):
        os.mkdir(output_dir)

    output_dir = fs.join(output_dir, dirname + '_module')

    if not fs.isdir(output_dir):
        os.mkdir(output_dir)

    src_dir = fs.join(output_dir, 'src')

    if not fs.isdir(src_dir):
        os.mkdir(src_dir)

    header_file = fs.join(src_dir, dirname + '_js_binding.h')
    header_text, api_headers = generate_header(directory)

    with open(header_file, 'w') as h:
        h.write(header_text)

    c_file = generate_c_source(header_file, api_headers, dirname, args)

    extension = 'cpp' if args.lang == 'c++' else 'c'
    with open(fs.join(src_dir, dirname + '_js_binding.' + extension), 'w') as c:
        c.write(c_file)

    library = search_for_lib(directory)

    if not library:
        print ('\033[93mWARNING: Cannot find library file. ' +
               'Only the binding layer source has generated.\033[00m')
        return

    lib_root, lib_name = library
    cmake_file = MODULE_CMAKE.format(NAME=dirname, LIBRARY=lib_name[3:-2])

    with open(fs.join(output_dir, 'module.cmake'), 'w') as cmake:
        cmake.write(cmake_file)

    fs.copyfile(fs.join(lib_root, lib_name), fs.join(output_dir, lib_name))

    json_file = MODULES_JSON.format(NAME=dirname, CMAKE='module.cmake')

    if args.lang == 'c++':
        cmake_lists = CMAKE_LISTS.format(NAME=dirname)
        with open(fs.join(src_dir, 'CMakeLists.txt'), 'w') as cmake:
            cmake.write(cmake_lists)

    with open(fs.join(output_dir, 'modules.json'), 'w') as json:
        json.write(json_file)


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('directory', help='Root directory of the C/C++ API.')

    parser.add_argument('lang', choices=['c', 'c++'],
        help='Specify the language of the API. (default: %(default)s)')

    parser.add_argument('--out-dir', help='Output directory for the module. ' +
                        '(default: tools/module_generator/output)')

    parser.add_argument('--off', choices=['functions', 'variables', 'enums',
                                          'macros', 'records'],
        action='append', default=[], help='Turn off source generating.')

    parser.add_argument('--define', action='append', default=[],
        help='Add macro definition.')
    parser.add_argument('--defines', type=argparse.FileType('r'),
        help='A file, which contains macro definitions.')

    parser.add_argument('--include', action='append', default=[],
        help='Add path to include file.')
    parser.add_argument('--includes', type=argparse.FileType('r'),
        help='A file, which contains paths to include files.')

    parser.add_argument('--check', action='store_true', default=False,
        help='Check the C API headers. Print the unsupported parts.')

    parser.add_argument('--check-all', action='store_true', default=False,
        help='Check the C API headers.')

    parser.add_argument('-v' , '--verbose', action='store_true', default=False,
        help='Print errors, detected by clang.')

    args = parser.parse_args()

    if args.lang == 'c':
        from module_generator.c_source_templates import INCLUDE, MODULES_JSON, \
            MODULE_CMAKE
    elif args.lang == 'c++':
        from module_generator.cpp_source_templates import INCLUDE, \
            MODULES_JSON, MODULE_CMAKE, CMAKE_LISTS

    generate_module(args)
