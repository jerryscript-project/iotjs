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

#include <stdlib.h>
#include <string.h>

#include "iotjs_def.h"
#include "iotjs_module_buffer.h"
#include "iotjs_module_mqtt.h"


#include "iotjs_handlewrap.h"
#include "iotjs_reqwrap.h"


static jerry_value_t mqtt_client_connack_error(const unsigned char error_code) {
  switch (error_code) {
    case UNACCEPTABLE_PROTOCOL:
      return JS_CREATE_ERROR(COMMON,
                             "MQTT: Connection refused: unacceptable protocol");
    case BAD_IDENTIFIER:
      return JS_CREATE_ERROR(COMMON,
                             "MQTT: Connection refused: bad client identifier");
    case SERVER_UNAVIABLE:
      return JS_CREATE_ERROR(COMMON,
                             "MQTT: Connection refused: server unaviable");
    case BAD_CREDENTIALS:
      return JS_CREATE_ERROR(
          COMMON, "MQTT: Connection refused: bad username or password");
    case UNAUTHORISED:
      return JS_CREATE_ERROR(COMMON, "MQTT: Connection refused: unauthorised");
    default:
      return JS_CREATE_ERROR(COMMON, "MQTT: Unknown error");
  }
}


static uint8_t *iotjs_encode_remaining_length(unsigned char *buffer,
                                              uint32_t len) {
  size_t rc = 0;
  do {
    unsigned char d = len & 0x7F;
    len >>= 7;
    if (len > 0) {
      d |= 0x80;
    }
    buffer[rc++] = d;
  } while (len > 0);

  return (buffer + rc);
}


static size_t get_remaining_length_size(uint32_t len) {
  uint8_t n = 0;
  while (len != 0) {
    len >>= 7;
    n++;
  }

  return n;
}


static uint32_t iotjs_decode_remaining_length(char *buffer, size_t *offset) {
  unsigned char c;
  uint32_t remaining_length = 0;
  uint32_t length = 0;
  uint32_t shift = 0;

  do {
    if (++length > IOTJS_MODULE_MQTT_MAX_REMAINING_LENGTH_BYTES) {
      return UINT32_MAX;
    }
    c = (unsigned char)buffer[*offset];
    remaining_length += (uint32_t)(c & 0x7F) << shift;
    shift += 7;

    (*offset)++;
  } while ((c & 128) != 0);

  return remaining_length;
}


static uint16_t iotjs_mqtt_calculate_length(uint8_t msb, uint8_t lsb) {
  return (msb << 8) | lsb;
}


static uint8_t *iotjs_mqtt_string_serialize(uint8_t *dst_buffer,
                                            iotjs_bufferwrap_t *src_buffer) {
  uint16_t len = src_buffer->length;
  dst_buffer[0] = (uint8_t)(len >> 8);
  dst_buffer[1] = (uint8_t)(len & 0x00FF);
  memcpy(dst_buffer + 2, src_buffer->buffer, src_buffer->length);
  return (dst_buffer + 2 + src_buffer->length);
}

void iotjs_create_ack_callback(char *buffer, char *name, jerry_value_t jsref) {
  uint8_t packet_identifier_MSB = (uint8_t)buffer[2];
  uint8_t packet_identifier_LSB = (uint8_t)buffer[3];

  uint16_t package_id =
      iotjs_mqtt_calculate_length(packet_identifier_MSB, packet_identifier_LSB);

  // The callback takes the packet identifier as parameter.
  iotjs_jargs_t args = iotjs_jargs_create(2);
  iotjs_jargs_append_jval(&args, jsref);
  iotjs_jargs_append_number(&args, package_id);

  jerry_value_t fn = iotjs_jval_get_property(jsref, name);
  iotjs_make_callback(fn, jsref, &args);
  jerry_release_value(fn);
  iotjs_jargs_destroy(&args);
}


