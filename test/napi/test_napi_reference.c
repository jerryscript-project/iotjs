#include "common.h"
#include "node_api.h"

static int test_value = 1;
static int finalize_count = 0;
static napi_ref test_reference = NULL;

static napi_value get_finalize_count(napi_env env, napi_callback_info info) {
  napi_value result;
  NAPI_CALL(env, napi_create_int32(env, finalize_count, &result));
  return result;
}

static void finalize_external(napi_env env, void* data, void* hint) {
  int* actual_value = data;
  NAPI_ASSERT_RETURN_VOID(env, actual_value == &test_value,
                          "The correct pointer was passed to the finalizer");
  finalize_count++;
}

static napi_value create_external(napi_env env, napi_callback_info info) {
  int* data = &test_value;

  napi_value result;
  NAPI_CALL(env, napi_create_external(env, data, NULL, /* finalize_cb */
                                      NULL,            /* finalize_hint */
                                      &result));

  finalize_count = 0;
  return result;
}

static napi_value create_external_with_finalize(napi_env env,
                                                napi_callback_info info) {
  napi_value result;
  NAPI_CALL(env, napi_create_external(env, &test_value, finalize_external,
                                      NULL, /* finalize_hint */
                                      &result));

  finalize_count = 0;
  return result;
}

static napi_value check_external(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value arg;
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &arg, NULL, NULL));

  NAPI_ASSERT(env, argc == 1, "Expected one argument.");

  napi_valuetype argtype;
  NAPI_CALL(env, napi_typeof(env, arg, &argtype));

  NAPI_ASSERT(env, argtype == napi_external, "Expected an external value.");

  void* data;
  NAPI_CALL(env, napi_get_value_external(env, arg, &data));

  NAPI_ASSERT(env, data != NULL && *(int*)data == test_value,
              "An external data value of 1 was expected.");

  return NULL;
}

static napi_value create_reference(napi_env env, napi_callback_info info) {
  NAPI_ASSERT(env, test_reference == NULL,
              "The test allows only one reference at a time.");

  size_t argc = 2;
  napi_value args[2];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));
  NAPI_ASSERT(env, argc == 2, "Expected two arguments.");

  uint32_t initial_refcount;
  NAPI_CALL(env, napi_get_value_uint32(env, args[1], &initial_refcount));

  NAPI_CALL(env, napi_create_reference(env, args[0], initial_refcount,
                                       &test_reference));

  NAPI_ASSERT(env, test_reference != NULL,
              "A reference should have been created.");

  return NULL;
}

static napi_value delete_reference(napi_env env, napi_callback_info info) {
  NAPI_ASSERT(env, test_reference != NULL,
              "A reference must have been created.");

  NAPI_CALL(env, napi_delete_reference(env, test_reference));
  test_reference = NULL;
  return NULL;
}

static napi_value increment_refcount(napi_env env, napi_callback_info info) {
  NAPI_ASSERT(env, test_reference != NULL,
              "A reference must have been created.");

  uint32_t refcount;
  NAPI_CALL(env, napi_reference_ref(env, test_reference, &refcount));

  napi_value result;
  NAPI_CALL(env, napi_create_uint32(env, refcount, &result));
  return result;
}

static napi_value decrement_refcount(napi_env env, napi_callback_info info) {
  NAPI_ASSERT(env, test_reference != NULL,
              "A reference must have been created.");

  uint32_t refcount;
  NAPI_CALL(env, napi_reference_unref(env, test_reference, &refcount));

  napi_value result;
  NAPI_CALL(env, napi_create_uint32(env, refcount, &result));
  return result;
}

static napi_value get_reference_value(napi_env env, napi_callback_info info) {
  NAPI_ASSERT(env, test_reference != NULL,
              "A reference must have been created.");

  napi_value result;
  NAPI_CALL(env, napi_get_reference_value(env, test_reference, &result));
  return result;
}

static napi_value init(napi_env env, napi_value exports) {
  napi_property_descriptor descriptors[] = {
    DECLARE_NAPI_GETTER("finalizeCount", get_finalize_count),
    DECLARE_NAPI_PROPERTY("createExternal", create_external),
    DECLARE_NAPI_PROPERTY("createExternalWithFinalize",
                          create_external_with_finalize),
    DECLARE_NAPI_PROPERTY("checkExternal", check_external),
    DECLARE_NAPI_PROPERTY("createReference", create_reference),
    DECLARE_NAPI_PROPERTY("deleteReference", delete_reference),
    DECLARE_NAPI_PROPERTY("incrementRefcount", increment_refcount),
    DECLARE_NAPI_PROPERTY("decrementRefcount", decrement_refcount),
    DECLARE_NAPI_GETTER("referenceValue", get_reference_value),
  };

  NAPI_CALL(env, napi_define_properties(env, exports, sizeof(descriptors) /
                                                          sizeof(*descriptors),
                                        descriptors));

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
