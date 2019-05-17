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

/*
 *  FIPS-180-1 compliant SHA-1 implementation
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

/*
 *  The SHA-1 standard was published by NIST in 1993.
 *
 *  http://www.itl.nist.gov/fipspubs/fip180-1.htm
 */

#include "iotjs_def.h"
#include "iotjs_module_crypto.h"
#include "iotjs_module_buffer.h"

/* These enum values are the same as the ones in crypto.js as well as the
   corresponding ones in sha.h in mbedTLS.*/
typedef enum {
  IOTJS_CRYPTO_SHA1 = 4,
  IOTJS_CRYPTO_SHA256 = 6,
} iotjs_crypto_sha_t;

#if !ENABLE_MODULE_TLS
const char no_tls_err_str[] = "TLS module must be enabled to use this feature";

/*
 * 32-bit integer manipulation macros (big endian)
 */
#ifndef GET_UINT32_BE
#define GET_UINT32_BE(n, b, i)                                          \
  {                                                                     \
    (n) = ((uint32_t)(b)[(i)] << 24) | ((uint32_t)(b)[(i) + 1] << 16) | \
          ((uint32_t)(b)[(i) + 2] << 8) | ((uint32_t)(b)[(i) + 3]);     \
  }
#endif

#ifndef PUT_UINT32_BE
#define PUT_UINT32_BE(n, b, i)                 \
  {                                            \
    (b)[(i)] = (unsigned char)((n) >> 24);     \
    (b)[(i) + 1] = (unsigned char)((n) >> 16); \
    (b)[(i) + 2] = (unsigned char)((n) >> 8);  \
    (b)[(i) + 3] = (unsigned char)((n));       \
  }
#endif

