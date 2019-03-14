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

import c_source_templates as c
import cpp_source_templates as cpp

class CSourceGenerator(object):
    def __init__(self):
        self.function_names = []
        self.record_names = []
        self.variable_names = []
        self.enums = []
        self.constant_variables = []
        self.number_arrays = []
        self.namespace = []
        self.macros = []
        self.init_func_body = []
        self.global_records = []
        self.global_const_records = []

    @property
    def ns_name(self):
        if self.namespace:
            return '_'.join(self.namespace) + '_'
        return ''

    @property
    def parent_ns_name(self):
        if self.namespace[:-1]:
            return '_'.join(self.namespace[:-1]) + '_'
        return ''

    @property
    def scope_name(self):
        if self.namespace:
            return '::'.join(self.namespace) + '::'
        return ''

    # methods for create/set a C variable
    def js_to_char(self, _type, name, jval):
        return c.JS_TO_CHAR.format(TYPE=_type, NAME=name, JVAL=jval)

    def js_set_char(self, name, jval):
        return c.JS_SET_CHAR.format(NAME=name, JVAL=jval)

    def js_to_number(self, _type, name, jval):
        return c.JS_TO_NUMBER.format(TYPE=_type, NAME=name, JVAL=jval)

    def js_set_number(self, name, jval):
        return c.JS_SET_NUMBER.format(NAME=name, JVAL=jval)

    def js_to_bool(self, _type, name, jval):
        return c.JS_TO_BOOL.format(TYPE=_type, NAME=name, JVAL=jval)

    def js_set_bool(self, name, jval):
        return c.JS_SET_BOOL.format(NAME=name, JVAL=jval)

    def js_to_string(self, _type, name, jval):
        return c.JS_TO_STRING.format(TYPE=_type, NAME=name, JVAL=jval)

    def js_free_string(self, name, jval):
        return c.JS_FREE_STRING.format(NAME=name, JVAL=jval)

    def js_set_char_pointer(self, _type, name, jval):
        return c.JS_SET_CHAR_PTR.format(TYPE=_type, NAME=name, JVAL=jval)

    def js_set_char_array(self, name, jval, size):
        return c.JS_SET_CHAR_ARR.format(NAME=name, JVAL=jval, SIZE=size)

    def js_to_num_pointer(self, _type, name, jval):
        return c.JS_TO_TYPEDARRAY.format(TYPE=_type, NAME=name, JVAL=jval)

    def js_free_buffer(self, name, jval):
        return c.JS_FREE_BUFFER.format(NAME=name, JVAL=jval)

    def js_free_write_buffer(self, name, jval):
        return c.JS_FREE_WRITE_BUFFER.format(NAME=name, JVAL=jval)

    def js_set_num_pointer(self, _type, name, jval):
        return c.JS_SET_TYPEDARRAY.format(TYPE=_type, NAME=name, JVAL=jval)

    def js_to_record(self, _type, name, jval, record):
        return c.JS_TO_RECORD.format(TYPE=_type, NAME=name, JVAL=jval,
                                     RECORD=record)

    def js_set_record(self, _type, name, jval, record):
        return c.JS_SET_RECORD.format(TYPE=_type, NAME=name, JVAL=jval,
                                      RECORD=record)

    def js_set_const_record(self, _type, name, jval, record):
        return c.JS_SET_CONST_RECORD.format(TYPE=_type, NAME=name, JVAL=jval,
                                            RECORD=record)

    def js_to_record_ptr(self, _type, name, jval, record):
        return c.JS_TO_RECORD_PTR.format(TYPE=_type, NAME=name, JVAL=jval,
                                         RECORD=record)

    def js_to_function(self, func, name, jval, _type, params):
        return c.JS_TO_FUNCTION.format(FUNC=func, NAME=name, JVAL=jval,
                                       TYPE=_type, PARAMS=params)

    def js_cb_function(self, func, name, ret_t, params, length, create_val,
                       result, ret):
        return c.JS_CB_FUNCTION.format(FUNC=func, NAME=name,  RET_TYPE=ret_t,
                                       PARAMS=params, LENGTH=length, RET=ret,
                                       CREATE_VAL=create_val, RESULT=result)

    def js_to_unsupported(self, _type, name):
        return c.JS_TO_UNSUPPORTED.format(TYPE=_type, NAME=name)

    def js_to_c(self, c_type, name, jval):
        if c_type.is_char():
            return self.js_to_char(c_type.name, name, jval)
        elif c_type.is_number() or c_type.is_enum():
            return self.js_to_number(c_type.name, name, jval)
        elif c_type.is_bool():
            return self.js_to_bool(c_type.name, name, jval)
        elif c_type.is_pointer():
            pointee = c_type.get_pointee_type()
            if pointee.is_char():
                return self.js_to_string(pointee.name, name, jval)
            elif pointee.is_number():
                return self.js_to_num_pointer(pointee.name, name, jval)
            elif pointee.is_record():
                record = pointee.get_as_record_decl().ns_name
                return self.js_to_record_ptr(pointee.name, name, jval, record)
        elif c_type.is_record():
            record = c_type.get_as_record_decl().ns_name
            return self.js_to_record(c_type.name, name, jval, record)

        return self.js_to_unsupported(c_type.name, name)

    def js_set_c(self, c_type, name, jval):
        if c_type.is_char():
            return self.js_set_char(name, jval)
        elif c_type.is_number() or c_type.is_enum():
            return self.js_set_number(name, jval)
        elif c_type.is_bool():
            return self.js_set_bool(name, jval)
        elif c_type.is_pointer():
            pointee = c_type.get_pointee_type()
            if c_type.is_array():
                size = c_type.get_array_size() - 1
                if pointee.is_char():
                    return self.js_set_char_array(name, jval, size)
                elif pointee.is_number():
                    return self.js_set_num_pointer(pointee.name, name, jval)
            else:
                if pointee.is_char():
                    return self.js_set_char_pointer(pointee.name, name, jval)
                elif pointee.is_number():
                    return self.js_set_num_pointer(pointee.name, name, jval)
        elif c_type.is_record():
            record = c_type.get_as_record_decl().ns_name
            if c_type.has_const_member():
                return self.js_set_const_record(c_type.name, name, jval, record)
            return self.js_set_record(c_type.name, name, jval, record)

        return self.js_to_unsupported(c_type.name, name)


    # methods for create a JS variable
    def void_to_js(self, name):
        return c.JS_CREATE_VAL.format(NAME=name, TYPE='undefined', FROM='')

    def char_to_js(self, name, cval):
        return c.JS_CREATE_CHAR.format(NAME=name, FROM=cval)

    def number_to_js(self, name, cval):
        return c.JS_CREATE_VAL.format(NAME=name, TYPE='number', FROM=cval)

    def bool_to_js(self, name, cval):
        return c.JS_CREATE_VAL.format(NAME=name, TYPE='boolean', FROM=cval)

    def string_to_js(self, name, cval):
        return c.JS_CREATE_STRING.format(NAME=name, FROM=cval)

    def num_pointer_to_js(self, name, cval, _type):
        return c.JS_CREATE_TYPEDARRAY.format(NAME=name, FROM=cval, TYPE=_type,
                                             ARRAY_TYPE=c.TYPEDARRAYS[_type])

    def record_to_js(self, name, cval, _type, record):
        return c.JS_CREATE_OBJECT.format(NAME=name, FROM=cval, TYPE=_type,
                                         RECORD=record)

    def const_record_to_js(self, name, cval, _type, record):
        return c.JS_CREATE_CONST_OBJECT.format(NAME=name, FROM=cval, TYPE=_type,
                                               RECORD=record)

    def unsupported_to_js(self, name, cval):
        return c.JS_CREATE_UNSUPPORTED.format(NAME=name, FROM=cval)

    def c_to_js(self, c_type, cval, name):
        if c_type.is_void():
            return self.void_to_js(name)
        elif c_type.is_char():
            return self.char_to_js(name, cval)
        elif c_type.is_number() or c_type.is_enum():
            return self.number_to_js(name, cval)
        elif c_type.is_bool():
            return self.bool_to_js(name, cval)
        elif c_type.is_pointer():
            pointee = c_type.get_pointee_type()
            if pointee.is_char():
                return self.string_to_js(name, cval)
            elif pointee.is_number():
                return self.num_pointer_to_js(name, cval, pointee.name)
        elif c_type.is_record():
            record = c_type.get_as_record_decl().ns_name
            if c_type.has_const_member():
                return self.const_record_to_js(name, cval, c_type.name, record)
            return self.record_to_js(name, cval, c_type.name, record)

        return self.unsupported_to_js(name, cval)

    def js_record_destructor(self, _type, record):
        return c.JS_RECORD_DESTRUCTOR.format(TYPE=_type, RECORD=record)

    def js_record_member(self, _type, record, name, body):
        return c.JS_RECORD_MEMBER.format(TYPE=_type, RECORD=record, NAME=name,
                                         BODY=body)

    def js_record_getter(self, name, record):
        return c.JS_RECORD_GETTER.format(NAME=name, RECORD=record)

    def js_record_creator(self, _type, record, regist):
        return c.JS_RECORD_CREATOR.format(TYPE=_type, RECORD=record,
                                          REGIST=regist)

    def js_record_constructor_c(self, _type, record, body):
        return c.JS_RECORD_CONSTRUCTOR.format(TYPE=_type, RECORD=record,
                                              BODY=body)

    def js_record_return(self, record):
        return c.JS_RECORD_RETURN.format(RECORD=record)

    def js_get_prop_struct(self, name, _type, get_val, init=''):
        return c.JS_GET_PROP_STRUCT.format(NAME=name, TYPE=_type,
                                           GET_VAL=get_val, INIT=init)

    def js_get_prop_union(self, name, _type, get_val, ret, init=''):
        return c.JS_GET_PROP_UNION.format(NAME=name, TYPE=_type,
                                          GET_VAL=get_val, RET=ret, INIT=init)

    def js_init_members(self, _type, members):
        return c.JS_INIT_MEMBERS.format(TYPE=_type, MEMBERS=members)

    def js_init_members_const(self, _type, members):
        return c.JS_INIT_MEMBERS_CONST.format(TYPE=_type, MEMBERS=members)

    def js_regist_member(self, record, name):
        return c.JS_REGIST_MEMBER.format(RECORD=record, NAME=name)

    def js_regist_record(self, name, record, ref, object):
        return c.JS_REGIST_RECORD.format(NAME=name, RECORD=record, REF=ref,
                                         OBJECT=object)

    def js_regist_const_member(self, name):
        return c.JS_REGIST_CONST_MEMBER.format(NAME=name)

    def js_regist_const_record(self, name, object, value):
        return c.JS_REGIST_CONST_RECORD.format(NAME=name, OBJECT=object,
                                               VALUE=value)

    def js_regist_num_arr_member(self, name, _type, size):
        return c.JS_REGIST_ARR_MEMBER.format(NAME=name, TYPE=_type, SIZE=size,
                                             ARRAY_TYPE=c.TYPEDARRAYS[_type])

    def create_member_getter_setter(self, member, record):
        name = 'native_ptr->' + member.name
        record_name = self.ns_name + record.name

        if member.type.is_record():
            getter = self.js_record_getter(member.name, record_name + '_')
            if member.type.has_const_member():
                return getter
        else:
            get_result = self.c_to_js(member.type, name, 'ret_val')
            getter = self.js_record_member(record.type.name, record_name,
                                           member.name + '_getter', get_result)

        set_result = self.js_check_type(member.type, 'args_p[0]',
                                        member.name + '_setter')
        set_result += self.js_set_c(member.type, name, 'args_p[0]')
        set_result += '  jerry_value_t ret_val = jerry_create_undefined();'
        setter = self.js_record_member(record.type.name, record_name,
                                       member.name + '_setter', set_result)

        return getter + setter

    def create_record_constructor(self, record):
        name = self.ns_name + record.name
        type_name = record.type.name
        is_struct = record.type.is_struct()
        is_union = record.type.is_union()
        has_const_member = record.type.has_const_member()
        constructor = []
        members = []
        free = []

        for member in record.field_decls:
            m_name = member.name
            m_type = member.type
            jval = '{}_value'.format(m_name)
            init = ''
            get_val = self.js_set_c(m_type, m_name, jval)

            if m_type.is_array():
                size = m_type.get_array_size()
                init_array = ['{' + m_name + '[0]']
                for i in range(size-2):
                    init_array.append('{}[{}]'.format(m_name, i+1))
                init_array.append('{}[{}]}}'.format(m_name, size-1))
                init_array = (', ').join(init_array)
                members.append(init_array)
                m_type = m_type.get_array_type()
                init = '[{}]'.format(size)
                if is_union:
                    if has_const_member:
                        get_val += self.js_init_members_const(type_name,
                                                              init_array)
                    else:
                        get_val += self.js_init_members(type_name, init_array)
            else:
                members.append(m_name)
                if member.type.is_record():
                    if not member.type.get_as_record_decl().has_constructor():
                        init = ' = {}'
                if is_union:
                    if has_const_member:
                        get_val += self.js_init_members_const(type_name, m_name)
                    else:
                        get_val += self.js_init_members(type_name, m_name)
                if m_type.is_pointer():
                    if m_type.get_pointee_type().is_char():
                        free.append(self.js_free_string(m_name, jval))
                        if is_union:
                            get_val += self.js_free_string(m_name, jval)
                    elif m_type.get_pointee_type().is_number():
                        free.append(self.js_free_buffer(m_name, jval))
                        if is_union:
                            get_val += self.js_free_buffer(m_name, jval)

            if is_struct:
                constructor.append(self.js_get_prop_struct(m_name, m_type.name,
                                                           get_val, init))
            elif is_union:
                ret = self.js_record_return(name)
                constructor.append(self.js_get_prop_union(m_name, m_type.name,
                                                          get_val, ret, init))
        constructor = ('').join(constructor)
        members = (', ').join(members)
        free = ('').join(free)
        if has_const_member:
            members = self.js_init_members_const(type_name, members)
        else:
            members = self.js_init_members(type_name, members)
        if is_struct:
            body = constructor + members + free
        elif is_union:
            body = constructor

        return self.js_record_constructor_c(type_name, name, body)

    def create_record(self, record):
        name = record.name
        type_name = record.type.name
        self.record_names.append(name)
        result = [self.js_record_destructor(type_name, name)]
        regist = []

        for member in record.field_decls:
            m_name = member.name
            m_type = member.type
            if m_type.is_const():
                cval = 'native_ptr->' + m_name
                jval = m_name + '_js'
                regist.append(self.c_to_js(m_type, cval, jval))
                regist.append(self.js_regist_const_member(m_name))
            elif m_type.get_array_type().is_number():
                arr_name = m_type.get_array_type().name
                size = m_type.get_array_size()
                regist.append(self.js_regist_num_arr_member(m_name, arr_name,
                                                            size))
            else:
                get_set = self.create_member_getter_setter(member, record)
                result.append(get_set)
                if m_type.is_record():
                    r_name = m_type.get_as_record_decl().ns_name
                    ref = 'native_ptr->{}'.format(m_name)
                    regist.append(self.js_regist_record(m_name, r_name, ref,
                                                        'js_obj'))
                    if m_type.has_const_member():
                        r_name = name + '_' + m_name
                        regist.append(self.js_regist_const_record(r_name,
                                                                  'js_obj',
                                                                  m_name))
                    else:
                        regist.append(self.js_regist_member(name, m_name))
                else:
                    regist.append(self.js_regist_member(name, m_name))

        regist = ('').join(regist)
        result.append(self.js_record_creator(type_name, name, regist))
        result.append(self.create_record_constructor(record))

        return '\n'.join(result)

    def create_c_function(self, func, funcname, name):
        params = []
        create_val = []
        res= ''
        ret = ''

        for index, param in enumerate(func.params):
            param_name = 'p_{}'.format(index)
            arg_name = 'arg_{}'.format(index)
            params.append(param.type.name + ' ' + param_name)
            create_val.append(self.c_to_js(param.type, param_name, arg_name))
            create_val.append('  args[{}] = {};'.format(index, arg_name))

        if not func.return_type.is_void():
            res = self.js_to_c(func.return_type, 'ret', 'result')
            ret = 'ret'

        return self.js_cb_function(funcname, name, func.return_type.name,
                                   (', ').join(params), len(func.params),
                                   ('\n').join(create_val), res, ret)

    def js_ext_func(self, name, body):
        return c.JS_EXT_FUNC.format(NAME=name, BODY=body)

    def js_check_arg_count(self, count, func):
        return c.JS_CHECK_ARG_COUNT.format(COUNT=count, FUNC=func)

    def js_check_type(self, c_type, jval, func):
        _type = ''
        template = c.JS_CHECK_TYPE
        if c_type.is_char():
            _type = 'string'
        elif c_type.is_number() or c_type.is_enum():
            _type = 'number'
        elif c_type.is_record():
            _type = 'object'
        elif c_type.is_function():
            template = c.JS_CHECK_POINTER
            _type = 'function'
        elif c_type.is_pointer():
            template = c.JS_CHECK_POINTER
            if c_type.get_pointee_type().is_char():
                _type = 'string'
            elif c_type.get_pointee_type().is_number():
                _type = 'typedarray'
            elif c_type.get_pointee_type().is_function():
                _type = 'function'
            elif c_type.get_pointee_type().is_record():
                _type = 'object'

        if _type:
            return template.format(TYPE=_type, JVAL=jval, FUNC=func)
        return ''

    def get_val_from_param(self, param, funcname, name, jval):
        buff = []
        callback = ''
        if (param.type.is_pointer() and
            param.type.get_pointee_type().is_function()):
            func = param.get_as_function()
            ret_type = func.return_type.name
            params = ', '.join([p.type.name for p in func.params])
            result = self.js_to_function(funcname, name, jval, ret_type, params)
            callback = self.create_c_function(func, funcname, name)
        elif param.type.is_function():
            func = param.get_as_function()
            ret_type = func.return_type.name
            params = ', '.join([p.type.name for p in func.params])
            result = self.js_to_function(funcname, name, jval, ret_type, params)
            callback = self.create_c_function(func, funcname, name)
        else:
            result = self.js_to_c(param.type, name, jval)
            if param.type.is_pointer():
                if param.type.get_pointee_type().is_char():
                    buff.append(self.js_free_string(name, jval))
                if param.type.get_pointee_type().is_number():
                    buff.append(self.js_free_write_buffer(name, jval))

        return result, buff, callback

    def create_ext_function(self, function):
        self.function_names.append(function.name)
        funcname = function.name
        params = function.params
        return_type = function.return_type
        jerry_function = []
        native_params = []
        buffers_to_free = []
        callbacks = []

        jerry_function.append(self.js_check_arg_count(len(params), funcname))

        for index, param in enumerate(params):
            jval = 'args_p[{}]'.format(index)
            native_name = 'arg_{}'.format(index)
            check_type = self.js_check_type(param.type, jval, funcname)

            result = self.get_val_from_param(param, funcname, native_name, jval)
            buffers_to_free += result[1]
            callbacks.append(result[2])
            native_params.append(native_name)
            jerry_function.append(check_type + result[0])

        native_params = (', ').join(native_params)

        if return_type.is_void():
            native_call = '  {} ({});\n'.format(funcname, native_params)
        else:
            native_call = '  {} {} = {} ({});\n'.format(return_type.name,
                                                        'result', funcname,
                                                        native_params)

        jerry_function.append('  // native function call\n' + native_call)
        jerry_function += buffers_to_free

        result = self.c_to_js(return_type, 'result', 'ret_val')

        jerry_function.append(result)

        callbacks = '\n'.join(callbacks)
        jerry_function = '\n'.join(jerry_function)
        ext_func = self.js_ext_func(funcname + '_handler', jerry_function)

        return callbacks + ext_func

    def create_getter_setter(self, var):
        if var.type.is_const():
            self.constant_variables.append(var)
            return ''
        elif var.type.get_array_type().is_number():
            self.number_arrays.append(var)
            return ''

        ns_name = self.ns_name + var.name
        scope_name = self.scope_name + var.name

        if var.type.is_record():
            record = var.type.get_as_record_decl().ns_name
            self.global_records.append((var.name, record))
            getter = self.js_record_getter(ns_name, '')
            if var.type.has_const_member():
                self.global_const_records.append(var.name)
                return getter
        else:
            get_result = self.c_to_js(var.type, scope_name, 'ret_val')
            getter = self.js_ext_func(ns_name + '_getter', get_result)

        self.variable_names.append(var.name)
        set_result = self.js_check_type(var.type, 'args_p[0]',
                                        ns_name + '_setter')
        set_result += self.js_set_c(var.type, scope_name, 'args_p[0]')
        set_result += '  jerry_value_t ret_val = jerry_create_undefined();'

        setter = self.js_ext_func(ns_name + '_setter', set_result)

        return getter + setter

    def init_func(self, name, body):
        return c.INIT_FUNC.format(NAME=name, BODY=body)

    def init_regist_func(self, name, object, func):
        return c.INIT_REGIST_FUNC.format(NAME=name, OBJECT=object, FUNC=func)

    def init_regist_record(self, name, object, record):
        return c.INIT_REGIST_RECORD.format(NAME=name, OBJECT=object,
                                           RECORD=record)

    def init_regist_enum(self, name, object, ref, enum):
        return c.INIT_REGIST_ENUM.format(NAME=name, OBJECT=object, REF=ref,
                                         ENUM=enum)

    def init_regist_value(self, name, object, value):
        return c.INIT_REGIST_VALUE.format(NAME=name, OBJECT=object, VALUE=value)

    def init_regist_const(self, name, object, value):
        return c.INIT_REGIST_CONST.format(NAME=name, OBJECT=object, VALUE=value)

    def init_regist_num_arr(self, name, object, ref, arr, _type, size):
        return c.INIT_REGIST_NUM_ARR.format(NAME=name, OBJECT=object, REF=ref,
                                            ARR=arr, TYPE=_type, SIZE=size,
                                            ARRAY_TYPE=c.TYPEDARRAYS[_type])

    def init_create_object(self, name):
        return c.INIT_CREATE_OBJECT.format(NAME=name)

    def init_regist_object(self, name, ref, object):
        return c.INIT_REGIST_OBJECT.format(NAME=name, REF=ref, OBJECT=object)

    def create_ns_obj(self):
        self.init_func_body.append(self.init_create_object(self.ns_name))

    def regist_ns_obj(self):
        if self.namespace:
            name = self.ns_name
            ref = self.namespace[-1]
            object = '{}object'.format(self.parent_ns_name)
            self.init_func_body.append(self.init_regist_object(name, ref,
                                                               object))

    def create_init_function_body(self):
        object = '{}object'.format(self.ns_name)

        for funcname in self.function_names:
            name = self.ns_name + funcname
            self.init_func_body.append(self.init_regist_func(name, object,
                                                             funcname))

        for record in self.record_names:
            name = self.ns_name + record
            self.init_func_body.append(self.init_regist_record(name, object,
                                                               record))

        for varname in self.variable_names:
            name = self.ns_name + varname
            self.init_func_body.append(self.init_regist_value(name, object,
                                                              varname))

        for glob_record in self.global_records:
            name = self.ns_name + glob_record[0]
            ref = self.scope_name + glob_record[0]
            record = glob_record[1]
            self.init_func_body.append(self.js_regist_record(name, record, ref,
                                                             object))

        for c_record in self.global_const_records:
            name = self.ns_name + c_record
            self.init_func_body.append(self.js_regist_const_record(name, object,
                                                                   c_record))

        for array in self.number_arrays:
            name = self.ns_name + array.name
            ref = self.scope_name + array.name
            typename = array.type.get_array_type().name
            size = array.type.get_array_size()
            self.init_func_body.append(self.init_regist_num_arr(name, object,
                                                                ref, array.name,
                                                                typename, size))

        for var in self.constant_variables:
            name = self.ns_name + var.name
            ref = self.scope_name + var.name
            jval = '{}_js'.format(name)
            self.init_func_body.append(self.c_to_js(var.type, ref, jval))
            self.init_func_body.append(self.init_regist_const(name, object,
                                                              var.name))

        for enum in self.enums:
            name = self.ns_name + enum
            ref = self.scope_name + enum
            self.init_func_body.append(self.init_regist_enum(name, object, ref,
                                                             enum))

        for macro in self.macros:
            name = macro.name
            jval = '{}_js'.format(name)
            if macro.is_char():
                create_val = '  char {N}_value = {N};'.format(N=name)
                value = '{}_value'.format(name)
                self.init_func_body.append(create_val)
                self.init_func_body.append(self.char_to_js(jval, value))
                self.init_func_body.append(self.init_regist_const(name, object,
                                                                  name))
            elif macro.is_string():
                self.init_func_body.append(self.string_to_js(jval, name))
                self.init_func_body.append(self.init_regist_const(name, object,
                                                                  name))
            elif macro.is_number():
                self.init_func_body.append(self.number_to_js(jval, name))
                self.init_func_body.append(self.init_regist_const(name, object,
                                                                  name))

        del self.function_names[:]
        del self.record_names[:]
        del self.variable_names[:]
        del self.global_records[:]
        del self.global_const_records[:]
        del self.number_arrays[:]
        del self.constant_variables[:]
        del self.enums[:]
        del self.macros[:]

    def create_init_function(self, dirname):
        return self.init_func(dirname, ('\n').join(self.init_func_body))