JS_FUNCTION(MqttConnect) {
  DJS_CHECK_THIS();

  DJS_CHECK_ARGS(1, object);

  jerry_value_t joptions = JS_GET_ARG(0, object);

  jerry_value_t jclient_id =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_CLIENTID);
  jerry_value_t jusername =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_USERNAME);
  jerry_value_t jpassword =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_PASSWORD);
  jerry_value_t jkeepalive =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_KEEPALIVE);
  jerry_value_t jwill =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_WILL);
  jerry_value_t jqos =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_QOS);
  jerry_value_t jmessage =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_MESSAGE);
  jerry_value_t jtopic =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_TOPIC);
  jerry_value_t jretain =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_TOPIC);

  uint8_t connect_flags = 0;
  uint8_t keep_alive_msb = 0;
  uint8_t keep_alive_lsb = 10;
  connect_flags |= MQTT_FLAG_CLEANSESSION;
  iotjs_bufferwrap_t *username = NULL;
  iotjs_bufferwrap_t *password = NULL;
  iotjs_bufferwrap_t *message = NULL;
  iotjs_bufferwrap_t *topic = NULL;

  uint8_t header_byte = 0;
  header_byte |= (CONNECT << 4);


  if (!jerry_value_is_undefined(jwill) && jerry_get_boolean_value(jwill)) {
    connect_flags |= MQTT_FLAG_WILL;
    if (!jerry_value_is_undefined(jqos)) {
      uint8_t qos = 0;
      qos = jerry_get_number_value(qos);
      if (qos) {
        connect_flags |= (qos == 1) ? MQTT_FLAG_WILLQOS_1 : MQTT_FLAG_WILLQOS_2;
      }
    }

    if (!jerry_value_is_undefined(jretain) &&
        jerry_get_boolean_value(jretain)) {
      connect_flags |= MQTT_FLAG_WILLRETAIN;
    }
    message = iotjs_bufferwrap_from_jbuffer(jmessage);
    topic = iotjs_bufferwrap_from_jbuffer(jtopic);
  }

  if (!jerry_value_is_undefined(jusername)) {
    connect_flags |= MQTT_FLAG_USERNAME;
    username = iotjs_bufferwrap_from_jbuffer(jusername);
  }
  if (!jerry_value_is_undefined(jpassword)) {
    connect_flags |= MQTT_FLAG_PASSWORD;
    password = iotjs_bufferwrap_from_jbuffer(jpassword);
  }
  if (!jerry_value_is_undefined(jkeepalive)) {
    uint16_t len = jerry_get_number_value(jkeepalive);
    keep_alive_msb = (uint8_t)(len >> 8);
    keep_alive_lsb = (uint8_t)(len & 0x00FF);
  }


  iotjs_bufferwrap_t *client_id = iotjs_bufferwrap_from_jbuffer(jclient_id);

  unsigned char variable_header_protocol[7];
  variable_header_protocol[0] = 0;
  variable_header_protocol[1] = 4;
  variable_header_protocol[2] = 'M';
  variable_header_protocol[3] = 'Q';
  variable_header_protocol[4] = 'T';
  variable_header_protocol[5] = 'T';
  variable_header_protocol[6] = 4;

  size_t variable_header_len = sizeof(variable_header_protocol) +
                               sizeof(connect_flags) + sizeof(keep_alive_lsb) +
                               sizeof(keep_alive_msb);

  size_t payload_len = client_id->length + IOTJS_MQTT_LSB_MSB_SIZE;
  if (connect_flags & MQTT_FLAG_USERNAME) {
    payload_len += IOTJS_MQTT_LSB_MSB_SIZE + username->length;
  }
  if (connect_flags & MQTT_FLAG_PASSWORD) {
    payload_len += IOTJS_MQTT_LSB_MSB_SIZE + password->length;
  }
  if (connect_flags & MQTT_FLAG_WILL) {
    payload_len += IOTJS_MQTT_LSB_MSB_SIZE + topic->length;
    payload_len += IOTJS_MQTT_LSB_MSB_SIZE + message->length;
  }
  uint32_t remaining_length = payload_len + variable_header_len;
  size_t full_len = sizeof(header_byte) +
                    get_remaining_length_size(remaining_length) +
                    variable_header_len + payload_len;

  jerry_value_t jbuff = iotjs_bufferwrap_create_buffer(full_len);
  iotjs_bufferwrap_t *buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuff);

  uint8_t *buff_ptr = (uint8_t *)buffer_wrap->buffer;

  *buff_ptr++ = header_byte;
  buff_ptr = iotjs_encode_remaining_length(buff_ptr, remaining_length);

  memcpy(buff_ptr, variable_header_protocol, sizeof(variable_header_protocol));
  buff_ptr += sizeof(variable_header_protocol);
  *buff_ptr++ = connect_flags;
  *buff_ptr++ = keep_alive_msb;
  *buff_ptr++ = keep_alive_lsb;

  buff_ptr = iotjs_mqtt_string_serialize(buff_ptr, client_id);

  if (connect_flags & MQTT_FLAG_WILL) {
    buff_ptr = iotjs_mqtt_string_serialize(buff_ptr, topic);
    buff_ptr = iotjs_mqtt_string_serialize(buff_ptr, message);
  }

  if (connect_flags & MQTT_FLAG_USERNAME) {
    buff_ptr = iotjs_mqtt_string_serialize(buff_ptr, username);
  }
  if (connect_flags & MQTT_FLAG_PASSWORD) {
    buff_ptr = iotjs_mqtt_string_serialize(buff_ptr, password);
  }

  jerry_release_value(jretain);
  jerry_release_value(jtopic);
  jerry_release_value(jmessage);
  jerry_release_value(jqos);
  jerry_release_value(jwill);
  jerry_release_value(jkeepalive);
  jerry_release_value(jclient_id);
  jerry_release_value(jusername);
  jerry_release_value(jpassword);

  return jbuff;
}


