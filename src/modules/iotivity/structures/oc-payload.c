/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "oc-payload.h"
#include "../common.h"

void js_OCResourcePayload(const iotjs_jval_t* req, OCResourcePayload* payload) {
  iotjs_jval_t js_types = iotjs_jval_create_array(0);
  iotjs_jval_t js_interfaces = iotjs_jval_create_array(0);

  js_OCStringLL(&js_types, payload->types);
  js_OCStringLL(&js_interfaces, payload->interfaces);

  iotjs_jval_set_property_string_raw(req, "uri", payload->uri);

  iotjs_jval_set_property_jval(req, "types", &js_types);
  iotjs_jval_set_property_jval(req, "interfaces", &js_interfaces);

  iotjs_jval_set_property_number(req, "bitmap", payload->bitmap);
  iotjs_jval_set_property_boolean(req, "secure", payload->secure);
  iotjs_jval_set_property_number(req, "port", payload->port);

  iotjs_jval_destroy(&js_types);
  iotjs_jval_destroy(&js_interfaces);
}

void js_OCDiscoveryPayload(const iotjs_jval_t* req,
                           OCDiscoveryPayload* payload) {
  iotjs_jval_set_property_number(req, "type", payload->base.type);
  iotjs_jval_set_property_string_raw(req, "sid", payload->sid);
  iotjs_jval_t js_resources = iotjs_jval_create_array(0);

  if (payload->resources) {
    uint32_t index;
    OCResourcePayload* res = payload->resources;
    for (index = 0; res != NULL; index++, res = res->next) {
      iotjs_jval_t js_res_payload = iotjs_jval_create_object();

      js_OCResourcePayload(&js_res_payload, res);
      iotjs_jval_set_property_by_index(&js_resources, index, &js_res_payload);

      iotjs_jval_destroy(&js_res_payload);
    }
    iotjs_jval_set_property_jval(req, "resources", &js_resources);
  }
  iotjs_jval_destroy(&js_resources);
}

void create_payload_array_recursively(OCRepPayloadValueArray* array,
                                      size_t* p_data_index, int dimension_index,
                                      const iotjs_jval_t* jarray) {
  uint32_t index;
  iotjs_jval_t js_value;

  for (index = 0; index < (uint32_t)(array->dimensions[dimension_index]);
       index++) {
    iotjs_jval_t js_array = iotjs_jval_create_array(0);
    iotjs_string_t str_value = iotjs_string_create();

    // Fill with arrays.
    if (dimension_index < MAX_REP_ARRAY_DEPTH - 1 &&
        array->dimensions[dimension_index + 1] > 0) {
      create_payload_array_recursively(array, p_data_index, dimension_index + 1,
                                       &js_array);
      iotjs_jval_set_property_by_index(jarray, index, &js_array);
    }
    // Fill with data.
    else {
      switch (array->type) {
        case OCREP_PROP_INT:
          js_value = iotjs_jval_create_number(array->iArray[(*p_data_index)++]);
          iotjs_jval_set_property_by_index(jarray, index, &js_value);
          iotjs_jval_destroy(&js_value);
          break;

        case OCREP_PROP_DOUBLE:
          js_value = iotjs_jval_create_number(array->dArray[(*p_data_index)++]);
          iotjs_jval_set_property_by_index(jarray, index, &js_value);
          iotjs_jval_destroy(&js_value);
          break;

        case OCREP_PROP_BOOL:
          iotjs_jval_set_property_by_index(jarray, index,
                                           iotjs_jval_get_boolean(
                                               array->bArray[(
                                                   *p_data_index)++]));
          break;

        case OCREP_PROP_STRING:
          iotjs_string_append(&str_value, array->strArray[*p_data_index],
                              strlen(array->strArray[*p_data_index]));
          (*p_data_index)++;
          js_value = iotjs_jval_create_string(&str_value);
          iotjs_jval_set_property_by_index(jarray, index, &js_value);
          iotjs_jval_destroy(&js_value);
          break;

        case OCREP_PROP_OBJECT:
          js_value = iotjs_jval_create_object();
          js_OCRepPayload(&js_value, array->objArray[*p_data_index]);
          iotjs_jval_set_property_by_index(jarray, index, &js_value);
          (*p_data_index)++;
          iotjs_jval_destroy(&js_value);
          break;

        default:
          break;
      }
    }
    iotjs_jval_destroy(&js_array);
    iotjs_string_destroy(&str_value);
  }
}

