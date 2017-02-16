/* Copyright 2017 Samsung Electronics Co., Ltd. and other contributors
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

#include "coap.h"

#include "iotjs_def.h"

#include "iotjs_module_coap.h"

#include "iotjs_reqwrap.h"

#define COAP_RESOURCE_CHECK_TIME 2

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

/* temporary storage for dynamic resource representations */
static int quit = 0;

/* changeable clock base (see handle_put_time()) */
static time_t clock_offset;
static time_t my_clock_base = 0;

struct coap_resource_t *time_resource = NULL;

#ifndef WITHOUT_ASYNC
/* This variable is used to mimic long-running tasks that require
 * asynchronous responses. */
static coap_async_state_t *async = NULL;
#endif /* WITHOUT_ASYNC */

#ifdef __GNUC__
#define UNUSED_PARAM __attribute__((unused))
#else /* not a GCC */
#define UNUSED_PARAM
#endif /* GCC */

#define INDEX                                \
  "This is a test server made with libcoap " \
  "(see https://libcoap.net)\n"              \
  "Copyright (C) 2017 Samsung Electronics\n\n"


static void hnd_get_index(coap_context_t *ctx UNUSED_PARAM,
                          struct coap_resource_t *resource UNUSED_PARAM,
                          const coap_endpoint_t *local_interface UNUSED_PARAM,
                          coap_address_t *peer UNUSED_PARAM,
                          coap_pdu_t *request UNUSED_PARAM,
                          str *token UNUSED_PARAM, coap_pdu_t *response) {
  unsigned char buf[3];

  response->hdr->code = COAP_RESPONSE_CODE(205);

  coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);

  coap_add_option(response, COAP_OPTION_MAXAGE,
                  coap_encode_var_bytes(buf, 0x2ffff), buf);

  coap_add_data(response, strlen(INDEX), (unsigned char *)INDEX);
}

static void hnd_get_time(coap_context_t *ctx, struct coap_resource_t *resource,
                         const coap_endpoint_t *local_interface UNUSED_PARAM,
                         coap_address_t *peer, coap_pdu_t *request, str *token,
                         coap_pdu_t *response) {
  coap_opt_iterator_t opt_iter;
  coap_opt_t *option;
  unsigned char buf[40];
  size_t len;
  time_t now;
  coap_tick_t t;

  /* FIXME: return time, e.g. in human-readable by default and ticks
   * when query ?ticks is given. */

  /* if my_clock_base was deleted, we pretend to have no such resource */
  response->hdr->code =
      my_clock_base ? COAP_RESPONSE_CODE(205) : COAP_RESPONSE_CODE(404);

  if (coap_find_observer(resource, peer, token)) {
    /* FIXME: need to check for resource->dirty? */
    coap_add_option(response, COAP_OPTION_OBSERVE,
                    coap_encode_var_bytes(buf, ctx->observe), buf);
  }

  if (my_clock_base)
    coap_add_option(response, COAP_OPTION_CONTENT_FORMAT,
                    coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);

  coap_add_option(response, COAP_OPTION_MAXAGE,
                  coap_encode_var_bytes(buf, 0x01), buf);

  if (my_clock_base) {
    /* calculate current time */
    coap_ticks(&t);
    now = my_clock_base + (t / COAP_TICKS_PER_SECOND);
    int outputmode = 0;
    if (request != NULL) {
      option = coap_check_option(request, COAP_OPTION_URI_QUERY, &opt_iter);
      if (option) {
        int memcmplen = min(5, COAP_OPT_LENGTH(option));
        if (memcmp(COAP_OPT_VALUE(option), "ticks", memcmplen) == 0) {
          outputmode = 1;
        }
      }
    }
    if (outputmode == 1) {
      /* output ticks */
      len = snprintf((char *)buf,
                     min(sizeof(buf), response->max_size - response->length),
                     "%u", (unsigned int)now);
      coap_add_data(response, len, buf);

    } else { /* output human-readable time */
      struct tm *tmp;
      tmp = gmtime(&now);
      len = strftime((char *)buf,
                     min(sizeof(buf), response->max_size - response->length),
                     "%b %d %H:%M:%S", tmp);
      coap_add_data(response, len, buf);
    }
  }
}

