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

#include "../common.h"

/* Converts OCDevAddr C-structure to JS object.
 *
 * @param req JS object.
 * @param dev_addr OCDevAddr C-structure.
 */
void js_OCDevAddr(const iotjs_jval_t* req, OCDevAddr* dev_addr) {
  iotjs_jval_set_property_number(req, "adapter", dev_addr->adapter);
  iotjs_jval_set_property_number(req, "flags", dev_addr->flags);
  iotjs_jval_set_property_number(req, "port", dev_addr->port);
  iotjs_jval_set_property_string_raw(req, "addr", dev_addr->addr);
  iotjs_jval_set_property_number(req, "ifindex", dev_addr->ifindex);
}

/* Sets OCDevAddr struct with values from JS-side.
 *
 * @param dev_addr devAddr C structure.
 * @param js_dev_addr devAddr object from JS.
 */
void c_OCDevAddr(OCDevAddr* dev_addr, const iotjs_jval_t* js_dev_addr) {
  iotjs_jval_t js_adapter = iotjs_jval_get_property(js_dev_addr, "adapter");
  iotjs_jval_t js_flags = iotjs_jval_get_property(js_dev_addr, "flags");
  iotjs_jval_t js_port = iotjs_jval_get_property(js_dev_addr, "port");
  iotjs_jval_t js_addr = iotjs_jval_get_property(js_dev_addr, "addr");
  iotjs_jval_t js_ifindex = iotjs_jval_get_property(js_dev_addr, "ifindex");

  dev_addr->adapter = iotjs_jval_as_number(&js_adapter);
  dev_addr->flags = iotjs_jval_as_number(&js_flags);
  dev_addr->port = iotjs_jval_as_number(&js_port);
  dev_addr->ifindex = iotjs_jval_as_number(&js_ifindex);
  iotjs_string_t str_addr = iotjs_jval_as_string(&js_addr);
  if (iotjs_string_size(&str_addr) >= MAX_ADDR_STR_SIZE) {
    goto clean;
  } else {
    char* addr = get_string_from_iotjs(&str_addr);
    strcpy(dev_addr->addr, addr);
    free(addr);
  }
clean:
  iotjs_jval_destroy(&js_adapter);
  iotjs_jval_destroy(&js_flags);
  iotjs_jval_destroy(&js_port);
  iotjs_jval_destroy(&js_addr);
  iotjs_jval_destroy(&js_ifindex);
  iotjs_string_destroy(&str_addr);
}