void js_OCRepPayloadValueArray(const iotjs_jval_t* jarray,
                               OCRepPayloadValueArray* array) {
  size_t data_index = 0;
  create_payload_array_recursively(array, &data_index, 0, jarray);
}

void js_OCRepPayload(const iotjs_jval_t* req, OCRepPayload* payload) {
  iotjs_jval_t js_types = iotjs_jval_create_array(0);
  iotjs_jval_t js_interfaces = iotjs_jval_create_array(0);

  iotjs_jval_set_property_number(req, "type", payload->base.type);

  if (payload->types) {
    js_OCStringLL(&js_types, payload->types);
    iotjs_jval_set_property_jval(req, "types", &js_types);
  }

  if (payload->interfaces) {
    js_OCStringLL(&js_interfaces, payload->interfaces);
    iotjs_jval_set_property_jval(req, "interfaces", &js_interfaces);
  }

  if (payload->values) {
    uint32_t index;
    OCRepPayloadValue* value;
    iotjs_jval_t values = iotjs_jval_create_object();

    for (index = 0, value = payload->values; value;
         value = value->next, index++) {
      iotjs_jval_t js_value;
      iotjs_jval_t js_array = iotjs_jval_create_array(0);
      iotjs_string_t str_value = iotjs_string_create();
      char* name = value->name;

      switch (value->type) {
        case OCREP_PROP_INT:
          js_value = iotjs_jval_create_number(value->i);
          iotjs_jval_set_property_jval(&values, name, &js_value);
          iotjs_jval_destroy(&js_value);
          break;

        case OCREP_PROP_DOUBLE:
          js_value = iotjs_jval_create_number(value->d);
          iotjs_jval_set_property_jval(&values, name, &js_value);
          iotjs_jval_destroy(&js_value);
          break;

        case OCREP_PROP_BOOL:
          iotjs_jval_set_property_boolean(&values, name, (value->b));
          break;

        case OCREP_PROP_STRING:
          iotjs_string_append(&str_value, value->str, strlen(value->str));
          js_value = iotjs_jval_create_string(&str_value);
          iotjs_jval_set_property_jval(&values, name, &js_value);
          iotjs_jval_destroy(&js_value);
          break;

        case OCREP_PROP_OBJECT:
          js_value = iotjs_jval_create_object();
          js_OCRepPayload(&js_value, value->obj);
          iotjs_jval_set_property_jval(&values, name, &js_value);
          iotjs_jval_destroy(&js_value);
          break;

        case OCREP_PROP_ARRAY:
          js_OCRepPayloadValueArray(&js_array, &(value->arr));
          iotjs_jval_set_property_jval(&values, name, &js_array);
          break;

        default:
          break;
      }
      iotjs_string_destroy(&str_value);
      iotjs_jval_destroy(&js_array);
    }

    iotjs_jval_set_property_jval(req, "values", &values);
    iotjs_jval_destroy(&values);
  }

  if (payload->uri) {
    iotjs_jval_set_property_string_raw(req, "uri", payload->uri);
  }

  if (payload->next) {
    iotjs_jval_t next_req = iotjs_jval_create_object();
    js_OCRepPayload(&next_req, payload->next);
    iotjs_jval_set_property_jval(req, "next", &next_req);
    iotjs_jval_destroy(&next_req);
  }

  iotjs_jval_destroy(&js_types);
  iotjs_jval_destroy(&js_interfaces);
}

void js_OCPresencePayload(const iotjs_jval_t* req, OCPresencePayload* payload) {
  iotjs_jval_set_property_number(req, "type", payload->base.type);
  iotjs_jval_set_property_number(req, "sequenceNumber",
                                 payload->sequenceNumber);
  iotjs_jval_set_property_number(req, "maxAge", payload->maxAge);
  iotjs_jval_set_property_number(req, "trigger", payload->trigger);
  iotjs_jval_set_property_string_raw(req, "resourceType",
                                     payload->resourceType);
}

void js_array_from_bytes(iotjs_jval_t* jval, unsigned char* bytes,
                         uint32_t length) {
  uint32_t index;
  for (index = 0; index < length; index++) {
    iotjs_jval_t array_prop = iotjs_jval_create_number(bytes[index]);
    iotjs_jval_set_property_by_index(jval, index, &array_prop);
  }
}

