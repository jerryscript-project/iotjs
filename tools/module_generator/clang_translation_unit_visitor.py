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

from clang.cindex import Index, conf, CursorKind, TypeKind, AccessSpecifier

# This class is a wrapper for the TypeKind and Type classes.
class ClangASTNodeType:
    char_type_kinds = [
        TypeKind.CHAR_U,
        TypeKind.CHAR16,
        TypeKind.CHAR32,
        TypeKind.CHAR_S,
        TypeKind.WCHAR
    ]

    number_type_kinds = [
        TypeKind.UCHAR,
        TypeKind.SCHAR,
        TypeKind.USHORT,
        TypeKind.UINT,
        TypeKind.ULONG,
        TypeKind.ULONGLONG,
        TypeKind.UINT128,
        TypeKind.SHORT,
        TypeKind.INT,
        TypeKind.LONG,
        TypeKind.LONGLONG,
        TypeKind.INT128,
        TypeKind.FLOAT,
        TypeKind.DOUBLE,
        TypeKind.LONGDOUBLE
    ]

    def __init__(self, clang_type):
        # We are only interested in the underlying canonical types.
        self._canonical_type = clang_type.get_canonical()
        self._type_name = clang_type.spelling.replace('const ', '')

    @property
    def name(self):
        return self._type_name

    @property
    def canonical_name(self):
        return self._canonical_type.spelling

    def is_bool(self):
        return self._canonical_type.kind == TypeKind.BOOL

    def is_char(self):
        return self._canonical_type.kind in ClangASTNodeType.char_type_kinds

    def is_number(self):
        return self._canonical_type.kind in ClangASTNodeType.number_type_kinds

    def is_enum(self):
        return self._canonical_type.kind == TypeKind.ENUM

    def is_void(self):
        return self._canonical_type.kind == TypeKind.VOID

    def is_pointer(self):
        return (self._canonical_type.kind == TypeKind.POINTER or
                self._canonical_type.kind == TypeKind.CONSTANTARRAY or
                self._canonical_type.kind == TypeKind.INCOMPLETEARRAY)

    def is_array(self):
        return (self._canonical_type.kind == TypeKind.CONSTANTARRAY or
                self._canonical_type.kind == TypeKind.INCOMPLETEARRAY)

    def is_record(self):
        return self._canonical_type.kind == TypeKind.RECORD

    def is_struct(self):
        return (self._canonical_type.get_declaration().kind ==
                CursorKind.STRUCT_DECL)

    def is_union(self):
        return (self._canonical_type.get_declaration().kind ==
                CursorKind.UNION_DECL)

    def is_function(self):
        return (self._canonical_type.kind == TypeKind.FUNCTIONPROTO or
                self._canonical_type.kind == TypeKind.FUNCTIONNOPROTO)

    def is_const(self):
        return self._canonical_type.is_const_qualified()

    def get_array_type(self):
        return ClangASTNodeType(self._canonical_type.get_array_element_type())

    def get_array_size(self):
        assert self.is_array()
        return self._canonical_type.element_count

    def get_pointee_type(self):
        if self.is_array():
            array_type = self._canonical_type.get_array_element_type()
            return ClangASTNodeType(array_type)
        if self.is_pointer():
            return ClangASTNodeType(self._canonical_type.get_pointee())

    def get_declaration(self):
        return ClangASTNode(self._canonical_type.get_declaration())

    def get_as_record_decl(self):
        assert (self.is_record())
        return ClangRecordDecl(self._canonical_type.get_declaration())

    def has_const_member(self):
        ret = False
        decl = self._canonical_type.get_declaration()
        for child in decl.get_children():
            if child.kind == CursorKind.FIELD_DECL:
                if child.type.get_canonical().kind == TypeKind.RECORD:
                    ret = ClangASTNodeType(child.type).has_const_member()
                if child.type.is_const_qualified():
                    ret = True
        return ret


# This class is a wrapper for the Cursor type.
class ClangASTNode:
    def __init__(self, cursor):
        self._cursor = cursor
        self._type = ClangASTNodeType(cursor.type)
        self._kind = cursor.kind

    @property
    def name(self):
        return self._cursor.spelling

    @property
    def type(self):
        return self._type

    @property
    def kind(self):
        return self._kind

    def get_as_record_decl(self):
        assert (self.type.is_record())
        return ClangRecordDecl(self._cursor)

    def get_as_function(self):
        return ClangFunctionDecl(self._cursor)