static void hnd_put_time(coap_context_t *ctx UNUSED_PARAM,
                         struct coap_resource_t *resource UNUSED_PARAM,
                         const coap_endpoint_t *local_interface UNUSED_PARAM,
                         coap_address_t *peer UNUSED_PARAM, coap_pdu_t *request,
                         str *token UNUSED_PARAM, coap_pdu_t *response) {
  coap_tick_t t;
  size_t size;
  unsigned char *data;

  /* FIXME: re-set my_clock_base to clock_offset if my_clock_base == 0
   * and request is empty. When not empty, set to value in request payload
   * (insist on query ?ticks). Return Created or Ok.
   */

  /* if my_clock_base was deleted, we pretend to have no such resource */
  response->hdr->code =
      my_clock_base ? COAP_RESPONSE_CODE(204) : COAP_RESPONSE_CODE(201);

  resource->dirty = 1;

  /* coap_get_data() sets size to 0 on error */
  (void)coap_get_data(request, &size, &data);

  if (size == 0) /* re-init */
    my_clock_base = clock_offset;
  else {
    my_clock_base = 0;
    coap_ticks(&t);
    while (size--)
      my_clock_base = my_clock_base * 10 + *data++;
    my_clock_base -= t / COAP_TICKS_PER_SECOND;
  }
}

static void hnd_delete_time(coap_context_t *ctx UNUSED_PARAM,
                            struct coap_resource_t *resource UNUSED_PARAM,
                            const coap_endpoint_t *local_interface UNUSED_PARAM,
                            coap_address_t *peer UNUSED_PARAM,
                            coap_pdu_t *request UNUSED_PARAM,
                            str *token UNUSED_PARAM,
                            coap_pdu_t *response UNUSED_PARAM) {
  my_clock_base = 0; /* mark clock as "deleted" */

  /* type = request->hdr->type == COAP_MESSAGE_CON  */
  /*   ? COAP_MESSAGE_ACK : COAP_MESSAGE_NON; */
}

static void hnd_get_async(coap_context_t *ctx,
                          struct coap_resource_t *resource UNUSED_PARAM,
                          const coap_endpoint_t *local_interface UNUSED_PARAM,
                          coap_address_t *peer, coap_pdu_t *request,
                          str *token UNUSED_PARAM, coap_pdu_t *response) {
  coap_opt_iterator_t opt_iter;
  coap_opt_t *option;
  unsigned long delay = 5;
  size_t size;

  if (async) {
    if (async->id != request->hdr->id) {
      coap_opt_filter_t f;
      coap_option_filter_clear(f);
      response->hdr->code = COAP_RESPONSE_CODE(503);
    }
    return;
  }

  option = coap_check_option(request, COAP_OPTION_URI_QUERY, &opt_iter);
  if (option) {
    unsigned char *p = COAP_OPT_VALUE(option);

    delay = 0;
    for (size = COAP_OPT_LENGTH(option); size; --size, ++p)
      delay = delay * 10 + (*p - '0');
  }

  async = coap_register_async(ctx, peer, request,
                              COAP_ASYNC_SEPARATE | COAP_ASYNC_CONFIRM,
                              (void *)(COAP_TICKS_PER_SECOND * delay));
}

#define COAP_ADD_ATTR(resource, name, nlen, val, vlen, flags) \
  coap_add_attr((resource), (unsigned char *)(name), (nlen),  \
                (unsigned char *)(val), (vlen), (flags));

#define COAP_RESOURCE_INIT(uri, len, flags) \
  coap_resource_init((unsigned char *)(uri), (len), (flags));

static void init_resources(coap_context_t *ctx) {
  coap_resource_t *r;

  r = COAP_RESOURCE_INIT(NULL, 0, 0);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_index);

  COAP_ADD_ATTR(r, "ct", 2, "0", 1, 0);
  COAP_ADD_ATTR(r, "title", 5, "\"General Info\"", 14, 0);
  coap_add_resource(ctx, r);

  /* store clock base to use in /time */
  my_clock_base = clock_offset;

  r = COAP_RESOURCE_INIT("time", 4, COAP_RESOURCE_FLAGS_NOTIFY_CON);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_time);
  coap_register_handler(r, COAP_REQUEST_PUT, hnd_put_time);
  coap_register_handler(r, COAP_REQUEST_DELETE, hnd_delete_time);

  COAP_ADD_ATTR(r, "ct", 2, "0", 1, 0);
  COAP_ADD_ATTR(r, "title", 5, "\"Internal Clock\"", 16, 0);
  COAP_ADD_ATTR(r, "rt", 2, "\"Ticks\"", 7, 0);
  r->observable = 1;
  COAP_ADD_ATTR(r, "if", 2, "\"clock\"", 7, 0);

  coap_add_resource(ctx, r);
  time_resource = r;