void js_OCSecurityPayload(const iotjs_jval_t* req, OCSecurityPayload* payload) {
  iotjs_jval_set_property_number(req, "type", payload->base.type);

  if (payload->securityData) {
    iotjs_jval_t jval = iotjs_jval_create_array(payload->payloadSize);
    js_array_from_bytes(&jval, (unsigned char*)(payload->securityData),
                        (uint32_t)(payload->payloadSize));
    iotjs_jval_set_property_jval(req, "securityData", &jval);
  }
}

void js_OCPayload(const iotjs_jval_t* req, OCPayload* payload) {
  switch (payload->type) {
    case PAYLOAD_TYPE_DISCOVERY:
      js_OCDiscoveryPayload(req, (OCDiscoveryPayload*)payload);
      break;

    case PAYLOAD_TYPE_REPRESENTATION:
      js_OCRepPayload(req, (OCRepPayload*)payload);
      break;

    case PAYLOAD_TYPE_PRESENCE:
      js_OCPresencePayload(req, (OCPresencePayload*)payload);
      break;

    case PAYLOAD_TYPE_SECURITY:
      js_OCSecurityPayload(req, (OCSecurityPayload*)payload);
      break;

    case PAYLOAD_TYPE_INVALID:
    default:
      break;
  }
  return;
}

bool set_payload_values_array(OCRepPayload* payload, iotjs_jval_t* js_value,
                              char* name) {
  size_t dimensions[MAX_REP_ARRAY_DEPTH] = { 0 };
  bool type_established = false;
  OCRepPayloadPropType array_type;
  iotjs_jval_t* jarray = js_value;
  if (!validate_rep_payload_array(jarray, &type_established, &array_type,
                                  dimensions, 0)) {
    return false;
  }

  if (dimensions[0] > 0) {
    void* flat_array;

    if (!flatten_array(jarray, &flat_array, dimensions, array_type)) {
      free(flat_array);
      return false;
    }

    switch (array_type) {
      case OCREP_PROP_INT:
        if (!OCRepPayloadSetIntArray(payload, name, (const int64_t*)flat_array,
                                     dimensions)) {
          DLOG("RepPayload: failed to set integer array property");
          free((char*)name);
          free((int64_t*)flat_array);
          return false;
        }
        free((int64_t*)flat_array);
        break;

      case OCREP_PROP_DOUBLE:
        if (!OCRepPayloadSetDoubleArray(payload, name,
                                        (const double*)flat_array,
                                        dimensions)) {
          DLOG("RepPayload: failed to set double array property");
          free((char*)name);
          free((double*)flat_array);
          return false;
        }
        free((double*)flat_array);
        break;

      case OCREP_PROP_STRING:
        if (!OCRepPayloadSetStringArray(payload, name, (const char**)flat_array,
                                        dimensions)) {
          DLOG("RepPayload: failed to set string array property");
          free((char*)name);
          free((char**)flat_array);
          return false;
        }
        free((char**)flat_array);
        break;

      case OCREP_PROP_BOOL:
        if (!OCRepPayloadSetBoolArray(payload, name, (const bool*)flat_array,
                                      dimensions)) {
          DLOG("RepPayload: failed to set boolean array property");
          free((char*)name);
          free((bool*)flat_array);
          return false;
        }
        free((bool*)flat_array);
        break;

      case OCREP_PROP_OBJECT:
        if (!OCRepPayloadSetPropObjectArray(payload, name,
                                            (const OCRepPayload**)flat_array,
                                            dimensions)) {
          DLOG("RepPayload: failed to set object array property");
          free((OCRepPayload**)flat_array);
          free((char*)name);
          return false;
        }
        free((OCRepPayload**)flat_array);
        break;

      default:
        break;
    }
  }
  return true;
}

