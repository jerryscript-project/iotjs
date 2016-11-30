/* Copyright 2016 Eunsoo Park (esevan.park@gmail.com)
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

#include "iotjs_module_blecentral.h"
#include "iotjs_module_buffer.h"

static void iotjs_blecentral_destroy(iotjs_bufferwrap_t* bufferwrap) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_bufferwrap_t, bufferwrap);
  iotjs_jobjectwrap_destroy(&_this->jobjectwrap);
  IOTJS_RELEASE(bufferwrap);
}

iotjs_blecentral_t* iotjs_blecentral_create(const iotjs_jval_t* jblecentral) {
  iotjs_blecentral_t* blecentral = IOTJS_ALLOC(iotjs_blecentral_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_blecentral_t, blecentral);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, jblecentral,
                               (JFreeHandlerType)iotjs_blecentral_destroy);

  return blecentral;
}

const iotjs_jval_t* iotjs_blecentral_get_jblecentral() {
  return iotjs_module_get(MODULE_BLECENTRAL);
}

iotjs_blecentral_t* iotjs_blecentral_get_instance() {
  const iotjs_jval_t* jblecentral = iotjs_blecentral_get_jblecentral();
  iotjs_jobjectwrap_t* jobjectwrap =
      iotjs_jobjectwrap_from_jobject(jblecentral);
  return (iotjs_blecentral_t*)jobjectwrap;
}

// serviceUuids, allowDuplicates
JHANDLER_FUNCTION(StartScanning) {
  JHANDLER_CHECK_ARGS(2, string, number);

  iotjs_string_t jsvc_uuid = JHANDLER_GET_ARG(0, string);
  int jallow_duplicates = JHANDLER_GET_ARG(1, number);

  binding_start_scanning(iotjs_string_data(&jsvc_uuid), jallow_duplicates);
}

// void
JHANDLER_FUNCTION(StopScanning) {
  JHANDLER_CHECK_ARGS(0);

  binding_stop_scanning();
}

// peripheralUuid
JHANDLER_FUNCTION(Connect) {
  JHANDLER_CHECK_ARGS(1, string);

  iotjs_string_t jperip_uuid = JHANDLER_GET_ARG(0, string);

  binding_connect(iotjs_string_data(&jperip_uuid));
}

// peripheralUuid
JHANDLER_FUNCTION(Disconnect) {
  JHANDLER_CHECK_ARGS(1, string);

  iotjs_string_t jperip_uuid = JHANDLER_GET_ARG(0, string);

  binding_disconnect(iotjs_string_data(&jperip_uuid));
}

// peripheraluuid, uuids
JHANDLER_FUNCTION(DiscoverServices) {
  JHANDLER_CHECK_ARGS(2, string, array);

  iotjs_string_t jperip_uuid = JHANDLER_GET_ARG(0, string);
  const iotjs_jval_t* jarray = JHANDLER_GET_ARG(1, array);

  /*TODO: Need to implement service uuids array binding
   * Now, all services are going to be discovered
   */
  binding_discover_service(iotjs_string_data(&jperip_uuid));
}

// peripheralUuid, handle
JHANDLER_FUNCTION(ReadHandle) {
  JHANDLER_CHECK_ARGS(2, string, string);

  iotjs_string_t jperip_uuid = JHANDLER_GET_ARG(0, string);
  iotjs_string_t jhandle = JHANDLER_GET_ARG(1, string);

  binding_read_handle(iotjs_string_data(&jperip_uuid),
                      iotjs_string_data(&jhandle));
}

// peripheralUuid, handle, data, withoutResponse
JHANDLER_FUNCTION(WriteHandle) {
  JHANDLER_CHECK_ARGS(4, string, string, object, number);

  iotjs_string_t jperip_uuid = JHANDLER_GET_ARG(0, string);
  iotjs_string_t jhandle = JHANDLER_GET_ARG(1, string);

  const iotjs_jval_t* jbuffer = JHANDLER_GET_ARG(2, object);
  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);
  char* buffer = iotjs_bufferwrap_buffer(buffer_wrap);
  int len = iotjs_bufferwrap_length(buffer_wrap);

  int without_response = JHANDLER_GET_ARG(3, number);

  binding_write_handle(iotjs_string_data(&jperip_uuid),
                       iotjs_string_data(&jhandle), buffer, len,
                       without_response);
}