static int iotjs_sha1_process(uint32_t state[5], const unsigned char data[64]) {
  uint32_t temp, W[16], A, B, C, D, E;

  GET_UINT32_BE(W[0], data, 0);
  GET_UINT32_BE(W[1], data, 4);
  GET_UINT32_BE(W[2], data, 8);
  GET_UINT32_BE(W[3], data, 12);
  GET_UINT32_BE(W[4], data, 16);
  GET_UINT32_BE(W[5], data, 20);
  GET_UINT32_BE(W[6], data, 24);
  GET_UINT32_BE(W[7], data, 28);
  GET_UINT32_BE(W[8], data, 32);
  GET_UINT32_BE(W[9], data, 36);
  GET_UINT32_BE(W[10], data, 40);
  GET_UINT32_BE(W[11], data, 44);
  GET_UINT32_BE(W[12], data, 48);
  GET_UINT32_BE(W[13], data, 52);
  GET_UINT32_BE(W[14], data, 56);
  GET_UINT32_BE(W[15], data, 60);

#define S(x, n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define R(t)                                                           \
  (temp = W[(t - 3) & 0x0F] ^ W[(t - 8) & 0x0F] ^ W[(t - 14) & 0x0F] ^ \
          W[t & 0x0F],                                                 \
   (W[t & 0x0F] = S(temp, 1)))

#define P(a, b, c, d, e, x)            \
  {                                    \
    e += S(a, 5) + F(b, c, d) + K + x; \
    b = S(b, 30);                      \
  }

  A = state[0];
  B = state[1];
  C = state[2];
  D = state[3];
  E = state[4];

#define F(x, y, z) (z ^ (x & (y ^ z)))
#define K 0x5A827999

  P(A, B, C, D, E, W[0]);
  P(E, A, B, C, D, W[1]);
  P(D, E, A, B, C, W[2]);
  P(C, D, E, A, B, W[3]);
  P(B, C, D, E, A, W[4]);
  P(A, B, C, D, E, W[5]);
  P(E, A, B, C, D, W[6]);
  P(D, E, A, B, C, W[7]);
  P(C, D, E, A, B, W[8]);
  P(B, C, D, E, A, W[9]);
  P(A, B, C, D, E, W[10]);
  P(E, A, B, C, D, W[11]);
  P(D, E, A, B, C, W[12]);
  P(C, D, E, A, B, W[13]);
  P(B, C, D, E, A, W[14]);
  P(A, B, C, D, E, W[15]);
  P(E, A, B, C, D, R(16));
  P(D, E, A, B, C, R(17));
  P(C, D, E, A, B, R(18));
  P(B, C, D, E, A, R(19));

#undef K
#undef F

#define F(x, y, z) (x ^ y ^ z)
#define K 0x6ED9EBA1

  P(A, B, C, D, E, R(20));
  P(E, A, B, C, D, R(21));
  P(D, E, A, B, C, R(22));
  P(C, D, E, A, B, R(23));
  P(B, C, D, E, A, R(24));
  P(A, B, C, D, E, R(25));
  P(E, A, B, C, D, R(26));
  P(D, E, A, B, C, R(27));
  P(C, D, E, A, B, R(28));
  P(B, C, D, E, A, R(29));
  P(A, B, C, D, E, R(30));
  P(E, A, B, C, D, R(31));
  P(D, E, A, B, C, R(32));
  P(C, D, E, A, B, R(33));
  P(B, C, D, E, A, R(34));
  P(A, B, C, D, E, R(35));
  P(E, A, B, C, D, R(36));
  P(D, E, A, B, C, R(37));
  P(C, D, E, A, B, R(38));
  P(B, C, D, E, A, R(39));

#undef K
#undef F

#define F(x, y, z) ((x & y) | (z & (x | y)))
#define K 0x8F1BBCDC

  P(A, B, C, D, E, R(40));
  P(E, A, B, C, D, R(41));
  P(D, E, A, B, C, R(42));
  P(C, D, E, A, B, R(43));
  P(B, C, D, E, A, R(44));
  P(A, B, C, D, E, R(45));
  P(E, A, B, C, D, R(46));
  P(D, E, A, B, C, R(47));
  P(C, D, E, A, B, R(48));
  P(B, C, D, E, A, R(49));
  P(A, B, C, D, E, R(50));
  P(E, A, B, C, D, R(51));
  P(D, E, A, B, C, R(52));
  P(C, D, E, A, B, R(53));
  P(B, C, D, E, A, R(54));
  P(A, B, C, D, E, R(55));
  P(E, A, B, C, D, R(56));
  P(D, E, A, B, C, R(57));
  P(C, D, E, A, B, R(58));
  P(B, C, D, E, A, R(59));

#undef K
#undef F

#define F(x, y, z) (x ^ y ^ z)
#define K 0xCA62C1D6

  P(A, B, C, D, E, R(60));
  P(E, A, B, C, D, R(61));
  P(D, E, A, B, C, R(62));
  P(C, D, E, A, B, R(63));
  P(B, C, D, E, A, R(64));
  P(A, B, C, D, E, R(65));
  P(E, A, B, C, D, R(66));
  P(D, E, A, B, C, R(67));
  P(C, D, E, A, B, R(68));
  P(B, C, D, E, A, R(69));
  P(A, B, C, D, E, R(70));
  P(E, A, B, C, D, R(71));
  P(D, E, A, B, C, R(72));
  P(C, D, E, A, B, R(73));
  P(B, C, D, E, A, R(74));
  P(A, B, C, D, E, R(75));
  P(E, A, B, C, D, R(76));
  P(D, E, A, B, C, R(77));
  P(C, D, E, A, B, R(78));
  P(B, C, D, E, A, R(79));

#undef K
#undef F

  state[0] += A;
  state[1] += B;
  state[2] += C;
  state[3] += D;
  state[4] += E;

  return (0);
}


static const unsigned char sha1_padding[64] = { 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                0,    0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                0,    0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                0,    0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                0,    0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                0,    0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                0,    0, 0, 0 };


static int iotjs_sha1_update(uint32_t total[2], uint32_t state[5],
                             unsigned char buffer[64],
                             const unsigned char *in_buff, size_t buff_len) {
  int ret;
  size_t fill;
  uint32_t left;

  if (buff_len == 0) {
    return 0;
  }

  left = total[0] & 0x3F;
  fill = 64 - left;

  total[0] += (uint32_t)buff_len;
  total[0] &= 0xFFFFFFFF;

  if (total[0] < (uint32_t)buff_len) {
    total[1]++;
  }

  if (left && buff_len >= fill) {
    memcpy((void *)(buffer + left), in_buff, fill);

    if ((ret = iotjs_sha1_process(state, buffer)) != 0) {
      return ret;
    }

    in_buff += fill;
    buff_len -= fill;
    left = 0;
  }

  while (buff_len >= 64) {
    if ((ret = iotjs_sha1_process(state, buffer)) != 0) {
      return ret;
    }
  }

  if (buff_len > 0) {
    memcpy((void *)(buffer + left), in_buff, buff_len);
  }

  return 0;
}


