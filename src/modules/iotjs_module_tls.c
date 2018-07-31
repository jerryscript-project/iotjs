/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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

#include "iotjs_module_tls.h"
#include "iotjs_module_buffer.h"

#include "stdarg.h"

static void iotjs_tls_context_destroy(iotjs_tls_context_t *tls_context);

static const jerry_object_native_info_t tls_context_native_info = {
  .free_cb = (jerry_object_native_free_callback_t)iotjs_tls_context_destroy
};


static void iotjs_tls_context_destroy(iotjs_tls_context_t *tls_context) {
  if (tls_context->ref_count > 1) {
    tls_context->ref_count--;
    return;
  }

  mbedtls_x509_crt_free(&tls_context->cert_auth);
  mbedtls_x509_crt_free(&tls_context->own_cert);
  mbedtls_pk_free(&tls_context->pkey);
  mbedtls_ctr_drbg_free(&tls_context->ctr_drbg);
  mbedtls_entropy_free(&tls_context->entropy);

  IOTJS_RELEASE(tls_context);
}


static iotjs_tls_context_t *iotjs_tls_context_create(
    const jerry_value_t jobject) {
  iotjs_tls_context_t *tls_context = IOTJS_ALLOC(iotjs_tls_context_t);

  tls_context->ref_count = 1;
  tls_context->context_flags = 0;
  mbedtls_entropy_init(&tls_context->entropy);
  mbedtls_ctr_drbg_init(&tls_context->ctr_drbg);
  mbedtls_pk_init(&tls_context->pkey);
  mbedtls_x509_crt_init(&tls_context->own_cert);
  mbedtls_x509_crt_init(&tls_context->cert_auth);

  jerry_set_object_native_pointer(jobject, tls_context,
                                  &tls_context_native_info);

  return tls_context;
}


IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(tls);


static void iotjs_tls_destroy(iotjs_tls_t *tls_data) {
  mbedtls_ssl_free(&tls_data->ssl);
  mbedtls_ssl_config_free(&tls_data->conf);
  iotjs_tls_context_destroy(tls_data->tls_context);

  IOTJS_RELEASE(tls_data->bio.receive_bio.mem);
  IOTJS_RELEASE(tls_data->bio.send_bio.mem);

  IOTJS_RELEASE(tls_data);
}


static iotjs_tls_t *iotjs_tls_create(const jerry_value_t jobject,
                                     iotjs_tls_context_t *tls_context) {
  iotjs_tls_t *tls_data = IOTJS_ALLOC(iotjs_tls_t);

  tls_context->ref_count++;

  tls_data->tls_context = tls_context;
  mbedtls_ssl_config_init(&tls_data->conf);
  mbedtls_ssl_init(&tls_data->ssl);
  tls_data->state = TLS_HANDSHAKE_READY;

  tls_data->jobject = jobject;
  jerry_set_object_native_pointer(jobject, tls_data, &this_module_native_info);

  return tls_data;
}


static void iotjs_bio_init(iotjs_bio_t *bio, size_t size) {
  bio->mem = (char *)iotjs_buffer_allocate(size);
  bio->size = size;
  bio->read_index = 0;
  bio->write_index = 0;
}


static size_t iotjs_bio_pending(iotjs_bio_t *bio) {
  if (bio->read_index <= bio->write_index) {
    return bio->write_index - bio->read_index;
  }

  return bio->write_index + bio->size - bio->read_index;
}


static size_t iotjs_bio_remaining(iotjs_bio_t *bio) {
  if (bio->write_index < bio->read_index) {
    return bio->read_index - bio->write_index - 1;
  }

  return bio->read_index + bio->size - bio->write_index - 1;
}


static void iotjs_bio_read(iotjs_bio_t *bio, char *buf, size_t size) {
  IOTJS_ASSERT(size <= iotjs_bio_pending(bio));

  if (bio->read_index + size > bio->size) {
    size_t copy_size = bio->size - bio->read_index;

    memcpy(buf, bio->mem + bio->read_index, copy_size);
    size -= copy_size;
    buf += copy_size;
    bio->read_index = 0;
  }

  memcpy(buf, bio->mem + bio->read_index, size);
  bio->read_index += size;
}