// peripheralUuid, serviceUuid, serviceUuids
JHANDLER_FUNCTION(DiscoverIncludedServices) {
  JHANDLER_CHECK_ARGS(3, string, string, array);

  iotjs_string_t jperip_uuid = JHANDLER_GET_ARG(0, string);
  iotjs_string_t jsvc_uuid = JHANDLER_GET_ARG(1, string);
  const iotjs_jval_t* jarray = JHANDLER_GET_ARG(2, array);

  /* TODO: Need to implement serviceUuids array binding
   *  Now, all included services are found
   */
  binding_discover_included_services(iotjs_string_data(&jperip_uuid),
                                     iotjs_string_data(&jsvc_uuid));
}

// peripheralUuid, servicdUuid, characteristicUuids
JHANDLER_FUNCTION(DiscoverCharacteristics) {
  JHANDLER_CHECK_ARGS(3, string, string, array);

  iotjs_string_t jperip_uuid = JHANDLER_GET_ARG(0, string);
  iotjs_string_t jsvc_uuid = JHANDLER_GET_ARG(1, string);
  const iotjs_jval_t* jarray = JHANDLER_GET_ARG(2, array);

  binding_discover_characteristics(iotjs_string_data(&jperip_uuid),
                                   iotjs_string_data(&jsvc_uuid));
}

// peripheralUuid, serviceUuid, characteristicUuid
JHANDLER_FUNCTION(Read) {
  JHANDLER_CHECK_ARGS(3, string, string, string);

  iotjs_string_t jperip_uuid = JHANDLER_GET_ARG(0, string);
  iotjs_string_t jsvc_uuid = JHANDLER_GET_ARG(1, string);
  iotjs_string_t jchar_uuid = JHANDLER_GET_ARG(2, string);

  binding_read(iotjs_string_data(&jperip_uuid), iotjs_string_data(&jsvc_uuid),
               iotjs_string_data(&jchar_uuid));
}

// peripheralUuid, servicdUuid, characteristicUuid, data, withoutResponse
JHANDLER_FUNCTION(Write) {
  JHANDLER_CHECK_ARGS(5, string, string, string, object, number);

  iotjs_string_t jperip_uuid = JHANDLER_GET_ARG(0, string);
  iotjs_string_t jsvc_uuid = JHANDLER_GET_ARG(1, string);
  iotjs_string_t jchar_uuid = JHANDLER_GET_ARG(2, string);

  const iotjs_jval_t* jbuffer = JHANDLER_GET_ARG(3, object);
  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);
  char* buffer = iotjs_bufferwrap_buffer(buffer_wrap);
  int len = iotjs_bufferwrap_length(buffer_wrap);

  int without_response = JHANDLER_GET_ARG(4, number);

  binding_write(iotjs_string_data(&jperip_uuid), iotjs_string_data(&jsvc_uuid),
                iotjs_string_data(&jchar_uuid), buffer, len, without_response);
}

// peripheralUuid, servicdUuid, characteristicUuid, broadcast
JHANDLER_FUNCTION(Broadcast) {
  JHANDLER_CHECK_ARGS(4, string, string, string, number);

  iotjs_string_t jperip_uuid = JHANDLER_GET_ARG(0, string);
  iotjs_string_t jsvc_uuid = JHANDLER_GET_ARG(1, string);
  iotjs_string_t jchar_uuid = JHANDLER_GET_ARG(2, string);
  int broadcast = JHANDLER_GET_ARG(3, number);

  binding_broadcast(iotjs_string_data(&jperip_uuid),
                    iotjs_string_data(&jsvc_uuid),
                    iotjs_string_data(&jchar_uuid), broadcast);
}

// peripheralUuid, serviceUuid, characteristicUuid, notify
JHANDLER_FUNCTION(Notify) {
  JHANDLER_CHECK_ARGS(4, string, string, string, number);

  iotjs_string_t jperip_uuid = JHANDLER_GET_ARG(0, string);
  iotjs_string_t jsvc_uuid = JHANDLER_GET_ARG(1, string);
  iotjs_string_t jchar_uuid = JHANDLER_GET_ARG(2, string);
  int notify = JHANDLER_GET_ARG(3, number);

  binding_notify(iotjs_string_data(&jperip_uuid), iotjs_string_data(&jsvc_uuid),
                 iotjs_string_data(&jchar_uuid), notify);
}