# This class represents enum declarations in libclang.
class ClangEnumDecl:
    def __init__(self, cursor):
        self._enum_constant_decls = []

        for child in cursor.get_children():
            if child.kind == CursorKind.ENUM_CONSTANT_DECL:
                self._enum_constant_decls.append(child.spelling)

    @property
    def enums(self):
        return self._enum_constant_decls


# This class represents function declarations in libclang.
class ClangFunctionDecl(ClangASTNode):
    def __init__(self, cursor):
        ClangASTNode.__init__(self, cursor)

        if cursor.type.get_canonical().kind == TypeKind.POINTER:
            return_type = cursor.type.get_canonical().get_pointee().get_result()
        else:
            return_type = cursor.type.get_canonical().get_result()

        self._return_type = ClangASTNodeType(return_type)

        self._parm_decls = []
        if cursor.type.kind == TypeKind.TYPEDEF:
            children = cursor.type.get_declaration().get_children()
        else:
            children = cursor.get_children()

        for arg in children:
            if arg.kind == CursorKind.PARM_DECL:
                arg = ClangASTNode(arg)
                self._parm_decls.append(arg)

    @property
    def return_type(self):
        return self._return_type

    @property
    def params(self):
        return self._parm_decls


# This class represents macro definitions in libclang.
# TokenKinds:
#    'PUNCTUATION' = 0
#    'KEYWORD' = 1
#    'IDENTIFIER' = 2
#    'LITERAL' = 3
#    'COMMENT' = 4
class ClangMacroDef(ClangASTNode):
    def __init__(self, cursor):
        ClangASTNode.__init__(self, cursor)

        self.tokens = []
        self.token_kinds = []
        for token in cursor.get_tokens():
            self.tokens.append(token.spelling)
            self.token_kinds.append(token.kind.value)

    @property
    def content(self):
        return (' ').join(self.tokens[1:])

    def is_char(self):
        if (self.token_kinds == [2, 3] and # "#define CH 'a'" like macros
            "'" in self.tokens[1]): # char literal
                return True
        return False

    def is_string(self):
        if (self.token_kinds == [2, 3] and # '#define STR "abc"' like macros
            '"' in self.tokens[1]): # string literal
                return True
        return False

    # macro contains only number literals and punctuations
    def is_number(self):
        if (self.content and
            not [x for x in self.token_kinds[1:] if x in [1, 2, 4]] and
            "'" not in self.content and '"' not in self.content):
            return True
        return False

    def is_valid(self):
        return self.is_char() or self.is_string() or self.is_number()

    def is_function(self):
        return conf.lib.clang_Cursor_isMacroFunctionLike(self._cursor)


class ClangRecordConstructor:
    def __init__(self, cursor_list, is_constructor = True, func_list = []):
        self._suffix = ''
        if cursor_list:
            self._cursor = cursor_list[0]

        self._parm_decls = {}
        param_lists = []
        for i, cursor in enumerate(cursor_list):
            arguments = list(cursor.get_arguments())
            param_lists.append(([ClangASTNode(a) for a in arguments], i))

            # handle default arguments
            for arg in reversed(arguments):
                if '=' in [t.spelling for t in arg.get_tokens()]:
                    arguments = arguments[:-1]
                    param_lists.append(([ClangASTNode(a) for a in arguments],
                                        i))

        # Codes:
        # String - 0
        # Number - 1
        # Boolean - 2
        # TypedArray - 3
        # Function - 4
        # Object - name of the object
        # Other - 5
        coded_param_lists = []
        for param_list, _ in param_lists:
            coded_params = []
            for param in param_list:
                param_t = param.type
                if (param_t.is_char() or
                    (param_t.is_pointer() and
                        param_t.get_pointee_type().is_char())):
                    coded_params.append(0)
                elif param_t.is_number() or param_t.is_enum():
                    coded_params.append(1)
                elif param_t.is_bool():
                    coded_params.append(2)
                elif param_t.is_function():
                    coded_params.append(4)
                elif param_t.is_record():
                    record = param_t.get_as_record_decl().ns_name
                    coded_params.append(record)
                elif param_t.is_pointer():
                    pointee_t = param_t.get_pointee_type()
                    if pointee_t.is_char():
                        coded_params.append(0)
                    elif pointee_t.is_number():
                        coded_params.append(3)
                    elif pointee_t.is_function():
                        coded_params.append(4)
                    elif pointee_t.is_record():
                        record = pointee_t.get_as_record_decl().ns_name
                        coded_params.append(record)
                    else:
                        coded_params.append(5)
                else:
                    coded_params.append(5)
            coded_param_lists.append(coded_params)

        # Remove lists from `param_lists`,
        # which have the same JS types of parameters
        j = 0
        same_types_params = []
        for i, coded_params in enumerate(coded_param_lists):
            if coded_params in coded_param_lists[:i] + coded_param_lists[i+1:]:
                same_types_params.append(param_lists.pop(j))
                j -= 1
            j += 1

        for param_list, _ in param_lists:
            if len(param_list) in self._parm_decls:
                self._parm_decls[len(param_list)].append(param_list)
            else:
                self._parm_decls[len(param_list)] = [param_list]

        for j, (params, i) in enumerate(same_types_params):
            if is_constructor:
                f = ClangRecordConstructor([cursor_list[i]])
            else:
                f = ClangRecordMethod(self._cursor.spelling, [cursor_list[i]])
            f._suffix = '_$' + str(j)
            func_list.append(f)
            func_name = cursor_list[i].spelling
            print ('\033[93mWARN: The following overload of ' + func_name +
                   ' has been renamed to ' + func_name + f._suffix +
                   ' :\033[00m')
            print ' '.join(t.spelling for t in cursor_list[i].get_tokens())

    @property
    def params(self):
        return self._parm_decls

    @property
    def suffix(self):
        return self._suffix