bool set_payload_values(OCRepPayload* payload, iotjs_jval_t* js_values) {
  iotjs_jval_t js_property_names = iotjs_jval_get_property(js_values, "names");
  int length = get_jarray_size(&js_property_names);
  int index;

  iotjs_jval_t js_name;
  iotjs_jval_t js_value;
  for (index = 0; index < length; ++index) {
    js_name =
        iotjs_jval_get_property_by_index(&js_property_names, (uint32_t)index);
    char* name = get_string_from_jval(js_name);
    js_value = iotjs_jval_get_property(js_values, name);

    if (!iotjs_exist_on_js_side(&js_value)) {
      if (!OCRepPayloadSetNull(payload, name)) {
        DLOG("RepPayload: failed to set null property");
        free((char*)name);
        return false;
      }
    } else if (iotjs_jval_is_number(&js_value)) {
      double value = iotjs_jval_as_number(&js_value);
      if (!OCRepPayloadSetPropDouble(payload, name, value)) {
        DLOG("RepPayload: failed to set floating point property");
        free((char*)name);
        return false;
      }
    } else if (iotjs_jval_is_boolean(&js_value)) {
      bool value = iotjs_jval_as_boolean(&js_value);
      if (!OCRepPayloadSetPropBool(payload, name, value)) {
        DLOG("RepPayload: failed to set boolean property");
        free((char*)name);
        return false;
      }
    } else if (iotjs_jval_is_string(&js_value)) {
      iotjs_string_t str_value = iotjs_jval_as_string(&js_value);
      const char* value = iotjs_string_data(&str_value);
      if (!OCRepPayloadSetPropString(payload, name, value)) {
        DLOG("RepPayload: failed to set string property");
        free((char*)name);
        iotjs_string_destroy(&str_value);
        return false;
      }
      iotjs_string_destroy(&str_value);
    } else if (iotjs_jval_is_array(&js_value)) {
      if (!set_payload_values_array(payload, &js_value, name)) {
        return false;
      }
    } else if (iotjs_jval_is_object(&js_value)) {
      OCRepPayload* child_payload = 0;
      if (c_OCRepPayload(&child_payload, &js_value)) {
        if (!OCRepPayloadSetPropObjectAsOwner(payload, name, child_payload)) {
          DLOG("RepPayload: failed to set object property");
          free((char*)name);
          return false;
        }
      } else {
        free((char*)name);
        return false;
      }
    }
    free((char*)name);
    iotjs_jval_destroy(&js_name);
    iotjs_jval_destroy(&js_value);
  }
  iotjs_jval_destroy(&js_property_names);
  return true;
}

bool set_payload_uri(OCRepPayload* payload, iotjs_jval_t* js_uri) {
  iotjs_string_t str_uri = iotjs_jval_as_string(js_uri);
  char* uri = get_string_from_iotjs(&str_uri);
  if (!OCRepPayloadSetUri(payload, uri)) {
    iotjs_string_destroy(&str_uri);
    free(uri);
    return false;
  }
  free(uri);
  iotjs_string_destroy(&str_uri);
  return true;
}

bool c_OCRepPayload(OCRepPayload** p_payload, const iotjs_jval_t* js_payload) {
  OCRepPayload* payload = OCRepPayloadCreate();

  iotjs_jval_t js_uri = iotjs_jval_get_property(js_payload, "uri");
  iotjs_jval_t js_types = iotjs_jval_get_property(js_payload, "types");
  iotjs_jval_t js_interfaces =
      iotjs_jval_get_property(js_payload, "interfaces");
  iotjs_jval_t js_values = iotjs_jval_get_property(js_payload, "values");
  iotjs_jval_t js_next = iotjs_jval_get_property(js_payload, "next");

  // reppayload.uri
  if (iotjs_exist_on_js_side(&js_uri)) {
    if (!set_payload_uri(payload, &js_uri)) {
      goto fail;
    }
  }

  // reppayload.types
  if (iotjs_exist_on_js_side(&js_types)) {
    SET_OCSTRING(payload, types);
  }

  // reppayload.interfaces
  if (iotjs_exist_on_js_side(&js_interfaces)) {
    SET_OCSTRING(payload, interfaces);
  }

  // reppayload.values
  if (iotjs_exist_on_js_side(&js_values)) {
    if (!set_payload_values(payload, &js_values)) {
      goto fail;
    }
  }

  if (iotjs_exist_on_js_side(&js_next)) {
    OCRepPayload* next_payload = 0;
    if (!c_OCRepPayload(&next_payload, &js_next)) {
      goto fail;
    }
    OCRepPayloadAppend(payload, next_payload);
  }

  (*p_payload) = payload;

  iotjs_jval_destroy(&js_uri);
  iotjs_jval_destroy(&js_values);
  iotjs_jval_destroy(&js_next);
  iotjs_jval_destroy(&js_types);
  iotjs_jval_destroy(&js_interfaces);
  return true;

fail:
  OCRepPayloadDestroy(payload);
  iotjs_jval_destroy(&js_types);
  iotjs_jval_destroy(&js_interfaces);
  iotjs_jval_destroy(&js_uri);
  iotjs_jval_destroy(&js_values);
  return false;
}