// peripheralUuid, serviceUuid, characteristciUuid
JHANDLER_FUNCTION(DiscoverDescriptors) {
  JHANDLER_CHECK_ARGS(3, string, string, string);

  iotjs_string_t jperip_uuid = JHANDLER_GET_ARG(0, string);
  iotjs_string_t jsvc_uuid = JHANDLER_GET_ARG(1, string);
  iotjs_string_t jchar_uuid = JHANDLER_GET_ARG(2, string);

  binding_discover_descriptors(iotjs_string_data(&jperip_uuid),
                               iotjs_string_data(&jsvc_uuid),
                               iotjs_string_data(&jchar_uuid));
}

// peripheralUuid, serviceUuid, characteristicUuid, descriptorUuid
JHANDLER_FUNCTION(ReadValue) {
  JHANDLER_CHECK_ARGS(4, string, string, string, string);

  iotjs_string_t jperip_uuid = JHANDLER_GET_ARG(0, string);
  iotjs_string_t jsvc_uuid = JHANDLER_GET_ARG(1, string);
  iotjs_string_t jchar_uuid = JHANDLER_GET_ARG(2, string);
  iotjs_string_t jdesc_uuid = JHANDLER_GET_ARG(3, string);

  binding_read_value(iotjs_string_data(&jperip_uuid),
                     iotjs_string_data(&jsvc_uuid),
                     iotjs_string_data(&jchar_uuid),
                     iotjs_string_data(&jdesc_uuid));
}

// peripheralUuid, serviceUuid, characteristicUuid, descriptorUuid, data
JHANDLER_FUNCTION(WriteValue) {
  JHANDLER_CHECK_ARGS(5, string, string, string, string, object);

  iotjs_string_t jperip_uuid = JHANDLER_GET_ARG(0, string);
  iotjs_string_t jsvc_uuid = JHANDLER_GET_ARG(1, string);
  iotjs_string_t jchar_uuid = JHANDLER_GET_ARG(2, string);
  iotjs_string_t jdesc_uuid = JHANDLER_GET_ARG(3, string);

  const iotjs_jval_t* jbuffer = JHANDLER_GET_ARG(4, object);
  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);
  char* buffer = iotjs_bufferwrap_buffer(buffer_wrap);
  int len = iotjs_bufferwrap_length(buffer_wrap);

  binding_write_value(iotjs_string_data(&jperip_uuid),
                      iotjs_string_data(&jsvc_uuid),
                      iotjs_string_data(&jchar_uuid),
                      iotjs_string_data(&jdesc_uuid), buffer, len);
}

JHANDLER_FUNCTION(Listen) {
  JHANDLER_CHECK_ARGS(1, function);

  // TODO: reqwrap
  binding_listen();
}

iotjs_jval_t InitBlecentral() {
  iotjs_jval_t jblecentral = iotjs_jval_create_object();

  // Ble-central
  iotjs_jval_set_method(&jblecentral, "startScanning", StartScanning);
  iotjs_jval_set_method(&jblecentral, "stopScanning", StopScanning);

  // Peripheral
  iotjs_jval_set_method(&jblecentral, "connect", Connect);
  iotjs_jval_set_method(&jblecentral, "disconnect", Disconnect);
  iotjs_jval_set_method(&jblecentral, "discoverServices", DiscoverServices);
  iotjs_jval_set_method(&jblecentral, "readHandle", ReadHandle);
  iotjs_jval_set_method(&jblecentral, "writeHandle", WriteHandle);

  // Service
  iotjs_jval_set_method(&jblecentral, "discoverIncludedServices",
                        DiscoverIncludedServices);
  iotjs_jval_set_method(&jblecentral, "discoverCharacteristics",
                        DiscoverCharacteristics);

  // Characteristic
  iotjs_jval_set_method(&jblecentral, "read", Read);
  iotjs_jval_set_method(&jblecentral, "write", Write);
  iotjs_jval_set_method(&jblecentral, "broadcast", Broadcast);
  iotjs_jval_set_method(&jblecentral, "notify", Notify);
  iotjs_jval_set_method(&jblecentral, "discoverDescriptors",
                        DiscoverDescriptors);

  // Descriptor
  iotjs_jval_set_method(&jblecentral, "readValue", ReadValue);
  iotjs_jval_set_method(&jblecentral, "writeValue", WriteValue);

  // EventListener
  iotjs_jval_set_method(&jblecentral, "listen", Listen);

  iotjs_blecentral_t* blecentral = iotjs_blecentral_create(&jblecentral);

  return jblecentral;
}