class ClangRecordMethod(ClangRecordConstructor):
    def __init__(self, name, cursor_list, func_list = []):
        ClangRecordConstructor.__init__(self, cursor_list, False, func_list)

        self._method_name = name
        return_type = cursor_list[0].type.get_canonical().get_result()
        self._return_type = ClangASTNodeType(return_type)

    @property
    def name(self):
        return self._method_name

    @property
    def return_type(self):
        return self._return_type


# This class represents struct/union/class declarations in libclang.
class ClangRecordDecl(ClangASTNode):
    def __init__(self, cursor):
        ClangASTNode.__init__(self, cursor)

        self._field_decls = []
        self._has_constructor = True
        self._has_default_constructor = True
        self._has_copy_constructor = True
        self._constructors = []
        if cursor.spelling:
            self._name = cursor.spelling
        else:
            self._name = self.type.name.split('::')[-1]

        constructors = []
        methods = {}
        for child in cursor.get_children():
            if child.access_specifier == AccessSpecifier.PUBLIC:
                if child.kind == CursorKind.CONSTRUCTOR:
                    constructors.append(child)
                if child.kind == CursorKind.FIELD_DECL:
                    self._field_decls.append(ClangASTNode(child))
                if child.kind == CursorKind.CXX_METHOD:
                    if child.spelling in methods:
                        methods[child.spelling].append(child)
                    else:
                        methods[child.spelling] = [child]

        if not constructors and self.type.has_const_member():
            self._has_constructor = False
            self._has_default_constructor = False
            self._has_copy_constructor = False
        elif constructors:
            constructor = ClangRecordConstructor(constructors, True,
                                                 self._constructors)
            if constructor.params:
                self._constructors.append(constructor)
        self._methods = []
        for name, cursor_list in methods.items():
            method = ClangRecordMethod(name, cursor_list, self._methods)
            if method.params:
                self._methods.append(method)

    @property
    def name(self):
        return self._name

    @property
    def ns_name(self):
        return self.type.name.replace('::', '_')

    @property
    def constructors(self):
        return self._constructors

    def has_constructor(self):
        return self._has_constructor

    def has_default_constructor(self):
        return self._has_default_constructor

    def has_copy_constructor(self):
        return self._has_copy_constructor

    @property
    def field_decls(self):
        return self._field_decls

    @property
    def methods(self):
        return self._methods


