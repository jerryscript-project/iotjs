#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "node_api.h"

static const char the_text[] =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit.";
static const unsigned int buffer_size = sizeof(the_text) - 1;

static int deleterCallCount = 0;
static void buffer_finalizer(napi_env env, void* data, void* finalize_hint) {
  NAPI_ASSERT_RETURN_VOID(env, data != NULL && strcmp(data, the_text) == 0,
                          "invalid data");
  (void)finalize_hint;
  free(data);
  deleterCallCount++;
}

static void noop_finilizer(napi_env env, void* data, void* finalize_hint) {
  NAPI_ASSERT_RETURN_VOID(env, data != NULL && strcmp(data, the_text) == 0,
                          "invalid data");
  (void)finalize_hint;
  deleterCallCount++;
}

static napi_value new_buffer(napi_env env, napi_callback_info info) {
  napi_value the_buffer;
  char* the_copy;

  NAPI_CALL(env, napi_create_buffer(env, buffer_size, (void**)(&the_copy),
                                    &the_buffer));
  NAPI_ASSERT(env, the_copy, "Failed to copy static text for newBuffer");
  memcpy(the_copy, the_text, buffer_size);

  return the_buffer;
}

static napi_value new_external_buffer(napi_env env, napi_callback_info info) {
  napi_value the_buffer;
  char* the_copy = strdup(the_text);
  NAPI_ASSERT(env, the_copy,
              "Failed to copy static text for newExternalBuffer");
  NAPI_CALL(env, napi_create_external_buffer(env, buffer_size, the_copy,
                                             buffer_finalizer,
                                             NULL, // finalize_hint
                                             &the_buffer));

  return the_buffer;
}

static napi_value get_deleter_call_count(napi_env env,
                                         napi_callback_info info) {
  napi_value callCount;
  NAPI_CALL(env, napi_create_int32(env, deleterCallCount, &callCount));
  return callCount;
}

static napi_value copy_buffer(napi_env env, napi_callback_info info) {
  napi_value the_buffer;
  NAPI_CALL(env, napi_create_buffer_copy(env, buffer_size, the_text, NULL,
                                         &the_buffer));
  return the_buffer;
}

static napi_value buffer_has_instance(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));
  NAPI_ASSERT(env, argc == 1, "Wrong number of arguments");
  napi_value the_buffer = args[0];
  bool hasInstance;
  napi_valuetype theType;
  NAPI_CALL(env, napi_typeof(env, the_buffer, &theType));
  NAPI_ASSERT(env, theType == napi_object,
              "bufferHasInstance: instance is not an object");
  NAPI_CALL(env, napi_is_buffer(env, the_buffer, &hasInstance));
  NAPI_ASSERT(env, hasInstance, "bufferHasInstance: instance is not a buffer");
  napi_value returnValue;
  NAPI_CALL(env, napi_get_boolean(env, hasInstance, &returnValue));
  return returnValue;
}

static napi_value buffer_info(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));
  NAPI_ASSERT(env, argc == 1, "Wrong number of arguments");
  napi_value the_buffer = args[0];
  char* bufferData;
  napi_value returnValue;
  size_t bufferLength;
  NAPI_CALL(env, napi_get_buffer_info(env, the_buffer, (void**)(&bufferData),
                                      &bufferLength));
  NAPI_CALL(env, napi_get_boolean(env, bufferLength == buffer_size &&
                                           !strncmp(bufferData, the_text,
                                                    bufferLength),
                                  &returnValue));
  return returnValue;
}

static napi_value static_buffer(napi_env env, napi_callback_info info) {
  napi_value the_buffer;
  NAPI_CALL(env, napi_create_external_buffer(env, buffer_size, (void*)the_text,
                                             noop_finilizer,
                                             NULL, // finalize_hint
                                             &the_buffer));
  return the_buffer;
}

static napi_value init(napi_env env, napi_value exports) {
  napi_value the_value;

  NAPI_CALL(env,
            napi_create_string_utf8(env, the_text, buffer_size, &the_value));
  NAPI_CALL(env, napi_set_named_property(env, exports, "theText", the_value));

  napi_property_descriptor methods[] = {
    DECLARE_NAPI_PROPERTY("newBuffer", new_buffer),
    DECLARE_NAPI_PROPERTY("newExternalBuffer", new_external_buffer),
    DECLARE_NAPI_PROPERTY("getDeleterCallCount", get_deleter_call_count),
    DECLARE_NAPI_PROPERTY("copyBuffer", copy_buffer),
    DECLARE_NAPI_PROPERTY("bufferHasInstance", buffer_has_instance),
    DECLARE_NAPI_PROPERTY("bufferInfo", buffer_info),
    DECLARE_NAPI_PROPERTY("staticBuffer", static_buffer),
  };

  NAPI_CALL(env, napi_define_properties(env, exports,
                                        sizeof(methods) / sizeof(methods[0]),
                                        methods));

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