static void iotjs_bio_write(iotjs_bio_t *bio, const char *buf, size_t size) {
  IOTJS_ASSERT(size <= iotjs_bio_remaining(bio));

  if (bio->write_index + size > bio->size) {
    size_t copy_size = bio->size - bio->write_index;

    memcpy(bio->mem + bio->write_index, buf, copy_size);
    size -= copy_size;
    buf += copy_size;
    bio->write_index = 0;
  }

  memcpy(bio->mem + bio->write_index, buf, size);
  bio->write_index += size;
}


static int iotjs_bio_net_send(void *ctx, const unsigned char *buf, size_t len) {
  iotjs_bio_t *send_bio = &(((iotjs_bio_pair_t *)ctx)->send_bio);

  size_t remaining = iotjs_bio_remaining(send_bio);

  if (remaining == 0) {
    return MBEDTLS_ERR_SSL_WANT_WRITE;
  }

  if (len > remaining) {
    len = remaining;
  }

  iotjs_bio_write(send_bio, (const char *)buf, len);
  return (int)len;
}

static int iotjs_bio_net_receive(void *ctx, unsigned char *buf, size_t len) {
  iotjs_bio_t *receive_bio = &(((iotjs_bio_pair_t *)ctx)->receive_bio);

  size_t pending = iotjs_bio_pending(receive_bio);

  if (pending == 0) {
    return MBEDTLS_ERR_SSL_WANT_READ;
  }

  if (len > pending) {
    len = pending;
  }

  iotjs_bio_read(receive_bio, (char *)buf, len);
  return (int)len;
}


JS_FUNCTION(TlsContext) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, object);

  jerry_value_t jtls = JS_GET_THIS();
  iotjs_tls_context_t *tls_context = iotjs_tls_context_create(jtls);

  jerry_value_t joptions = JS_GET_ARG(0, object);

  // Set deterministic random bit generator
  if (mbedtls_ctr_drbg_seed(&tls_context->ctr_drbg, mbedtls_entropy_func,
                            &tls_context->entropy, NULL, 0) != 0) {
    return JS_CREATE_ERROR(COMMON, "drbg seeding failed");
  }

  // User provided certificate
  int ret = 0;

  jerry_value_t jcert =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_CERT);
  jerry_value_t jkey =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_KEY);

  iotjs_string_t cert_string;
  iotjs_string_t key_string;

  if (iotjs_jbuffer_as_string(jcert, &cert_string)) {
    const char *cert_chars = iotjs_string_data(&cert_string);

    ret = mbedtls_x509_crt_parse(&tls_context->own_cert,
                                 (const unsigned char *)cert_chars,
                                 (size_t)iotjs_string_size(&cert_string) + 1);

    iotjs_string_destroy(&cert_string);

    if (ret == 0 && iotjs_jbuffer_as_string(jkey, &key_string)) {
      const char *key_chars = iotjs_string_data(&key_string);

      ret = mbedtls_pk_parse_key(&tls_context->pkey,
                                 (const unsigned char *)key_chars,
                                 (size_t)iotjs_string_size(&key_string) + 1,
                                 NULL, 0);

      iotjs_string_destroy(&key_string);

      if (ret == 0) {
        // Both own_cert and pkey must be valid for setting this flag.
        tls_context->context_flags |= SSL_CONTEXT_HAS_KEY;
      }
    } else {
      ret = -1;
    }
  } else if (!jerry_value_is_undefined(jcert) ||
             !jerry_value_is_undefined(jkey)) {
    ret = -1;
  }

  jerry_release_value(jcert);
  jerry_release_value(jkey);

  if (ret != 0) {
    return JS_CREATE_ERROR(COMMON, "key or certificate parsing failed");
  }

  // User provided trusted certificates
  jerry_value_t jcert_auth =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_CA);
  iotjs_string_t cert_auth_string;

  if (iotjs_jbuffer_as_string(jcert_auth, &cert_auth_string)) {
    const char *cert_auth_chars = iotjs_string_data(&cert_auth_string);

    ret = mbedtls_x509_crt_parse(&tls_context->cert_auth,
                                 (const unsigned char *)cert_auth_chars,
                                 (size_t)iotjs_string_size(&cert_auth_string) +
                                     1);

    iotjs_string_destroy(&cert_auth_string);
  } else if (!jerry_value_is_undefined(jcert_auth)) {
    ret = -1;
  } else {
    // Parse the default certificate authority
    ret = mbedtls_x509_crt_parse(&tls_context->cert_auth,
                                 (const unsigned char *)SSL_CA_PEM,
                                 sizeof(SSL_CA_PEM));
  }

  jerry_release_value(jcert_auth);

  if (ret) {
    return JS_CREATE_ERROR(COMMON, "certificate authority (CA) parsing failed");
  }

  return jerry_create_undefined();
}