bool fill_array(void* flat_array, int* index, iotjs_jval_t* array,
                OCRepPayloadPropType array_type) {
  uint32_t local_index, length = (uint32_t)get_jarray_size(array);

  for (local_index = 0; local_index < length; local_index++) {
    iotjs_jval_t member = iotjs_jval_get_property_by_index(array, local_index);

    if (iotjs_jval_is_array(&member)) {
      if (!fill_array(flat_array, index, &member, array_type)) {
        iotjs_jval_destroy(&member);
        return false;
      }
    } else {
      switch (array_type) {
        case OCREP_PROP_INT:
          ((uint64_t*)flat_array)[(*index)++] =
              (uint64_t)iotjs_jval_as_number(&member);
          break;

        case OCREP_PROP_DOUBLE:
          ((double*)flat_array)[(*index)++] = iotjs_jval_as_number(&member);
          break;

        case OCREP_PROP_BOOL:
          ((bool*)flat_array)[(*index)++] = iotjs_jval_as_boolean(&member);
          break;

        case OCREP_PROP_STRING: {
          char* the_string = get_string_from_jval(member);
          if (the_string) {
            ((char**)flat_array)[(*index)++] = the_string;
          } else {
            int free_index;
            for (free_index = 0; free_index < (*index); free_index++) {
              free(((char**)flat_array)[free_index]);
            }
            iotjs_jval_destroy(&member);
            return false;
          }
          break;
        }

        case OCREP_PROP_OBJECT: {
          OCRepPayload* the_object;
          if (c_OCRepPayload(&the_object, &member)) {
            ((OCRepPayload**)flat_array)[(*index)++] = the_object;
          } else {
            int free_index;
            for (free_index = 0; free_index < (*index); free_index++) {
              OCRepPayloadDestroy(((OCRepPayload**)flat_array)[free_index]);
            }
            iotjs_jval_destroy(&member);
            return false;
          }
          break;
        }

        case OCREP_PROP_NULL:
        case OCREP_PROP_ARRAY:
        default:
          break;
      }
    }
    iotjs_jval_destroy(&member);
  }

  return true;
}

bool flatten_array(iotjs_jval_t* array, void** flat_array,
                   size_t dimensions[MAX_REP_ARRAY_DEPTH],
                   OCRepPayloadPropType array_type) {
  size_t total_elements = 1;
  void* return_value = 0;
  size_t dim_index;

  for (dim_index = 0;
       dim_index < MAX_REP_ARRAY_DEPTH && dimensions[dim_index] > 0;
       dim_index++) {
    total_elements *= dimensions[dim_index];
  }

  // The validation ensures that the
  // array type is always valid
  size_t needed_amount =
      (array_type == OCREP_PROP_INT
           ? sizeof(uint64_t)
           : array_type == OCREP_PROP_DOUBLE
                 ? sizeof(double)
                 : array_type == OCREP_PROP_BOOL
                       ? sizeof(bool)
                       : array_type == OCREP_PROP_STRING
                             ? sizeof(char*)
                             : array_type == OCREP_PROP_OBJECT
                                   ? sizeof(OCRepPayload*)
                                   : 0) *
      total_elements;

  return_value = malloc(needed_amount);

  if (!return_value) {
    DLOG("Not enough memory for flattening rep payload array");
    return false;
  }

  memset(return_value, 0, needed_amount);

  int index = 0;
  if (!fill_array(return_value, &index, array, array_type)) {
    free(return_value);
    return false;
  }

  *flat_array = return_value;
  return true;
}