class ClangNamespace:
    def __init__(self, name, cursor_list):
        self.name = name
        self.enum_constant_decls = []
        self.function_decls = []
        self.var_decls = []
        self.record_decls = []
        self.namespaces = []

        cpp_funcs = {}
        namespaces = {}
        for cursor in cursor_list:
            children = cursor.get_children()
            for child in children:
                if child.kind == CursorKind.ENUM_DECL:
                    self.enum_constant_decls.append(ClangEnumDecl(child))

                elif child.kind == CursorKind.FUNCTION_DECL:
                    if child.spelling in cpp_funcs:
                        cpp_funcs[child.spelling].append(child)
                    else:
                        cpp_funcs[child.spelling] = [child]

                elif child.kind == CursorKind.VAR_DECL:
                    self.var_decls.append(ClangASTNode(child))

                elif (child.kind == CursorKind.CLASS_DECL or
                      child.kind == CursorKind.STRUCT_DECL or
                      child.kind == CursorKind.UNION_DECL):
                    self.record_decls.append(ClangRecordDecl(child))

                elif child.kind == CursorKind.NAMESPACE:
                    if child.spelling in namespaces:
                        namespaces[child.spelling].append(child)
                    else:
                        namespaces[child.spelling] = [child]

        for name, cursor_list in cpp_funcs.items():
            func = ClangRecordMethod(name, cursor_list, self.function_decls)
            if func.params:
                self.function_decls.append()

        for name, cursor_list in namespaces.items():
            self.namespaces.append(ClangNamespace(name, cursor_list))