JS_FUNCTION(TlsInit) {
  DJS_CHECK_ARGS(3, object, object, object);

  jerry_value_t jtls_socket = JS_GET_ARG(0, object);
  jerry_value_t joptions = JS_GET_ARG(1, object);

  // Get context
  jerry_value_t jtls_context = JS_GET_ARG(2, object);

  void *native_ptr;
  const jerry_object_native_info_t *native_info;
  bool tls_context_available =
      jerry_get_object_native_pointer(jtls_context, &native_ptr, &native_info);

  if (!tls_context_available || native_info != &tls_context_native_info) {
    return JS_CREATE_ERROR(COMMON, "secure context not available");
  }

  iotjs_tls_context_t *tls_context = (iotjs_tls_context_t *)native_ptr;

  iotjs_tls_t *tls_data = iotjs_tls_create(jtls_socket, tls_context);

  // Check server
  jerry_value_t jis_server =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_ISSERVER);
  bool is_server = jerry_value_to_boolean(jis_server);
  jerry_release_value(jis_server);

  if (tls_context->context_flags & SSL_CONTEXT_HAS_KEY) {
    if (mbedtls_ssl_conf_own_cert(&tls_data->conf, &tls_context->own_cert,
                                  &tls_context->pkey) != 0) {
      return JS_CREATE_ERROR(COMMON, "certificate/private key cannot be set");
    }
  }

  mbedtls_ssl_conf_ca_chain(&tls_data->conf, &tls_context->cert_auth, NULL);

  mbedtls_ssl_conf_rng(&tls_data->conf, mbedtls_ctr_drbg_random,
                       &tls_context->ctr_drbg);

  int endpoint = is_server ? MBEDTLS_SSL_IS_SERVER : MBEDTLS_SSL_IS_CLIENT;

  if (mbedtls_ssl_config_defaults(&tls_data->conf, endpoint,
                                  MBEDTLS_SSL_TRANSPORT_STREAM,
                                  MBEDTLS_SSL_PRESET_DEFAULT)) {
    return JS_CREATE_ERROR(COMMON, "SSL Configuration failed");
  }

  // if true, verifies CAs, must emit error if fails
  int auth_mode =
      is_server ? MBEDTLS_SSL_VERIFY_NONE : MBEDTLS_SSL_VERIFY_REQUIRED;

  jerry_value_t jauth_mode =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_REJECTUNAUTHORIZED);

  if (!jerry_value_is_undefined(jauth_mode)) {
    if (jerry_value_to_boolean(jauth_mode)) {
      auth_mode = MBEDTLS_SSL_VERIFY_REQUIRED;
    } else {
      auth_mode = MBEDTLS_SSL_VERIFY_OPTIONAL;
    }
  }

  jerry_release_value(jauth_mode);

  mbedtls_ssl_conf_authmode(&tls_data->conf, auth_mode);

  if (mbedtls_ssl_setup(&tls_data->ssl, &tls_data->conf)) {
    return JS_CREATE_ERROR(COMMON, "SSL setup failed");
  }

  // Connect mbedtls with iotjs_net_send and iotjs_net_recv functions
  iotjs_bio_init(&(tls_data->bio.receive_bio), SSL_BIO_SIZE);
  iotjs_bio_init(&(tls_data->bio.send_bio), SSL_BIO_SIZE);
  mbedtls_ssl_set_bio(&tls_data->ssl, &(tls_data->bio), iotjs_bio_net_send,
                      iotjs_bio_net_receive, NULL);

  return jerry_create_undefined();
}


JS_FUNCTION(Connect) {
  JS_DECLARE_THIS_PTR(tls, tls_data);
  DJS_CHECK_ARGS(1, string);

  if (tls_data->state == TLS_HANDSHAKE_READY) {
    iotjs_string_t server_name = JS_GET_ARG(0, string);
    mbedtls_ssl_set_hostname(&tls_data->ssl, iotjs_string_data(&server_name));
    iotjs_string_destroy(&server_name);
  }

  return jerry_create_undefined();
}