JS_FUNCTION(MqttPublish) {
  DJS_CHECK_THIS();

  DJS_CHECK_ARGS(1, object);

  jerry_value_t joptions = JS_GET_ARG(0, object);

  jerry_value_t jmessage =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_MESSAGE);
  jerry_value_t jtopic =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_TOPIC);
  jerry_value_t jretain =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_RETAIN);
  jerry_value_t jqos =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_QOS);
  jerry_value_t jpacket_id =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_PACKETID);

  uint8_t qos = 0;
  if (jerry_value_is_number(jqos)) {
    qos = jerry_get_number_value(jqos);
  }

  bool dup = false;

  uint8_t header_byte = 0;
  header_byte |= (PUBLISH << 4);
  header_byte |= (dup << 3);
  header_byte |= (qos << 1);
  header_byte |= (jerry_get_boolean_value(jretain));

  iotjs_bufferwrap_t *message_payload = iotjs_bufferwrap_from_jbuffer(jmessage);
  iotjs_bufferwrap_t *topic_payload = iotjs_bufferwrap_from_jbuffer(jtopic);

  uint8_t packet_identifier_lsb = 0;
  uint8_t packet_identifier_msb = 0;

  if (qos > 0 && jerry_value_is_number(jpacket_id)) {
    uint16_t packet_identifier = jerry_get_number_value(jpacket_id);
    packet_identifier_msb = (uint8_t)(packet_identifier >> 8);
    packet_identifier_lsb = (uint8_t)(packet_identifier & 0x00FF);
  }

  size_t payload_len = message_payload->length + IOTJS_MQTT_LSB_MSB_SIZE;
  size_t variable_header_len = topic_payload->length + IOTJS_MQTT_LSB_MSB_SIZE +
                               sizeof(packet_identifier_msb) +
                               sizeof(packet_identifier_lsb);
  uint32_t remaining_length = payload_len + variable_header_len;
  size_t full_len = sizeof(header_byte) +
                    get_remaining_length_size(remaining_length) +
                    variable_header_len + payload_len;

  jerry_value_t jbuff = iotjs_bufferwrap_create_buffer(full_len);
  iotjs_bufferwrap_t *buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuff);

  uint8_t *buff_ptr = (uint8_t *)buffer_wrap->buffer;

  *buff_ptr++ = header_byte;
  buff_ptr = iotjs_encode_remaining_length(buff_ptr, remaining_length);
  buff_ptr = iotjs_mqtt_string_serialize(buff_ptr, topic_payload);
  *buff_ptr++ = packet_identifier_msb;
  *buff_ptr++ = packet_identifier_lsb;

  // Don't need to put length before the payload, so we can't use the
  // iotjs_mqtt_string_serialize. The broker and the other clients calculate
  // the payload length from remaining length and the topic length.
  memcpy(buff_ptr, message_payload->buffer, message_payload->length);

  jerry_release_value(jmessage);
  jerry_release_value(jtopic);
  jerry_release_value(jretain);

  return jbuff;
}