static int iotjs_sha1_finish(uint32_t total[2], uint32_t state[5],
                             unsigned char buffer[64],
                             unsigned char *out_buff) {
  int ret;
  uint32_t last, padn;
  uint32_t high, low;
  unsigned char msglen[8];

  high = (total[0] >> 29) | (total[1] << 3);
  low = (total[0] << 3);

  PUT_UINT32_BE(high, msglen, 0);
  PUT_UINT32_BE(low, msglen, 4);

  last = total[0] & 0x3F;
  padn = (last < 56) ? (56 - last) : (120 - last);

  if ((ret = iotjs_sha1_update(total, state, buffer, sha1_padding, padn)) !=
      0) {
    return ret;
  }

  if ((ret = iotjs_sha1_update(total, state, buffer, msglen, 8)) != 0) {
    return ret;
  }

  PUT_UINT32_BE(state[0], out_buff, 0);
  PUT_UINT32_BE(state[1], out_buff, 4);
  PUT_UINT32_BE(state[2], out_buff, 8);
  PUT_UINT32_BE(state[3], out_buff, 12);
  PUT_UINT32_BE(state[4], out_buff, 16);

  return 0;
}
#else /* ENABLE_MODULE_TLS */

#include "mbedtls/pk.h"
#include "mbedtls/sha1.h"
#include "mbedtls/sha256.h"

#endif /* !ENABLE_MODULE_TLS */

size_t iotjs_sha1_encode(unsigned char **out_buff, const unsigned char *in_buff,
                         size_t buff_len) {
  size_t sha1_size = 20; // 160 bytes
  *out_buff = IOTJS_CALLOC(sha1_size, unsigned char);
#if !ENABLE_MODULE_TLS
  uint32_t total[2] = { 0 };
  uint32_t state[5] = { 0 };
  unsigned char buffer[64] = { 0 };

  total[0] = 0;
  total[1] = 0;

  state[0] = 0x67452301;
  state[1] = 0xEFCDAB89;
  state[2] = 0x98BADCFE;
  state[3] = 0x10325476;
  state[4] = 0xC3D2E1F0;

  iotjs_sha1_update(total, state, buffer, in_buff, buff_len);
  iotjs_sha1_finish(total, state, buffer, *out_buff);
#else /* ENABLE_MODULE_TLS */
  mbedtls_sha1_context sha_ctx;
  mbedtls_sha1_init(&sha_ctx);
#if defined(__TIZENRT__)
  mbedtls_sha1_starts(&sha_ctx);
  mbedtls_sha1_update(&sha_ctx, in_buff, buff_len);
  mbedtls_sha1_finish(&sha_ctx, *out_buff);
#else  /* !__TIZENRT__ */
  mbedtls_sha1_starts_ret(&sha_ctx);
  mbedtls_sha1_update_ret(&sha_ctx, in_buff, buff_len);
  mbedtls_sha1_finish_ret(&sha_ctx, *out_buff);
#endif /* __TIZENRT__ */
  mbedtls_sha1_free(&sha_ctx);
#endif /* ENABLE_MODULE_TLS */

  return sha1_size;
}


#if ENABLE_MODULE_TLS
size_t iotjs_sha256_encode(unsigned char **out_buff,
                           const unsigned char *in_buff, size_t buff_len) {
  size_t sha256_size = 32;
  *out_buff = IOTJS_CALLOC(sha256_size, unsigned char);

  mbedtls_sha256_context sha_ctx;
  mbedtls_sha256_init(&sha_ctx);
#if defined(__TIZENRT__)
  mbedtls_sha256_starts(&sha_ctx, 0);
  mbedtls_sha256_update(&sha_ctx, in_buff, buff_len);
  mbedtls_sha256_finish(&sha_ctx, *out_buff);
#else  /* !__TIZENRT__ */
  mbedtls_sha256_starts_ret(&sha_ctx, 0);
  mbedtls_sha256_update_ret(&sha_ctx, in_buff, buff_len);
  mbedtls_sha256_finish_ret(&sha_ctx, *out_buff);
#endif /* __TIZENRT__ */
  mbedtls_sha256_free(&sha_ctx);

  return sha256_size;
}
#endif /* ENABLE_MODULE_TLS */