class CppSourceGenerator(CSourceGenerator):
    def __init__(self):
        CSourceGenerator.__init__(self)
        self.class_names = []

    def js_to_string(self, _type, name, jval):
        return cpp.JS_TO_STRING.format(TYPE=_type, NAME=name, JVAL=jval)

    def js_free_string(self, name, jval):
        return cpp.JS_FREE_STRING.format(NAME=name, JVAL=jval)

    def js_set_char_pointer(self, _type, name, jval):
        return cpp.JS_SET_CHAR_PTR.format(TYPE=_type, NAME=name, JVAL=jval)

    def js_to_num_pointer(self, _type, name, jval):
        return cpp.JS_TO_TYPEDARRAY.format(TYPE=_type, NAME=name, JVAL=jval)

    def js_free_write_buffer(self, name, jval):
        return cpp.JS_FREE_WRITE_BUFFER.format(NAME=name, JVAL=jval)

    def js_set_num_pointer(self, _type, name, jval):
        return cpp.JS_SET_TYPEDARRAY.format(TYPE=_type, NAME=name, JVAL=jval)

    def js_return_object(self, name, record, cval):
        return cpp.JS_RETURN_OBJECT.format(NAME=name, RECORD=record, FROM=cval)

    def js_alloc_record(self, record, name):
        return cpp.JS_ALLOC_RECORD.format(RECORD=record, NAME=name)

    def js_value_is(self, c_type, jval):
        if c_type.is_char():
            return cpp.JS_VALUE_IS.format(TYPE='string', JVAL=jval)
        elif c_type.is_number() or c_type.is_enum():
            return cpp.JS_VALUE_IS.format(TYPE='number', JVAL=jval)
        elif c_type.is_bool():
            return cpp.JS_VALUE_IS.format(TYPE='boolean', JVAL=jval)
        elif c_type.is_record():
            record = c_type.get_as_record_decl().ns_name
            return cpp.JS_VALUE_IS.format(TYPE=record, JVAL=jval)
        elif c_type.is_function():
            return cpp.JS_POINTER_IS.format(TYPE='function', JVAL=jval)
        elif c_type.is_pointer():
            if c_type.get_pointee_type().is_char():
                return cpp.JS_POINTER_IS.format(TYPE='string', JVAL=jval)
            elif c_type.get_pointee_type().is_number():
                return cpp.JS_POINTER_IS.format(TYPE='typedarray', JVAL=jval)
            elif c_type.get_pointee_type().is_function():
                return cpp.JS_POINTER_IS.format(TYPE='function', JVAL=jval)
            elif c_type.get_pointee_type().is_record():
                record = c_type.get_pointee_type().get_as_record_decl().ns_name
                return cpp.JS_POINTER_IS.format(TYPE=record, JVAL=jval)
        return ''

    def js_check_record(self, record):
        return cpp.JS_CHECK_RECORD.format(RECORD=record)

    def js_record_destructor(self, _type, record):
        return cpp.JS_RECORD_DESTRUCTOR.format(TYPE=_type, RECORD=record)

    def js_record_constructor_cpp(self, _type, record, case, suff = ''):
        return cpp.JS_RECORD_CONSTRUCTOR.format(TYPE=_type, RECORD=record,
                                                CASE=case, SUFF=suff)

    def js_constr_call(self, condition, get_val, name, params, free):
        return cpp.JS_CONSTR_CALL.format(CONDITION=condition, GET_VAL=get_val,
                                         NAME=name, PARAMS=params, FREE=free)

    def js_constr_case_0(self, name):
        return cpp.JS_CONSTR_CASE_0.format(NAME=name)

    def js_constr_case(self, num, calls, name):
        return cpp.JS_CONSTR_CASE.format(NUM=num, CALLS=calls, NAME=name)

    def js_regist_method(self, record, name):
        return cpp.JS_REGIST_METHOD.format(RECORD=record, NAME=name)

    def js_regist_const_member(self, record, name):
        return cpp.JS_REGIST_CONST_MEMBER.format(RECORD=record, NAME=name)

    def js_record_method(self, record, name, _type, result, case, ret_val,
                         suff):
        return cpp.JS_RECORD_METHOD.format(RECORD=record, NAME=name, TYPE=_type,
                                           RESULT=result, CASE=case,
                                           RET_VAL=ret_val, SUFF=suff)

    def js_method_call(self, condition, get_val, result, name, params, free):
        return cpp.JS_METHOD_CALL.format(CONDITION=condition, GET_VAL=get_val,
                                         RESULT=result, NAME=name,
                                         PARAMS=params, FREE=free)

    def js_method_case_0(self, result, name):
        return cpp.JS_METHOD_CASE_0.format(RESULT=result, NAME=name)

    def js_method_case(self, num, calls, record, name):
        return cpp.JS_METHOD_CASE.format(NUM=num, CALLS=calls, RECORD=record,
                                         NAME=name)

    def js_ext_cpp_func(self, name, result, case, ret_val, suff):
        return cpp.JS_EXT_CPP_FUNC.format(NAME=name, RESULT=result, CASE=case,
                                          RET_VAL=ret_val, SUFF=suff)

    def js_func_call(self, condition, get_val, result, name, params, free):
        return cpp.JS_FUNC_CALL.format(CONDITION=condition, GET_VAL=get_val,
                                       RESULT=result, NAME=name, PARAMS=params,
                                       FREE=free)

    def js_func_case_0(self, result, name):
        return cpp.JS_FUNC_CASE_0.format(RESULT=result, NAME=name)

    def js_func_case(self, num, calls, name):
        return cpp.JS_FUNC_CASE.format(NUM=num, CALLS=calls, NAME=name)

    def create_record(self, record):
        name = record.name
        ns_name = self.ns_name + name
        record_type = record.type.name
        result = [self.js_record_destructor(record_type, ns_name),
                  self.js_check_record(ns_name)]
        regist = []

        for member in record.field_decls:
            m_name = member.name
            if member.type.is_const():
                cval = 'native_ptr->' + m_name
                jval = name + '_' + m_name + '_js'
                regist.append(self.c_to_js(member.type, cval, jval))
                regist.append(self.js_regist_const_member(name, m_name))
            elif member.type.get_array_type().is_number():
                arr_name = member.type.get_array_type().name
                size = member.type.get_array_size()
                regist.append(self.js_regist_num_arr_member(m_name, arr_name,
                                                            size))
            else:
                get_set = self.create_member_getter_setter(member, record)
                result.append(get_set)
                if member.type.is_record():
                    r_name = member.type.get_as_record_decl().ns_name
                    ref = 'native_ptr->{}'.format(m_name)
                    regist.append(self.js_regist_record(m_name, r_name, ref,
                                                        'js_obj'))
                    if member.type.has_const_member():
                        r_name = ns_name + '_' + m_name
                        regist.append(self.js_regist_const_record(r_name,
                                                                  'js_obj',
                                                                  m_name))
                    else:
                        regist.append(self.js_regist_member(ns_name, m_name))
                else:
                    regist.append(self.js_regist_member(ns_name, m_name))

        for method in record.methods:
            result.append(self.create_ext_function(method, name, record_type,
                                                   is_method=True))
            regist.append(self.js_regist_method(ns_name,
                                                method.name + method.suffix))

        regist = ('\n').join(regist)
        result.append(self.js_record_creator(record_type, ns_name, regist))

        if record.has_constructor():
            if not record.constructors:
                self.record_names.append(name)
                cases = self.js_constr_case_0(self.scope_name + name)
                result.append(self.js_record_constructor_cpp(record_type,
                                                             ns_name, cases))
            else:
                for constr in record.constructors:
                    self.record_names.append(name + constr.suffix)
                    cases = self.create_ext_function(constr, name,
                                                     is_constructor=True)
                    result.append(self.js_record_constructor_cpp(record_type,
                                                                 ns_name, cases,
                                                                 constr.suffix))
        else:
            self.record_names.append(name)
            result.append(self.create_record_constructor(record))

        return '\n'.join(result)

    def create_ext_function(self, func, record_name = None, record_type = None,
                            is_constructor = False, is_method = False):
        name = func.name if not is_constructor else record_name
        scope_name = self.scope_name + name
        cases = []
        callbacks = []
        if not is_constructor:
            ret_type = func.return_type
            if ret_type.is_void():
                result = ''
            elif ret_type.is_record():
                result = '*result = '
            else:
                result = 'result = '

        if is_constructor and 0 in func.params:
            cases.append(self.js_constr_case_0(scope_name))
            if func.params:
                del func.params[0]
        elif is_method and 0 in func.params:
            cases.append(self.js_method_case_0(result, name))
            del func.params[0]
        elif 0 in func.params:
            cases.append(self.js_func_case_0(result, scope_name))
            del func.params[0]

        for parm_len, param_lists in func.params.items():
            calls = []
            for param_list in param_lists:
                get_val = ''
                condition = []
                native_params = []
                free_buffers = []
                for index, param in enumerate(param_list):
                    jval = 'args_p[{}]'.format(index)
                    p_name = 'arg_{}'.format(index)
                    condition.append(self.js_value_is(param.type, jval))

                    res = self.get_val_from_param(param, name, p_name, jval)
                    get_val += res[0]
                    free_buffers += res[1]
                    callbacks.append(res[2])
                    native_params.append(p_name)

                native_params = (', ').join(native_params)
                condition = (' && ').join(condition)
                free_buffers = ('\n').join(free_buffers)

                if is_constructor:
                    calls.append(self.js_constr_call(condition, get_val,
                                                     scope_name, native_params,
                                                     free_buffers))
                elif is_method:
                    calls.append(self.js_method_call(condition, get_val, result,
                                                     name, native_params,
                                                     free_buffers))
                else:
                    calls.append(self.js_func_call(condition, get_val, result,
                                                   scope_name, native_params,
                                                   free_buffers))
            calls = ('\n').join(calls)
            if is_constructor:
                cases.append(self.js_constr_case(parm_len, calls, name))
            elif is_method:
                cases.append(self.js_method_case(parm_len, calls, record_name,
                                                 name))
            else:
                cases.append(self.js_func_case(parm_len, calls, name))

        callbacks = '\n'.join(callbacks)
        cases = ''.join(cases)

        if is_constructor:
            return cases

        if ret_type.is_record():
            record = ret_type.get_as_record_decl()
            if record.has_default_constructor():
                result = '{T}* result = new {T}();'.format(T=ret_type.name)
            else:
                result = self.js_alloc_record(ret_type.name, 'result')
            ns_name = self.ns_name + record.name
            ret_val = self.js_return_object('ret_val', ns_name, 'result')
        else:
            if not ret_type.is_void():
                result = '{} result;'.format(ret_type.name)
            ret_val = self.c_to_js(ret_type, 'result', 'ret_val')

        if is_method:
            return self.js_record_method(self.ns_name + record_name, name,
                                         record_type, result, cases, ret_val,
                                         func.suffix)
        else:
            self.function_names.append(name + func.suffix)
            return callbacks + self.js_ext_cpp_func(self.ns_name + name, result,
                                                    cases, ret_val, func.suffix)

    def init_func(self, name, body):
        return cpp.INIT_FUNC.format(NAME=name, BODY=body)