JS_FUNCTION(MqttHandle) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(2, object, object);

  jerry_value_t jsref = JS_GET_ARG(0, object);
  jerry_value_t jparam = JS_GET_ARG(1, object);

  iotjs_bufferwrap_t *buffer_wrap = iotjs_bufferwrap_from_jbuffer(jparam);
  char *buffer = buffer_wrap->buffer;

  char first_byte = buffer[0];
  char packet_type = (first_byte >> 4) & 0x0F;

  size_t offset = 1;
  uint32_t remaining_length = iotjs_decode_remaining_length(buffer, &offset);

  switch (packet_type) {
    case CONNACK: {
      if (remaining_length != 2) {
        return JS_CREATE_ERROR(COMMON, "MQTT: CONNACK packet is corrupted");
      }

      uint8_t return_code = (uint8_t)buffer[++offset];

      if (return_code != 0) {
        return mqtt_client_connack_error(return_code);
      }

      jerry_value_t fn =
          iotjs_jval_get_property(jsref, IOTJS_MAGIC_STRING__ONCONNECT);
      iotjs_jargs_t jargs = iotjs_jargs_create(1);
      iotjs_jargs_append_jval(&jargs, jsref);
      iotjs_make_callback(fn, jsref, &jargs);

      iotjs_jargs_destroy(&jargs);
      jerry_release_value(fn);
      break;
    }
    case PUBLISH: {
      MQTTHeader header = { 0 };
      header.bits.type = PUBLISH;
      header.bits.dup = first_byte & 0x08;
      header.bits.qos = (first_byte & 0x06) >> 1;
      header.bits.retain = first_byte & 0x01;

      uint8_t topic_length_MSB = (uint8_t)buffer[offset];
      offset += sizeof(topic_length_MSB);

      uint8_t topic_length_LSB = (uint8_t)buffer[offset];
      offset += sizeof(topic_length_LSB);

      uint16_t topic_length =
          iotjs_mqtt_calculate_length(topic_length_MSB, topic_length_LSB);

      jerry_value_t jtopic = iotjs_bufferwrap_create_buffer(topic_length);
      iotjs_bufferwrap_t *topic_wrap = iotjs_bufferwrap_from_jbuffer(jtopic);

      memcpy(topic_wrap->buffer, buffer + offset, topic_length);
      offset += topic_length;

      // The Packet Identifier field is only present in PUBLISH packets
      // where the QoS level is 1 or 2.
      uint16_t packet_identifier = 0;
      if (header.bits.qos > 0) {
        uint8_t packet_identifier_MSB = 0;
        uint8_t packet_identifier_LSB = 0;


        memcpy(&packet_identifier_MSB, buffer + offset, sizeof(uint8_t));
        offset += sizeof(packet_identifier_MSB);

        memcpy(&packet_identifier_LSB, buffer + offset, sizeof(uint8_t));
        offset += sizeof(packet_identifier_LSB);

        packet_identifier = iotjs_mqtt_calculate_length(packet_identifier_MSB,
                                                        packet_identifier_LSB);
      }

      size_t payload_length =
          (size_t)remaining_length - topic_length - sizeof(topic_length);

      if (header.bits.qos > 0) {
        payload_length -= sizeof(packet_identifier);
      }

      jerry_value_t jmessage = iotjs_bufferwrap_create_buffer(payload_length);
      iotjs_bufferwrap_t *msg_wrap = iotjs_bufferwrap_from_jbuffer(jmessage);

      IOTJS_ASSERT(jerry_is_valid_utf8_string((const uint8_t *)msg_wrap->buffer,
                                              msg_wrap->length));

      IOTJS_ASSERT(
          jerry_is_valid_utf8_string((const uint8_t *)topic_wrap->buffer,
                                     topic_wrap->length));

      memcpy(msg_wrap->buffer, buffer + offset, payload_length);
      offset += payload_length;

      iotjs_jargs_t args = iotjs_jargs_create(5);
      iotjs_jargs_append_jval(&args, jsref);
      iotjs_jargs_append_string_raw(&args, msg_wrap->buffer);
      iotjs_jargs_append_string_raw(&args, topic_wrap->buffer);
      iotjs_jargs_append_number(&args, header.bits.qos);
      iotjs_jargs_append_number(&args, packet_identifier);

      jerry_value_t fn =
          iotjs_jval_get_property(jsref, IOTJS_MAGIC_STRING__ONMESSAGE);
      iotjs_make_callback(fn, jsref, &args);
      jerry_release_value(fn);

      iotjs_jargs_destroy(&args);
      jerry_release_value(jmessage);
      jerry_release_value(jtopic);

      break;
    }
    case PUBACK: {
      if (remaining_length != 2) {
        return JS_CREATE_ERROR(COMMON, "MQTT: PUBACK packet is corrupted");
      }

      iotjs_create_ack_callback(buffer, IOTJS_MAGIC_STRING__ONPUBACK, jsref);
      break;
    }
    case PUBREC: {
      if (remaining_length != 2) {
        return JS_CREATE_ERROR(COMMON, "MQTT: RUBREC packet is corrupted");
      }

      iotjs_create_ack_callback(buffer, IOTJS_MAGIC_STRING__ONPUBREC, jsref);
      break;
    }
    case PUBREL: {
      if (remaining_length != 2) {
        return JS_CREATE_ERROR(COMMON, "MQTT: PUBREL packet is corrupted");
      }

      char control_packet_reserved = first_byte & 0x0F;
      if (control_packet_reserved != 2) {
        return JS_CREATE_ERROR(COMMON, "MQTT: PUBREL packet is corrupted");
      }

      iotjs_create_ack_callback(buffer, IOTJS_MAGIC_STRING__ONPUBREL, jsref);
      break;
    }
    case PUBCOMP: {
      if (remaining_length != 2) {
        return JS_CREATE_ERROR(COMMON, "MQTT: PUBCOMP packet is corrupted");
      }

      iotjs_create_ack_callback(buffer, IOTJS_MAGIC_STRING__ONPUBCOMP, jsref);
      break;
    }
    case SUBACK: {
      // We assume that only one topic was in the SUBSCRIBE packet.
      if (remaining_length != 3) {
        return JS_CREATE_ERROR(COMMON, "MQTT: SUBACK packet is corrupted");
      }

      uint8_t return_code = (uint8_t)buffer[4];
      if (return_code == 128) {
        return JS_CREATE_ERROR(COMMON, "MQTT: Subscription was unsuccessful");
      }

      // The callback takes the granted QoS as parameter.
      iotjs_jargs_t args = iotjs_jargs_create(2);
      iotjs_jargs_append_jval(&args, jsref);
      iotjs_jargs_append_number(&args, return_code);

      jerry_value_t sub_fn =
          iotjs_jval_get_property(jsref, IOTJS_MAGIC_STRING__ONSUBACK);
      iotjs_make_callback(sub_fn, jsref, &args);
      jerry_release_value(sub_fn);
      iotjs_jargs_destroy(&args);
      break;
    }
    case UNSUBACK: {
      if (remaining_length != 2) {
        return JS_CREATE_ERROR(COMMON, "MQTT: UNSUBACK packet is corrupted");
      }

      iotjs_create_ack_callback(buffer, IOTJS_MAGIC_STRING__ONUNSUBACK, jsref);
      break;
    }
    case PINGRESP: {
      if (remaining_length != 0) {
        return JS_CREATE_ERROR(COMMON, "MQTT: PingRESP packet is corrupted");
      }
      jerry_value_t fn =
          iotjs_jval_get_property(jsref, IOTJS_MAGIC_STRING__ONPINGRESP);
      iotjs_jargs_t jargs = iotjs_jargs_create(1);
      iotjs_jargs_append_jval(&jargs, jsref);
      iotjs_make_callback(fn, jsref, &jargs);
      jerry_release_value(fn);
      iotjs_jargs_destroy(&jargs);
      break;
    }
    case DISCONNECT: {
      iotjs_jargs_t jargs = iotjs_jargs_create(2);
      iotjs_jargs_append_jval(&jargs, jsref);
      jerry_value_t str_arg = jerry_create_string(
          (jerry_char_t *)"The broker disconnected the client");
      iotjs_jargs_append_jval(&jargs, str_arg);
      jerry_value_t fn =
          iotjs_jval_get_property(jsref, IOTJS_MAGIC_STRING__ONDISCONNECT);
      iotjs_make_callback(fn, jsref, &jargs);
      jerry_release_value(str_arg);
      jerry_release_value(fn);
      iotjs_jargs_destroy(&jargs);
      break;
    }

    case CONNECT:
    case SUBSCRIBE:
    case UNSUBSCRIBE:
    case PINGREQ:
      return JS_CREATE_ERROR(
          COMMON, "MQTT: Unallowed packet has arrived to the client");
  }

  return jerry_create_undefined();
}