#ifndef WITHOUT_ASYNC
  r = COAP_RESOURCE_INIT("async", 5, 0);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_async);

  COAP_ADD_ATTR(r, "ct", 2, "0", 1, 0);
  coap_add_resource(ctx, r);
#endif /* WITHOUT_ASYNC */
}

static coap_context_t *get_context(const char *node, const char *port) {
  coap_context_t *ctx = NULL;
  int s;
  struct addrinfo hints;
  struct addrinfo *result, *rp;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_DGRAM; /* Coap uses UDP */
  hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;

  s = getaddrinfo(node, port, &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return NULL;
  }

  /* iterate through results until success */
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    coap_address_t addr;

    if (rp->ai_addrlen <= sizeof(addr.addr)) {
      coap_address_init(&addr);
      addr.size = rp->ai_addrlen;
      memcpy(&addr.addr, rp->ai_addr, rp->ai_addrlen);

      ctx = coap_new_context(&addr);
      if (ctx) {
        /* TODO: output address:port for successful binding */
        goto finish;
      }
    }
  }

  fprintf(stderr, "no context available for interface '%s'\n", node);

finish:
  freeaddrinfo(result);
  return ctx;
}

JHANDLER_FUNCTION(init) {
  JHANDLER_CHECK_THIS(object);
  coap_context_t *ctx;
  char addr_str[NI_MAXHOST] = "::";
  char port_str[NI_MAXSERV] = "6666";
  char *group = NULL;
  fd_set readfds;
  struct timeval tv, *timeout;
  int result;
  coap_tick_t now;
  coap_queue_t *nextpdu;
  ctx = get_context(addr_str, port_str);
  int opt;
  coap_log_t log_level = LOG_WARNING;

  clock_offset = time(NULL);

  init_resources(ctx);
  while (!quit) {
    FD_ZERO(&readfds);
    FD_SET(ctx->sockfd, &readfds);

    nextpdu = coap_peek_next(ctx);

    coap_ticks(&now);
    while (nextpdu && nextpdu->t <= now - ctx->sendqueue_basetime) {
      coap_retransmit(ctx, coap_pop_next(ctx));
      nextpdu = coap_peek_next(ctx);
    }

    if (nextpdu && nextpdu->t <= COAP_RESOURCE_CHECK_TIME) {
      /* set timeout if there is a pdu to send before our
       * automatic timeout occurs */
      long long ticks = COAP_TICKS_PER_SECOND;
      tv.tv_usec = ((nextpdu->t) % ticks) * 1000000 / ticks;
      tv.tv_sec = (nextpdu->t) / COAP_TICKS_PER_SECOND;
      timeout = &tv;
    } else {
      tv.tv_usec = 0;
      tv.tv_sec = COAP_RESOURCE_CHECK_TIME;
      timeout = &tv;
    }
    result = select(FD_SETSIZE, &readfds, 0, 0, timeout);

    if (result < 0) { /* error */
      if (errno != EINTR)
        perror("select");
    } else if (result > 0) { /* read from socket */
      if (FD_ISSET(ctx->sockfd, &readfds)) {
        coap_read(ctx); /* read received data */
        /* coap_dispatch( ctx );  /\* and dispatch PDUs from receivequeue *\/ */
      }
    } else { /* timeout */
      if (time_resource) {
        time_resource->dirty = 1;
      }
    }

#ifndef WITHOUT_OBSERVE
    /* check if we have to send observe notifications */
    coap_check_notify(ctx);
#endif /* WITHOUT_OBSERVE */
  }

  coap_free_context(ctx);
}

iotjs_jval_t InitCoap() {
  iotjs_jval_t coap = iotjs_jval_create_object();

  iotjs_jval_set_method(&coap, "init", init);

  return coap;
}