bool js_set_prop_payload_type(iotjs_jval_t* value, OCRepPayloadPropType* type) {
  if (iotjs_jval_is_string(value)) {
    *type = OCREP_PROP_STRING;
    return true;
  }
  if (iotjs_jval_is_number(value)) {
    *type = OCREP_PROP_DOUBLE;
    return true;
  }
  if (iotjs_jval_is_boolean(value)) {
    *type = OCREP_PROP_BOOL;
    return true;
  }
  if (iotjs_jval_is_object(value)) {
    *type = OCREP_PROP_OBJECT;
    return true;
  }
  DLOG("Value type not allowed in RepPayload");
  return false;
}

bool validate_rep_payload_array(iotjs_jval_t* jarray, bool* type_established,
                                OCRepPayloadPropType* array_type,
                                size_t dimensions[MAX_REP_ARRAY_DEPTH],
                                int index) {
  if (index >= MAX_REP_ARRAY_DEPTH) {
    DLOG("RepPayload array has too many dimensions");
    return false;
  }

  uint32_t length = (uint32_t)get_jarray_size(jarray);

  if (length > 0) {
    iotjs_jval_t first_value = iotjs_jval_get_property_by_index(jarray, 0);
    if (iotjs_jval_is_array(&first_value)) {
      int child_length = get_jarray_size(&first_value);
      uint32_t a_index;
      for (a_index = 0; a_index < length; a_index++) {
        iotjs_jval_t member = iotjs_jval_get_property_by_index(jarray, a_index);
        if (!iotjs_jval_is_array(&member)) {
          DLOG("RepPayload array is heterogeneous");
          iotjs_jval_destroy(&member);
          return false;
        }

        bool child_established = false;
        OCRepPayloadPropType child_type;
        iotjs_jval_t* child_array = &member;

        if (get_jarray_size(child_array) != child_length) {
          DLOG("RepPayload array contains child arrays of different lengths");
          return false;
        }

        if (!validate_rep_payload_array(child_array, &child_established,
                                        &child_type, dimensions, index + 1)) {
          return false;
        }

        // Reconcile array types.
        if (*type_established) {
          if (!child_established || child_type != *array_type) {
            DLOG("RepPayload array is heterogeneous");
            return false;
          }
        } else {
          if (child_established) {
            *type_established = true;
            *array_type = child_type;
          }
        }
        iotjs_jval_destroy(&member);
      }
    } else {
      OCRepPayloadPropType value_type;

      if (!js_set_prop_payload_type(&first_value, &value_type)) {
        return false;
      }

      if (*type_established) {
        if (value_type != *array_type) {
          DLOG("RepPayload array is heterogeneous");
          return false;
        }
      } else {
        *type_established = true;
        *array_type = value_type;
      }
      size_t a_index;
      for (a_index = 1; a_index < length; a_index++) {
        iotjs_jval_t array_diff =
            iotjs_jval_get_property_by_index(jarray, a_index);
        if (!js_set_prop_payload_type(&array_diff, &value_type)) {
          iotjs_jval_destroy(&array_diff);
          return false;
        }
        if (value_type != *array_type) {
          DLOG("RepPayload array is heterogeneous");
          iotjs_jval_destroy(&array_diff);
          return false;
        }
        iotjs_jval_destroy(&array_diff);
      }
    }
    iotjs_jval_destroy(&first_value);
  }

  dimensions[index] = length;
  return true;
}

void c_OCPayload(OCPayload** payload, const iotjs_jval_t* js_payload) {
  iotjs_jval_t js_type = iotjs_jval_get_property(js_payload, "type");

  if (!iotjs_exist_on_js_side(&js_type)) {
    payload = 0;
    iotjs_jval_destroy(&js_type);
    return;
  }
  OCPayloadType type = iotjs_jval_as_number(&js_type);

  switch (type) {
    case PAYLOAD_TYPE_REPRESENTATION:
      if (c_OCRepPayload((OCRepPayload**)payload, js_payload)) {
        iotjs_jval_destroy(&js_type);
        return;
      }
      DLOG("Creating payload failed");
      payload = 0;
      iotjs_jval_destroy(&js_type);
      return;
    // We do not need other cases, because only OCRepPayload is used to set
    // new properties to devices. Others *Payloads are used to printing
    // existing devices and their properties.
    default:
      printf("Implementation for other types does not exist\n");
      payload = 0;
      iotjs_jval_destroy(&js_type);
      return;
  }
  iotjs_jval_destroy(&js_type);
  return;
}