JS_FUNCTION(MqttSubscribe) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, object);

  jerry_value_t joptions = JS_GET_ARG(0, object);

  jerry_value_t jtopic =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_TOPIC);
  jerry_value_t jqos =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_QOS);

  uint8_t qos = 0;
  if (jerry_value_is_number(jqos)) {
    qos = (uint8_t)jerry_get_number_value(jqos);
  }

  bool dup = false;
  bool retain = false;

  uint8_t header_byte = 0;
  header_byte |= (SUBSCRIBE << 4);
  header_byte |= (dup << 3);
  header_byte |= (1 << 1); // always 1
  header_byte |= retain;

  iotjs_bufferwrap_t *topic_payload = iotjs_bufferwrap_from_jbuffer(jtopic);

  uint8_t packet_identifier_lsb = 0;
  uint8_t packet_identifier_msb = 0;

  size_t payload_len =
      sizeof(qos) + topic_payload->length + IOTJS_MQTT_LSB_MSB_SIZE;
  size_t variable_header_len =
      sizeof(packet_identifier_msb) + sizeof(packet_identifier_lsb);
  uint32_t remaining_length = payload_len + variable_header_len;
  size_t full_len = sizeof(header_byte) +
                    get_remaining_length_size(remaining_length) +
                    variable_header_len + payload_len;

  jerry_value_t jbuff = iotjs_bufferwrap_create_buffer(full_len);
  iotjs_bufferwrap_t *buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuff);

  uint8_t *buff_ptr = (uint8_t *)buffer_wrap->buffer;

  *buff_ptr++ = header_byte;

  buff_ptr = iotjs_encode_remaining_length(buff_ptr, remaining_length);

  *buff_ptr++ = packet_identifier_msb;
  *buff_ptr++ = packet_identifier_lsb;

  buff_ptr = iotjs_mqtt_string_serialize(buff_ptr, topic_payload);

  buff_ptr[0] = qos;

  jerry_release_value(jtopic);
  jerry_release_value(jqos);

  return jbuff;
}