static void iotjs_tls_send_pending(iotjs_tls_t *tls_data) {
  iotjs_bio_t *send_bio = &(tls_data->bio.send_bio);
  size_t pending = iotjs_bio_pending(send_bio);

  if (pending == 0) {
    return;
  }

  jerry_value_t jbuffer = iotjs_bufferwrap_create_buffer(pending);
  iotjs_bufferwrap_t *buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);
  iotjs_bio_read(send_bio, buffer_wrap->buffer, pending);

  jerry_value_t jthis = tls_data->jobject;
  jerry_value_t fn = iotjs_jval_get_property(jthis, IOTJS_MAGIC_STRING_ONWRITE);

  iotjs_invoke_callback(fn, jthis, &jbuffer, 1);

  jerry_release_value(fn);
  jerry_release_value(jbuffer);
}


static void iotjs_tls_notify_error(iotjs_tls_t *tls_data) {
  jerry_value_t jerror = jerry_create_string((const jerry_char_t *)"error");
  jerry_value_t jmessage =
      jerry_create_string((const jerry_char_t *)"TLS error");

  jerry_value_t jthis = tls_data->jobject;
  jerry_value_t fn = iotjs_jval_get_property(jthis, IOTJS_MAGIC_STRING_EMIT);

  jerry_value_t jargv[2] = { jerror, jmessage };
  iotjs_invoke_callback(fn, jthis, jargv, 2);

  jerry_release_value(fn);
  jerry_release_value(jargv[0]);
  jerry_release_value(jargv[1]);
}


JS_FUNCTION(Write) {
  JS_DECLARE_THIS_PTR(tls, tls_data);

  if (tls_data->state != TLS_CONNECTED) {
    return jerry_create_null();
  }

  const unsigned char *data = NULL;
  size_t length = 0;
  bool is_end = false;

  if (jargc >= 1 && jerry_value_to_boolean(jargv[0])) {
    jerry_value_t jbuffer = JS_GET_ARG(0, object);

    iotjs_bufferwrap_t *buf = iotjs_bufferwrap_from_jbuffer(jbuffer);
    data = (const unsigned char *)buf->buffer;
    length = iotjs_bufferwrap_length(buf);
  }

  if (jargc >= 2 && jerry_value_to_boolean(jargv[1])) {
    is_end = true;
  }

  while (true) {
    int ret_val = mbedtls_ssl_write(&tls_data->ssl, data, length);

    if ((int)length == ret_val) {
      break;
    }

    iotjs_tls_send_pending(tls_data);

    if (ret_val > 0) {
      data += ret_val;
      length -= (size_t)ret_val;
    } else if (ret_val != MBEDTLS_ERR_SSL_WANT_WRITE) {
      tls_data->state = TLS_CLOSED;
      return jerry_create_null();
    }
  }

  if (is_end) {
    while (true) {
      int ret_val = mbedtls_ssl_close_notify(&tls_data->ssl);
      if (ret_val == 0) {
        tls_data->state = TLS_CLOSED;
        break;
      }

      iotjs_tls_send_pending(tls_data);

      if (ret_val != MBEDTLS_ERR_SSL_WANT_WRITE) {
        iotjs_tls_notify_error(tls_data);
        tls_data->state = TLS_CLOSED;
        return jerry_create_null();
      }
    }
  }

  /* Last package is returned as a buffer. */
  iotjs_bio_t *send_bio = &(tls_data->bio.send_bio);
  size_t pending = iotjs_bio_pending(send_bio);

  jerry_value_t jbuffer = iotjs_bufferwrap_create_buffer(pending);
  iotjs_bufferwrap_t *buf = iotjs_bufferwrap_from_jbuffer(jbuffer);
  iotjs_bio_read(send_bio, buf->buffer, pending);

  return jbuffer;
}


static void tls_handshake(iotjs_tls_t *tls_data, jerry_value_t jthis) {
  tls_data->state = TLS_HANDSHAKE_IN_PROGRESS;
  // Continue handshaking process
  int ret_val = mbedtls_ssl_handshake(&tls_data->ssl);

  iotjs_tls_send_pending(tls_data);

  bool error;
  bool authorized;

  // Check whether handshake completed
  if (ret_val == 0) {
    tls_data->state = TLS_CONNECTED;
    error = false;
    authorized = mbedtls_ssl_get_verify_result(&tls_data->ssl) == 0;
  } else {
    if (ret_val == MBEDTLS_ERR_SSL_WANT_READ ||
        ret_val == MBEDTLS_ERR_SSL_WANT_WRITE) {
      return;
    }

    tls_data->state = TLS_CLOSED;
    error = true;
    authorized = false;
  }

  // Result of certificate verification
  jerry_value_t jargv[2] = { jerry_create_boolean(error),
                             jerry_create_boolean(authorized) };

  jerry_value_t fn =
      iotjs_jval_get_property(jthis, IOTJS_MAGIC_STRING_ONHANDSHAKEDONE);
  iotjs_invoke_callback(fn, jthis, jargv, 2);

  jerry_release_value(fn);
  jerry_release_value(jargv[0]);
  jerry_release_value(jargv[1]);
}