# This class responsible for initializing and visiting
# the AST provided by libclang.
class ClangTUVisitor:
    def __init__(self, lang, header, api_headers, check_all, args, verbose):
        index = Index.create()

        self.is_cpp = True if lang == 'c++' else False
        self.clang_args = ['-x', lang]
        self.translation_unit = index.parse(header, args + self.clang_args,
                                            options=1)

        if verbose:
            for diag in self.translation_unit.diagnostics:
                if diag.severity == 2:
                    msg = '\033[93mWARNING : '
                elif diag.severity == 3:
                    msg = '\033[91mERROR : '
                elif diag.severity == 4:
                    msg = '\033[91mFATAL : '
                msg += '{} at {} line {}, column {}\033[00m'
                print (msg.format(diag.spelling, diag.location.file,
                                diag.location.line, diag.location.column))

        self.api_headers = api_headers
        self.check_all = check_all
        self.enum_constant_decls = []
        self.function_decls = []
        self.var_decls = []
        self.macro_defs = []
        self.record_decls = []
        self.namespaces = []

    def visit(self):
        children = self.translation_unit.cursor.get_children()

        cpp_funcs = {}
        namespaces = {}
        for cursor in children:
            if (cursor.location.file != None and
                cursor.location.file.name in self.api_headers):

                if cursor.kind == CursorKind.ENUM_DECL:
                    self.enum_constant_decls.append(ClangEnumDecl(cursor))

                elif cursor.kind == CursorKind.FUNCTION_DECL:
                    if self.is_cpp:
                        if cursor.spelling in cpp_funcs:
                            cpp_funcs[cursor.spelling].append(cursor)
                        else:
                            cpp_funcs[cursor.spelling] = [cursor]
                    else:
                        self.function_decls.append(ClangFunctionDecl(cursor))

                elif cursor.kind == CursorKind.VAR_DECL:
                    self.var_decls.append(ClangASTNode(cursor))

                elif cursor.kind == CursorKind.MACRO_DEFINITION:
                    self.macro_defs.append(ClangMacroDef(cursor))

                elif (cursor.kind == CursorKind.CLASS_DECL or
                      cursor.kind == CursorKind.STRUCT_DECL or
                      cursor.kind == CursorKind.UNION_DECL):
                    self.record_decls.append(ClangRecordDecl(cursor))

                elif cursor.kind == CursorKind.NAMESPACE:
                    if cursor.spelling in namespaces:
                        namespaces[cursor.spelling].append(cursor)
                    else:
                        namespaces[cursor.spelling] = [cursor]

        for name, cursor_list in cpp_funcs.items():
            func = ClangRecordMethod(name, cursor_list, self.function_decls)
            if func.params:
                self.function_decls.append(func)

        for name, cursor_list in namespaces.items():
            self.namespaces.append(ClangNamespace(name, cursor_list))

        # Resolve other macros in macro definition
        for first in self.macro_defs:
            for second in self.macro_defs:
                for i, token in enumerate(second.tokens):
                    if i and first.name == token:
                        second.tokens = (second.tokens[:i] +
                                          first.tokens[1:] +
                                          second.tokens[i+1:])
                        second.token_kinds = (second.token_kinds[:i] +
                                               first.token_kinds[1:] +
                                               second.token_kinds[i+1:])

    def ok(self, msg):
        return '\033[92m{}\033[00m'.format(msg)

    def warn(self, msg):
        return '\033[91m{}\033[00m'.format(msg)

    def check(self, namespace):
        if namespace == self:
            self.check_macros()
        self.check_variables(namespace)

        for record in namespace.record_decls:
            record_is_ok, record_msg = self.check_record(record)
            if not record_is_ok:
                print(record_msg)
                print(str(record._cursor.location) + '\n')
            elif self.check_all:
                print(self.ok('Supported record: ' + record.name) + '\n')

        for func in namespace.function_decls:
            if self.is_cpp:
                param_msg = []
                param_is_ok = True
                for _, param_lists in func.params.items():
                    for p_list in param_lists:
                        is_ok, msg = self.check_parameters(p_list)
                        if not is_ok:
                            param_is_ok = False
                            p_types = ', '.join([p.type.name for p in p_list])
                            warn = self.warn(
                                'Unsupported overload: {}({})'.format(func.name,
                                                                      p_types))
                            param_msg.append(warn)
                            param_msg.append(msg)
                param_msg = '\n'.join(param_msg)
            else:
                param_is_ok, param_msg = self.check_parameters(func.params)

            ret_is_ok, ret_msg = self.check_return_type(func.return_type)
            if not (param_is_ok and ret_is_ok):
                print(self.warn('Unsupported function: ' + func.name))
                if param_msg:
                    print(param_msg)
                if ret_msg:
                    print(ret_msg)
                print(str(func._cursor.location) + '\n')
            elif self.check_all:
                print(self.ok('Supported function: ' + func.name) + '\n')

        for ns in namespace.namespaces:
            self.check(ns)

    def check_macros(self):
        for macro in self.macro_defs:
            if not macro.is_valid():
                print(self.warn('Unsupported macro: ' + macro.name))
                print(str(macro._cursor.location) + '\n')
            elif self.check_all:
                print(self.ok('Supported macro: ' + macro.name) + '\n')

    def check_variables(self, namespace):
        for var in namespace.var_decls:
            is_supported = False
            msg = ''
            if (var.type.is_char() or var.type.is_number() or
                var.type.is_enum() or var.type.is_bool()):
                is_supported = True
            elif var.type.is_pointer():
                pointee = var.type.get_pointee_type()
                if pointee.is_char() or pointee.is_number():
                    is_supported = True
            elif var.type.is_record():
                var_record = var.type.get_as_record_decl()
                is_supported, msg = self.check_record(var_record)

            if not is_supported:
                print(self.warn(
                    'Unsupported variable: {} {}'.format(var.type.name,
                                                         var.name)))
                if msg:
                    print(msg)
                print(str(var._cursor.location) + '\n')
            elif self.check_all:
                print(self.ok(
                    'Supported variable: {} {}'.format(var.type.name,
                                                       var.name)) + '\n')

    def check_record(self, record):
        record_msg = ''
        # Check fields
        field_msg = []
        field_is_ok = True
        for field in record.field_decls:
            is_supported = False
            msg = ''
            if (field.type.is_char() or field.type.is_number() or
                field.type.is_enum() or field.type.is_bool()):
                is_supported = True
            elif field.type.is_pointer():
                pointee = field.type.get_pointee_type()
                if pointee.is_char() or pointee.is_number():
                    is_supported = True
            elif field.type.is_record():
                field_record = field.type.get_as_record_decl()
                is_supported, msg = self.check_record(field_record)

            if not is_supported:
                field_is_ok = False
                warn = self.warn(
                    'Unsupported field: {} {}'.format(field.type.name,
                                                      field.name))
                field_msg.append(warn)
                if msg:
                    field_msg.append(msg)

        # Check constructor
        constructor_msg = []
        constructor_is_ok = True
        for _, param_lists in record.constructor.params.items():
            for param_list in param_lists:
                param_is_ok, param_msg = self.check_parameters(param_list)
                if not param_is_ok:
                    constructor_is_ok = False
                    p_types = ', '.join([p.type.name for p in param_list])
                    warn = self.warn(
                        'Unsupported constructor: {}({})'.format(record.name,
                                                                 p_types))
                    constructor_msg.append(warn)
                    constructor_msg.append(param_msg)

        # Check methods
        method_msg = []
        method_is_ok = True
        for method in record.methods:
            for _, param_lists in method.params.items():
                for param_list in param_lists:
                    param_is_ok, param_msg = self.check_parameters(param_list)
                    if not param_is_ok:
                        method_is_ok = False
                        p_types = ', '.join([p.type.name for p in param_list])
                        warn = self.warn(
                            'Unsupported overload: {}({})'.format(method.name,
                                                                  p_types))
                        method_msg.append(warn)
                        method_msg.append(param_msg)

            ret_is_ok, ret_msg = self.check_return_type(method.return_type)

            if not ret_is_ok:
                method_is_ok = False
                method_msg.append(ret_msg)

            if not method_is_ok:
                warn = self.warn('Unsupported method: ' + method.name)
                method_msg.insert(0, warn)

        record_msg = ('\n'.join(field_msg) + '\n'.join(constructor_msg) +
                      '\n'.join(method_msg))
        record_is_ok =  field_is_ok and constructor_is_ok and method_is_ok

        if not record_is_ok:
            record_msg = (self.warn('Unsupported record: ' + record.name) + '\n'
                          + record_msg)

        return record_is_ok, record_msg

    def check_parameters(self, param_list):
        param_msg = []
        param_is_ok = True
        for param in param_list:
            is_supported = False
            msg = ''
            if (param.type.is_char() or param.type.is_number() or
                param.type.is_enum() or param.type.is_bool()):
                is_supported = True
            elif param.type.is_pointer():
                pointee = param.type.get_pointee_type()
                if pointee.is_char() or pointee.is_number():
                    is_supported = True
                elif pointee.is_record():
                    record = pointee.get_as_record_decl()
                    is_supported, msg = self.check_record(record)
                elif pointee.is_function():
                    is_supported = self.check_func_ptr(param.get_as_function())
            elif param.type.is_record():
                record = param.get_as_record_decl()
                is_supported, msg = self.check_record(record)
            elif param.type.is_function():
                is_supported = self.check_func_ptr(param.get_as_function())

            if not is_supported:
                param_is_ok = False
                warn = self.warn(
                    'Unsupported parameter: {} {}'.format(param.type.name,
                                                          param.name))
                param_msg.append(warn)
                if msg:
                    param_msg.append(msg)

        return param_is_ok, '\n'.join(param_msg)

    def check_return_type(self, return_type):
        msg = ''
        return_type_is_ok = False
        if (return_type.is_void() or return_type.is_char() or
            return_type.is_number() or return_type.is_enum() or
            return_type.is_bool()):
            return_type_is_ok = True
        elif return_type.is_pointer():
            pointee = return_type.get_pointee_type()
            if pointee.is_char() or pointee.is_number():
                return_type_is_ok = True
        elif return_type.is_record():
            record = return_type.get_as_record_decl()
            return_type_is_ok, msg = self.check_record(record)

        if not return_type_is_ok:
            warn = self.warn('Unsupported return type: ' + return_type.name)
            if msg:
                msg = warn + '\n' + msg
            else:
                msg = warn

        return return_type_is_ok, msg

    def check_func_ptr(self, function):
        param_is_ok = True
        for param in function.params:
            is_supported = False
            if (param.type.is_char() or param.type.is_number() or
                param.type.is_enum() or param.type.is_bool()):
                is_supported = True
            elif param.type.is_pointer():
                pointee = param.type.get_pointee_type()
                if pointee.is_char() or pointee.is_number():
                    is_supported = True
            elif param.type.is_record():
                record = param.get_as_record_decl()
                is_supported, _ = self.check_record(record)
            if not is_supported:
                param_is_ok = False
                break

        ret_type = function.return_type
        ret_is_ok = False
        if (ret_type.is_void() or ret_type.is_char() or ret_type.is_number() or
            ret_type.is_enum() or ret_type.is_bool()):
            ret_is_ok = True
        elif ret_type.is_pointer():
            pointee = ret_type.get_pointee_type()
            if pointee.is_char() or pointee.is_number():
                ret_is_ok = True
            elif pointee.is_record():
                record = pointee.get_as_record_decl()
                ret_is_ok, _ = self.check_record(record)
        elif ret_type.is_record():
            record = ret_type.get_as_record_decl()
            ret_is_ok, _ = self.check_record(record)

        return (param_is_ok and ret_is_ok)