JS_FUNCTION(MqttPing) {
  DJS_CHECK_THIS();

  uint8_t header_byte = 0;
  header_byte |= (PINGREQ << 4);

  uint8_t remaining_length = 0;
  size_t full_len = sizeof(header_byte) + sizeof(remaining_length);

  jerry_value_t jbuff = iotjs_bufferwrap_create_buffer(full_len);
  iotjs_bufferwrap_t *buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuff);

  uint8_t *buff_ptr = (uint8_t *)buffer_wrap->buffer;

  buff_ptr[0] = header_byte;
  buff_ptr[1] = remaining_length;

  return jbuff;
}


JS_FUNCTION(MqttUnsubscribe) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, object);

  jerry_value_t jtopic = JS_GET_ARG(0, object);

  iotjs_bufferwrap_t *topic_payload = iotjs_bufferwrap_from_jbuffer(jtopic);

  uint8_t header_byte = 0;
  header_byte |= (UNSUBSCRIBE << 4);
  // Reserved
  header_byte |= (1 << 1);

  uint8_t packet_identifier_msb = 0;
  uint8_t packet_identifier_lsb = 0;

  size_t payload_len = topic_payload->length + IOTJS_MQTT_LSB_MSB_SIZE;
  size_t variable_header_len =
      sizeof(packet_identifier_msb) + sizeof(packet_identifier_lsb);
  uint32_t remaining_length = payload_len + variable_header_len;
  size_t full_len = sizeof(header_byte) +
                    get_remaining_length_size(remaining_length) +
                    variable_header_len + payload_len;

  jerry_value_t jbuff = iotjs_bufferwrap_create_buffer(full_len);
  iotjs_bufferwrap_t *buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuff);

  uint8_t *buff_ptr = (uint8_t *)buffer_wrap->buffer;

  *buff_ptr++ = header_byte;

  buff_ptr = iotjs_encode_remaining_length(buff_ptr, remaining_length);

  *buff_ptr++ = packet_identifier_msb;
  *buff_ptr++ = packet_identifier_lsb;

  buff_ptr = iotjs_mqtt_string_serialize(buff_ptr, topic_payload);

  return jbuff;
}