JS_FUNCTION(Read) {
  JS_DECLARE_THIS_PTR(tls, tls_data);

  if (tls_data->state == TLS_CLOSED) {
    return jerry_create_boolean(false);
  }

  iotjs_bio_t *receive_bio = &(tls_data->bio.receive_bio);
  const char *data = NULL;
  size_t length = 0;

  if (jargc >= 1 && jerry_value_to_boolean(jargv[0])) {
    jerry_value_t jbuffer = JS_GET_ARG(0, object);

    iotjs_bufferwrap_t *buf = iotjs_bufferwrap_from_jbuffer(jbuffer);
    data = buf->buffer;
    length = iotjs_bufferwrap_length(buf);
  }

  do {
    size_t copy_size = iotjs_bio_remaining(receive_bio);

    if (copy_size > length) {
      copy_size = length;
    }

    iotjs_bio_write(receive_bio, data, copy_size);
    data += copy_size;
    length -= copy_size;

    if (tls_data->state != TLS_CONNECTED) {
      IOTJS_ASSERT(tls_data->state == TLS_HANDSHAKE_READY ||
                   tls_data->state == TLS_HANDSHAKE_IN_PROGRESS);
      tls_handshake(tls_data, jthis);

      if (tls_data->state != TLS_CONNECTED) {
        IOTJS_ASSERT(tls_data->state == TLS_HANDSHAKE_IN_PROGRESS ||
                     tls_data->state == TLS_CLOSED);

        if (length > 0 && tls_data->state == TLS_HANDSHAKE_IN_PROGRESS) {
          continue;
        }

        bool result = (tls_data->state != TLS_CLOSED);
        return jerry_create_boolean(result);
      }
    }

    while (true) {
      int ret_val = mbedtls_ssl_read(&tls_data->ssl, NULL, 0);
      iotjs_tls_send_pending(tls_data);

      if (ret_val == 0) {
        size_t pending = mbedtls_ssl_get_bytes_avail(&tls_data->ssl);

        if (pending == 0) {
          continue;
        }

        jerry_value_t jbuffer = iotjs_bufferwrap_create_buffer(pending);
        iotjs_bufferwrap_t *buf = iotjs_bufferwrap_from_jbuffer(jbuffer);
        ret_val = mbedtls_ssl_read(&tls_data->ssl, (unsigned char *)buf->buffer,
                                   pending);

        IOTJS_ASSERT(ret_val == (int)pending);
        IOTJS_UNUSED(ret_val);

        jerry_value_t fn =
            iotjs_jval_get_property(jthis, IOTJS_MAGIC_STRING_ONREAD);
        iotjs_invoke_callback(fn, jthis, &jbuffer, 1);

        jerry_release_value(jbuffer);
        jerry_release_value(fn);
        continue;
      }

      if (ret_val == MBEDTLS_ERR_SSL_WANT_READ) {
        break;
      }

      if (ret_val == MBEDTLS_ERR_SSL_WANT_WRITE) {
        continue;
      }

      tls_data->state = TLS_CLOSED;

      if (ret_val == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
        return jerry_create_boolean(true);
      }

      iotjs_tls_notify_error(tls_data);
      return jerry_create_boolean(false);
    }
  } while (length > 0);

  return jerry_create_boolean(true);
}


jerry_value_t InitTls() {
  jerry_value_t jtls = jerry_create_object();

  iotjs_jval_set_method(jtls, IOTJS_MAGIC_STRING_CONNECT, Connect);
  iotjs_jval_set_method(jtls, IOTJS_MAGIC_STRING_READ, Read);
  iotjs_jval_set_method(jtls, IOTJS_MAGIC_STRING_TLSCONTEXT, TlsContext);
  iotjs_jval_set_method(jtls, IOTJS_MAGIC_STRING_TLSINIT, TlsInit);
  iotjs_jval_set_method(jtls, IOTJS_MAGIC_STRING_WRITE, Write);

  return jtls;
}