JS_FUNCTION(sha_encode) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(2, any, number);

  uint8_t type = JS_GET_ARG(1, number);

  jerry_value_t jstring = JS_GET_ARG(0, any);
  iotjs_string_t user_str = iotjs_string_create();

  if (!iotjs_jbuffer_as_string(jstring, &user_str)) {
    return jerry_create_undefined();
  }

  const unsigned char *user_str_data =
      (const unsigned char *)iotjs_string_data(&user_str);
  size_t user_str_sz = iotjs_string_size(&user_str);

  size_t sha_sz = 0;

  unsigned char *sha_ret = NULL;

  switch (type) {
    case IOTJS_CRYPTO_SHA1: {
      sha_sz = iotjs_sha1_encode(&sha_ret, user_str_data, user_str_sz);
      break;
    }
    case IOTJS_CRYPTO_SHA256: {
#if !ENABLE_MODULE_TLS
      iotjs_string_destroy(&user_str);
      return JS_CREATE_ERROR(COMMON, no_tls_err_str);
#else  /* ENABLE_MODULE_TLS */
      sha_sz = iotjs_sha256_encode(&sha_ret, user_str_data, user_str_sz);
      break;
#endif /* !ENABLE_MODULE_TLS */
    }
    default: {
      iotjs_string_destroy(&user_str);
      return JS_CREATE_ERROR(COMMON, "Unknown SHA hashing algorithm");
    }
  }

  iotjs_string_destroy(&user_str);

  jerry_value_t ret_val;
  ret_val = iotjs_bufferwrap_create_buffer(sha_sz);
  iotjs_bufferwrap_t *ret_wrap = iotjs_bufferwrap_from_jbuffer(ret_val);
  memcpy(ret_wrap->buffer, sha_ret, sha_sz);
  ret_wrap->length = sha_sz;

  IOTJS_RELEASE(sha_ret);
  return ret_val;
}


JS_FUNCTION(rsa_verify) {
#if !ENABLE_MODULE_TLS
  return JS_CREATE_ERROR(COMMON, no_tls_err_str);
#else  /* ENABLE_MODULE_TLS */
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(2, any, any);

  uint8_t type = JS_GET_ARG(0, number);
  jerry_value_t jdata = JS_GET_ARG(1, any);
  jerry_value_t jkey = JS_GET_ARG(2, any);
  jerry_value_t jsignature = JS_GET_ARG(3, any);

  iotjs_string_t key = iotjs_string_create();
  iotjs_string_t data = iotjs_string_create();
  iotjs_string_t signature = iotjs_string_create();

  if ((!iotjs_jbuffer_as_string(jkey, &key)) ||
      (!iotjs_jbuffer_as_string(jdata, &data)) ||
      (!iotjs_jbuffer_as_string(jsignature, &signature))) {
    iotjs_string_destroy(&key);
    iotjs_string_destroy(&data);
    iotjs_string_destroy(&signature);

    return jerry_create_boolean(false);
  }

  mbedtls_pk_context pk;

  char *raw_signature = NULL;
  size_t raw_signature_sz =
      iotjs_base64_decode(&raw_signature, iotjs_string_data(&signature),
                          iotjs_string_size(&signature));
  mbedtls_pk_init(&pk);
  int ret_val =
      mbedtls_pk_parse_public_key(&pk, (const unsigned char *)iotjs_string_data(
                                           &key),
                                  iotjs_string_size(&key) + 1);


  jerry_value_t js_ret_val = jerry_create_boolean(true);
  if ((ret_val =
           mbedtls_pk_verify(&pk, type,
                             (const unsigned char *)iotjs_string_data(&data), 0,
                             (const unsigned char *)raw_signature,
                             raw_signature_sz))) {
    js_ret_val = jerry_create_boolean(false);
  }

  iotjs_string_destroy(&key);
  iotjs_string_destroy(&data);
  iotjs_string_destroy(&signature);
  mbedtls_pk_free(&pk);
  IOTJS_RELEASE(raw_signature);

  return js_ret_val;
#endif /* !ENABLE_MODULE_TLS */
}


JS_FUNCTION(base64_encode) {
  DJS_CHECK_THIS();

  jerry_value_t jstring = JS_GET_ARG(0, any);
  iotjs_string_t user_str = iotjs_string_create();

  if (!iotjs_jbuffer_as_string(jstring, &user_str)) {
    return jerry_create_undefined();
  }

  unsigned char *out_buff = NULL;
  size_t out_size =
      iotjs_base64_encode(&out_buff,
                          (const unsigned char *)iotjs_string_data(&user_str),
                          iotjs_string_size(&user_str));

  iotjs_string_destroy(&user_str);
  jerry_value_t ret_val = jerry_create_string_sz(out_buff, out_size);

  IOTJS_RELEASE(out_buff);
  return ret_val;
}


jerry_value_t iotjs_init_crypto(void) {
  jerry_value_t jcrypto = jerry_create_object();

  iotjs_jval_set_method(jcrypto, IOTJS_MAGIC_STRING_SHAENCODE, sha_encode);
  iotjs_jval_set_method(jcrypto, IOTJS_MAGIC_STRING_BASE64ENCODE,
                        base64_encode);
  iotjs_jval_set_method(jcrypto, IOTJS_MAGIC_STRING_RSAVERIFY, rsa_verify);

  return jcrypto;
}