JS_FUNCTION(MqttDisconnect) {
  DJS_CHECK_THIS();

  uint8_t header_byte = 0;
  uint8_t remaining_length = 0;
  header_byte |= (DISCONNECT << 4);

  size_t full_len = sizeof(header_byte) + sizeof(remaining_length);

  jerry_value_t jbuff = iotjs_bufferwrap_create_buffer(full_len);
  iotjs_bufferwrap_t *buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuff);

  uint8_t *buff_ptr = (uint8_t *)buffer_wrap->buffer;

  buff_ptr[0] = header_byte;
  buff_ptr[1] = remaining_length;

  return jbuff;
}

JS_FUNCTION(MqttSendAck) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(1, object);

  jerry_value_t joptions = JS_GET_ARG(0, object);
  jerry_value_t jack_type =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_ACKTYPE);
  jerry_value_t jpacket_id =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_PACKETID);

  uint16_t packet_id = 0;
  if (!jerry_value_is_undefined(jpacket_id)) {
    packet_id = (uint16_t)jerry_get_number_value(jpacket_id);
  }

  uint8_t ack_type = 0;
  if (!jerry_value_is_undefined(jack_type)) {
    ack_type = (uint8_t)jerry_get_number_value(jack_type);
  }

  uint8_t header_byte = 0;
  uint8_t remaining_length = 2;

  switch (ack_type) {
    case PUBACK: {
      header_byte |= (PUBACK << 4);
      break;
    }
    case PUBREC: {
      header_byte |= (PUBREC << 4);
      break;
    }
    case PUBREL: {
      header_byte |= (PUBREL << 4);
      header_byte |= 2;
      break;
    }
    case PUBCOMP: {
      header_byte |= (PUBCOMP << 4);
      break;
    }
  }

  uint8_t packet_identifier_msb = (uint8_t)(packet_id >> 8);
  uint8_t packet_identifier_lsb = (uint8_t)(packet_id & 0x00FF);

  size_t full_len =
      sizeof(header_byte) + sizeof(remaining_length) + sizeof(packet_id);

  jerry_value_t jbuff = iotjs_bufferwrap_create_buffer(full_len);
  iotjs_bufferwrap_t *buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuff);

  uint8_t *buff_ptr = (uint8_t *)buffer_wrap->buffer;

  buff_ptr[0] = header_byte;
  buff_ptr[1] = remaining_length;
  buff_ptr[2] = packet_identifier_msb;
  buff_ptr[3] = packet_identifier_lsb;

  jerry_release_value(jpacket_id);
  jerry_release_value(jack_type);

  return jbuff;
}

jerry_value_t InitMQTT() {
  jerry_value_t jMQTT = jerry_create_object();
  iotjs_jval_set_method(jMQTT, IOTJS_MAGIC_STRING_CONNECT, MqttConnect);
  iotjs_jval_set_method(jMQTT, IOTJS_MAGIC_STRING_DISCONNECT, MqttDisconnect);
  iotjs_jval_set_method(jMQTT, IOTJS_MAGIC_STRING_MQTTHANDLE, MqttHandle);
  iotjs_jval_set_method(jMQTT, IOTJS_MAGIC_STRING_PING, MqttPing);
  iotjs_jval_set_method(jMQTT, IOTJS_MAGIC_STRING_PUBLISH, MqttPublish);
  iotjs_jval_set_method(jMQTT, IOTJS_MAGIC_STRING_SENDACK, MqttSendAck);
  iotjs_jval_set_method(jMQTT, IOTJS_MAGIC_STRING_SUBSCRIBE, MqttSubscribe);
  iotjs_jval_set_method(jMQTT, IOTJS_MAGIC_STRING_UNSUBSCRIBE, MqttUnsubscribe);

  return jMQTT;
}
