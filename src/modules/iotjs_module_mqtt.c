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

IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(mqttclient);

static void iotjs_mqttclient_destroy(iotjs_mqttclient_t *mqttclient) {
  IOTJS_RELEASE(mqttclient->buffer);
  IOTJS_RELEASE(mqttclient);
}

iotjs_mqttclient_t *iotjs_mqttclient_create(const jerry_value_t jobject) {
  iotjs_mqttclient_t *mqttclient = IOTJS_ALLOC(iotjs_mqttclient_t);

  jerry_set_object_native_pointer(jobject, mqttclient,
                                  &this_module_native_info);
  return mqttclient;
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


static uint32_t iotjs_decode_remaining_length(char *buffer,
                                              uint32_t *out_length) {
  // There must be at least 2 bytes to decode
  uint32_t c = (uint32_t)(*buffer);
  uint32_t decoded_length = (c & 0x7F);
  uint32_t length = 1;
  uint32_t shift = 7;

  buffer++;

  do {
    if (++length > IOTJS_MODULE_MQTT_MAX_REMAINING_LENGTH_BYTES) {
      return 0;
    }
    c = (uint32_t)(*buffer);
    decoded_length += (c & 0x7F) << shift;

    shift += 7;
    buffer++;
  } while ((c & 0x80) != 0);

  if (c == 0) {
    // The length must be encoded with the least amount of bytes.
    // This rule is not fulfilled if the last byte is zero.
    return 0;
  }

  *out_length = length;
  return decoded_length;
}


static uint16_t iotjs_mqtt_calculate_length(uint8_t msb, uint8_t lsb) {
  return (msb << 8) | lsb;
}


static uint8_t *iotjs_mqtt_string_serialize(uint8_t *dst_buffer,
                                            iotjs_tmp_buffer_t *src_buffer) {
  uint16_t len = src_buffer->length;
  dst_buffer[0] = (uint8_t)(len >> 8);
  dst_buffer[1] = (uint8_t)(len & 0x00FF);
  memcpy(dst_buffer + 2, src_buffer->buffer, src_buffer->length);
  return (dst_buffer + 2 + src_buffer->length);
}

void iotjs_mqtt_ack(char *buffer, char *name, jerry_value_t jsref,
                    char *error) {
  uint16_t package_id =
      iotjs_mqtt_calculate_length((uint8_t)buffer[0], (uint8_t)buffer[1]);

  // The callback takes the packet identifier as parameter.
  jerry_value_t args[2] = { jerry_create_number(package_id),
                            jerry_create_undefined() };

  if (error) {
    args[1] = iotjs_jval_create_error_without_error_flag(error);
  }

  jerry_value_t fn = iotjs_jval_get_property(jsref, name);
  iotjs_invoke_callback(fn, jsref, args, 2);
  jerry_release_value(fn);
  jerry_release_value(args[0]);
  jerry_release_value(args[1]);
}


JS_FUNCTION(mqtt_init) {
  DJS_CHECK_THIS();

  const jerry_value_t jmqtt = JS_GET_ARG(0, object);

  iotjs_mqttclient_t *mqttclient = iotjs_mqttclient_create(jmqtt);
  mqttclient->buffer = NULL;
  mqttclient->buffer_length = 0;

  return jerry_create_undefined();
}


JS_FUNCTION(mqtt_connect) {
  DJS_CHECK_THIS();

  DJS_CHECK_ARGS(1, object);

  jerry_value_t joptions = JS_GET_ARG(0, object);

  iotjs_tmp_buffer_t client_id;
  iotjs_jval_get_jproperty_as_tmp_buffer(joptions, IOTJS_MAGIC_STRING_CLIENTID,
                                         &client_id);
  iotjs_tmp_buffer_t username;
  iotjs_jval_get_jproperty_as_tmp_buffer(joptions, IOTJS_MAGIC_STRING_USERNAME,
                                         &username);
  iotjs_tmp_buffer_t password;
  iotjs_jval_get_jproperty_as_tmp_buffer(joptions, IOTJS_MAGIC_STRING_PASSWORD,
                                         &password);
  iotjs_tmp_buffer_t message;
  iotjs_jval_get_jproperty_as_tmp_buffer(joptions, IOTJS_MAGIC_STRING_MESSAGE,
                                         &message);
  iotjs_tmp_buffer_t topic;
  iotjs_jval_get_jproperty_as_tmp_buffer(joptions, IOTJS_MAGIC_STRING_TOPIC,
                                         &topic);

  if (client_id.buffer == NULL || client_id.length >= UINT16_MAX ||
      username.length >= UINT16_MAX || password.length >= UINT16_MAX ||
      message.length >= UINT16_MAX || topic.length >= UINT16_MAX) {
    iotjs_free_tmp_buffer(&client_id);
    iotjs_free_tmp_buffer(&username);
    iotjs_free_tmp_buffer(&password);
    iotjs_free_tmp_buffer(&message);
    iotjs_free_tmp_buffer(&topic);
    return JS_CREATE_ERROR(COMMON, "mqtt id too long (max: 65535)");
  }

  jerry_value_t jkeepalive =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_KEEPALIVE);
  jerry_value_t jwill =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_WILL);
  jerry_value_t jqos =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_QOS);
  jerry_value_t jretain =
      iotjs_jval_get_property(joptions, IOTJS_MAGIC_STRING_RETAIN);

  uint8_t connect_flags = 0;
  connect_flags |= MQTT_FLAG_CLEANSESSION;

  uint8_t header_byte = 0;
  header_byte |= (CONNECT << 4);

  if (!jerry_value_is_undefined(jwill) && jerry_get_boolean_value(jwill)) {
    connect_flags |= MQTT_FLAG_WILL;
    if (jerry_value_is_number(jqos)) {
      double qos = jerry_get_number_value(jqos);

      if (qos == 1.0) {
        connect_flags |= MQTT_FLAG_WILLQOS_1;
      } else if (qos == 2.0) {
        connect_flags |= MQTT_FLAG_WILLQOS_2;
      }
    }

    if (!jerry_value_is_undefined(jretain) &&
        jerry_get_boolean_value(jretain)) {
      connect_flags |= MQTT_FLAG_WILLRETAIN;
    }
  }

  if (username.buffer != NULL) {
    connect_flags |= MQTT_FLAG_USERNAME;
  }
  if (password.buffer != NULL) {
    connect_flags |= MQTT_FLAG_PASSWORD;
  }

  uint16_t keepalive = 0;

  if (jerry_value_is_number(jkeepalive)) {
    keepalive = (uint16_t)jerry_get_number_value(jkeepalive);
  }

  if (keepalive < 30) {
    keepalive = 30;
  }

  unsigned char variable_header_protocol[7];
  variable_header_protocol[0] = 0;
  variable_header_protocol[1] = 4;
  variable_header_protocol[2] = 'M';
  variable_header_protocol[3] = 'Q';
  variable_header_protocol[4] = 'T';
  variable_header_protocol[5] = 'T';
  variable_header_protocol[6] = 4;

  size_t variable_header_len = sizeof(variable_header_protocol) +
                               sizeof(connect_flags) + IOTJS_MQTT_LSB_MSB_SIZE;

  size_t payload_len = IOTJS_MQTT_LSB_MSB_SIZE + client_id.length;

  if (connect_flags & MQTT_FLAG_USERNAME) {
    payload_len += IOTJS_MQTT_LSB_MSB_SIZE + username.length;
  }
  if (connect_flags & MQTT_FLAG_PASSWORD) {
    payload_len += IOTJS_MQTT_LSB_MSB_SIZE + password.length;
  }
  if (connect_flags & MQTT_FLAG_WILL) {
    payload_len += IOTJS_MQTT_LSB_MSB_SIZE + topic.length;
    payload_len += IOTJS_MQTT_LSB_MSB_SIZE + message.length;
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
  *buff_ptr++ = (uint8_t)(keepalive >> 8);
  *buff_ptr++ = (uint8_t)(keepalive & 0x00FF);

  buff_ptr = iotjs_mqtt_string_serialize(buff_ptr, &client_id);

  if (connect_flags & MQTT_FLAG_WILL) {
    buff_ptr = iotjs_mqtt_string_serialize(buff_ptr, &topic);
    buff_ptr = iotjs_mqtt_string_serialize(buff_ptr, &message);
  }

  if (connect_flags & MQTT_FLAG_USERNAME) {
    buff_ptr = iotjs_mqtt_string_serialize(buff_ptr, &username);
  }
  if (connect_flags & MQTT_FLAG_PASSWORD) {
    buff_ptr = iotjs_mqtt_string_serialize(buff_ptr, &password);
  }

  iotjs_free_tmp_buffer(&client_id);
  iotjs_free_tmp_buffer(&username);
  iotjs_free_tmp_buffer(&password);
  iotjs_free_tmp_buffer(&message);
  iotjs_free_tmp_buffer(&topic);

  jerry_release_value(jkeepalive);
  jerry_release_value(jwill);
  jerry_release_value(jqos);
  jerry_release_value(jretain);

  return jbuff;
}


JS_FUNCTION(mqtt_publish) {
  DJS_CHECK_THIS();

  DJS_CHECK_ARGS(3, any, any, number);

  iotjs_tmp_buffer_t topic;
  iotjs_jval_as_tmp_buffer(JS_GET_ARG(0, any), &topic);

  if (jerry_value_is_error(topic.jval)) {
    return topic.jval;
  }

  if (topic.buffer == NULL || topic.length >= UINT16_MAX) {
    iotjs_free_tmp_buffer(&topic);

    return JS_CREATE_ERROR(COMMON, "Topic for PUBLISH is empty or too long.");
  }

  iotjs_tmp_buffer_t message;
  iotjs_jval_as_tmp_buffer(JS_GET_ARG(1, any), &message);

  if (jerry_value_is_error(message.jval)) {
    iotjs_free_tmp_buffer(&topic);
    return message.jval;
  }

  // header bits: | 16 bit packet id | 4 bit PUBLISH header |
  uint32_t header = (uint32_t)JS_GET_ARG(2, number);
  uint32_t packet_identifier = (header >> 4);

  uint8_t header_byte = 0;
  header_byte |= (PUBLISH << 4) | (header & 0xf);

  const uint8_t qos_mask = (0x3 << 1);

  size_t payload_len = message.length + IOTJS_MQTT_LSB_MSB_SIZE;
  size_t variable_header_len = topic.length + IOTJS_MQTT_LSB_MSB_SIZE;

  if (header_byte & qos_mask) {
    variable_header_len += IOTJS_MQTT_LSB_MSB_SIZE;
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
  buff_ptr = iotjs_mqtt_string_serialize(buff_ptr, &topic);

  if (header_byte & qos_mask) {
    // Packet id format is MSB / LSB.
    *buff_ptr++ = (uint8_t)(packet_identifier >> 8);
    *buff_ptr++ = (uint8_t)(packet_identifier & 0x00FF);
  }

  // Don't need to put length before the payload, so we can't use the
  // iotjs_mqtt_string_serialize. The broker and the other clients calculate
  // the payload length from remaining length and the topic length.
  memcpy(buff_ptr, message.buffer, message.length);

  iotjs_free_tmp_buffer(&message);
  iotjs_free_tmp_buffer(&topic);
  return jbuff;
}


static int iotjs_mqtt_handle(jerry_value_t jsref, char first_byte, char *buffer,
                             uint32_t packet_size) {
  char packet_type = (first_byte >> 4) & 0x0F;

  switch (packet_type) {
    case CONNACK: {
      if (packet_size != 2) {
        return MQTT_ERR_CORRUPTED_PACKET;
      }

      uint8_t return_code = (uint8_t)buffer[1];

      if (return_code != 0) {
        return return_code;
      }

      jerry_value_t fn =
          iotjs_jval_get_property(jsref, IOTJS_MAGIC_STRING_ONCONNECTION);
      iotjs_invoke_callback(fn, jsref, NULL, 0);

      jerry_release_value(fn);
      break;
    }
    case PUBLISH: {
      mqtt_header_t header = { 0 };
      header.bits.type = PUBLISH;
      header.bits.dup = first_byte & 0x08;
      header.bits.qos = (first_byte & 0x06) >> 1;
      header.bits.retain = first_byte & 0x01;

      uint8_t topic_length_msb = (uint8_t)buffer[0];
      uint8_t topic_length_lsb = (uint8_t)buffer[1];
      buffer += 2;

      uint16_t topic_length =
          iotjs_mqtt_calculate_length(topic_length_msb, topic_length_lsb);

      if (!jerry_is_valid_utf8_string((const uint8_t *)buffer, topic_length)) {
        return MQTT_ERR_CORRUPTED_PACKET;
      }

      const jerry_char_t *topic = (const jerry_char_t *)buffer;
      jerry_value_t jtopic = jerry_create_string_sz(topic, topic_length);
      buffer += topic_length;

      // The Packet Identifier field is only present in PUBLISH packets
      // where the QoS level is 1 or 2.
      uint16_t packet_identifier = 0;
      if (header.bits.qos > 0) {
        uint8_t packet_identifier_msb = (uint8_t)buffer[0];
        uint8_t packet_identifier_lsb = (uint8_t)buffer[1];
        buffer += 2;

        packet_identifier = iotjs_mqtt_calculate_length(packet_identifier_msb,
                                                        packet_identifier_lsb);
      }

      size_t payload_length =
          (size_t)packet_size - topic_length - sizeof(topic_length);

      if (header.bits.qos > 0) {
        payload_length -= sizeof(packet_identifier);
      }

      jerry_value_t jmessage = iotjs_bufferwrap_create_buffer(payload_length);
      iotjs_bufferwrap_t *msg_wrap = iotjs_bufferwrap_from_jbuffer(jmessage);

      memcpy(msg_wrap->buffer, buffer, payload_length);

      jerry_value_t args[4] = { jmessage, jtopic,
                                jerry_create_number(header.bits.qos),
                                jerry_create_number(packet_identifier) };

      jerry_value_t fn =
          iotjs_jval_get_property(jsref, IOTJS_MAGIC_STRING_ONMESSAGE);
      iotjs_invoke_callback(fn, jsref, args, 4);
      jerry_release_value(fn);

      for (uint8_t i = 0; i < 4; i++) {
        jerry_release_value(args[i]);
      }

      break;
    }
    case PUBACK: {
      if (packet_size != 2 || (first_byte & 0x0F) != 0x0) {
        return MQTT_ERR_CORRUPTED_PACKET;
      }

      iotjs_mqtt_ack(buffer, IOTJS_MAGIC_STRING_ONACK, jsref, NULL);
      break;
    }
    case PUBREC: {
      if (packet_size != 2 || (first_byte & 0x0F) != 0x0) {
        return MQTT_ERR_CORRUPTED_PACKET;
      }

      iotjs_mqtt_ack(buffer, IOTJS_MAGIC_STRING_ONPUBREC, jsref, NULL);
      break;
    }
    case PUBREL: {
      if (packet_size != 2 || (first_byte & 0x0F) != 0x2) {
        return MQTT_ERR_CORRUPTED_PACKET;
      }

      iotjs_mqtt_ack(buffer, IOTJS_MAGIC_STRING_ONPUBREL, jsref, NULL);
      break;
    }
    case PUBCOMP: {
      if (packet_size != 2 || (first_byte & 0x0F) != 0x0) {
        return MQTT_ERR_CORRUPTED_PACKET;
      }

      iotjs_mqtt_ack(buffer, IOTJS_MAGIC_STRING_ONACK, jsref, NULL);
      break;
    }
    case SUBACK: {
      // We assume that only one topic was in the SUBSCRIBE packet.
      if (packet_size != 3 || (first_byte & 0x0F) != 0x0) {
        return MQTT_ERR_CORRUPTED_PACKET;
      }

      char *error = NULL;

      if ((uint8_t)buffer[2] == 0x80) {
        error = "Subscribe failed";
      }

      iotjs_mqtt_ack(buffer, IOTJS_MAGIC_STRING_ONACK, jsref, error);
      break;
    }
    case UNSUBACK: {
      if (packet_size != 2 || (first_byte & 0x0F) != 0x0) {
        return MQTT_ERR_CORRUPTED_PACKET;
      }

      iotjs_mqtt_ack(buffer, IOTJS_MAGIC_STRING_ONACK, jsref, NULL);
      break;
    }
    case PINGRESP: {
      if (packet_size != 0 || (first_byte & 0x0F) != 0x0) {
        return MQTT_ERR_CORRUPTED_PACKET;
      }

      jerry_value_t fn =
          iotjs_jval_get_property(jsref, IOTJS_MAGIC_STRING_ONPINGRESP);
      iotjs_invoke_callback(fn, jsref, NULL, 0);
      jerry_release_value(fn);
      break;
    }
    case DISCONNECT: {
      if (packet_size != 0 || (first_byte & 0x0F) != 0x0) {
        return MQTT_ERR_CORRUPTED_PACKET;
      }

      jerry_value_t fn =
          iotjs_jval_get_property(jsref, IOTJS_MAGIC_STRING_ONEND);
      iotjs_invoke_callback(fn, jsref, NULL, 0);
      jerry_release_value(fn);
      break;
    }

    case CONNECT:
    case SUBSCRIBE:
    case UNSUBSCRIBE:
    case PINGREQ:
      return MQTT_ERR_UNALLOWED_PACKET;
  }

  return 0;
}


static void iotjs_mqtt_concat_buffers(iotjs_mqttclient_t *mqttclient,
                                      iotjs_bufferwrap_t *buff_recv) {
  char *tmp_buf = mqttclient->buffer;
  mqttclient->buffer =
      IOTJS_CALLOC(mqttclient->buffer_length + buff_recv->length, char);
  memcpy(mqttclient->buffer, tmp_buf, mqttclient->buffer_length);
  memcpy(mqttclient->buffer + mqttclient->buffer_length, buff_recv->buffer,
         buff_recv->length);
  mqttclient->buffer_length += buff_recv->length;
  IOTJS_RELEASE(tmp_buf);
}


static jerry_value_t iotjs_mqtt_handle_error(
    iotjs_mqtt_packet_error_t error_code) {
  switch (error_code) {
    case MQTT_ERR_UNACCEPTABLE_PROTOCOL:
      return JS_CREATE_ERROR(COMMON,
                             "MQTT: Connection refused: unacceptable protocol");
    case MQTT_ERR_BAD_IDENTIFIER:
      return JS_CREATE_ERROR(COMMON,
                             "MQTT: Connection refused: bad client identifier");
    case MQTT_ERR_SERVER_UNAVIABLE:
      return JS_CREATE_ERROR(COMMON,
                             "MQTT: Connection refused: server unavailable");
    case MQTT_ERR_BAD_CREDENTIALS:
      return JS_CREATE_ERROR(
          COMMON, "MQTT: Connection refused: bad username or password");
    case MQTT_ERR_UNAUTHORISED:
      return JS_CREATE_ERROR(COMMON, "MQTT: Connection refused: unauthorised");
    case MQTT_ERR_CORRUPTED_PACKET:
      return JS_CREATE_ERROR(COMMON, "MQTT: Corrupted packet");
    case MQTT_ERR_UNALLOWED_PACKET:
      return JS_CREATE_ERROR(COMMON, "MQTT: Broker sent an unallowed packet");
    case MQTT_ERR_SUBSCRIPTION_FAILED:
      return JS_CREATE_ERROR(COMMON, "MQTT: Subscription failed");
    default:
      return JS_CREATE_ERROR(COMMON, "MQTT: Unknown error");
  }
}


JS_FUNCTION(mqtt_receive) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(2, object, object);

  jerry_value_t jnat = JS_GET_ARG(0, object);

  iotjs_mqttclient_t *mqttclient = NULL;
  if (!jerry_get_object_native_pointer(jnat, (void **)&mqttclient,
                                       &this_module_native_info)) {
    return JS_CREATE_ERROR(COMMON, "MQTT native pointer not available");
  }

  jerry_value_t jbuffer = JS_GET_ARG(1, object);
  iotjs_bufferwrap_t *buff_recv = iotjs_bufferwrap_from_jbuffer(jbuffer);
  if (buff_recv->length == 0) {
    return jerry_create_undefined();
  }

  char *current_buffer = buff_recv->buffer;
  char *current_buffer_end = current_buffer + buff_recv->length;

  // Concat the buffers if we previously needed to
  if (mqttclient->buffer_length > 0) {
    iotjs_mqtt_concat_buffers(mqttclient, buff_recv);
    current_buffer = mqttclient->buffer;
    current_buffer_end = current_buffer + mqttclient->buffer_length;
  }

  // Keep on going, if data remains just continue looping
  while (true) {
    if (current_buffer >= current_buffer_end) {
      if (mqttclient->buffer_length > 0) {
        IOTJS_RELEASE(mqttclient->buffer);
        mqttclient->buffer_length = 0;
      }
      return jerry_create_undefined();
    }

    if (current_buffer + 2 > current_buffer_end) {
      break;
    }

    uint32_t packet_size;
    uint32_t packet_size_length;

    if ((uint8_t)current_buffer[1] <= 0x7f) {
      packet_size = (uint32_t)current_buffer[1];
      packet_size_length = 1;

      if (current_buffer + 2 + packet_size > current_buffer_end) {
        break;
      }
    } else {
      // At least 128 bytes arrived
      if (current_buffer + 5 >= current_buffer_end) {
        break;
      }

      packet_size = iotjs_decode_remaining_length(current_buffer + 1,
                                                  &packet_size_length);

      if (packet_size == 0) {
        return iotjs_mqtt_handle_error(MQTT_ERR_CORRUPTED_PACKET);
      }

      if (current_buffer + 1 + packet_size_length + packet_size >
          current_buffer_end) {
        break;
      }
    }

    char first_byte = current_buffer[0];
    current_buffer += 1 + packet_size_length;

    int ret_val =
        iotjs_mqtt_handle(jnat, first_byte, current_buffer, packet_size);

    if (ret_val != 0) {
      return iotjs_mqtt_handle_error(ret_val);
    }

    current_buffer += packet_size;
  }

  if (current_buffer == mqttclient->buffer) {
    return jerry_create_undefined();
  }

  uint32_t remaining_size = (uint32_t)(current_buffer_end - current_buffer);
  char *buffer = IOTJS_CALLOC(remaining_size, char);
  memcpy(buffer, current_buffer, remaining_size);

  if (mqttclient->buffer != NULL) {
    IOTJS_RELEASE(mqttclient->buffer);
  }

  mqttclient->buffer = buffer;
  mqttclient->buffer_length = remaining_size;

  return jerry_create_undefined();
}

static jerry_value_t iotjs_mqtt_subscribe_handler(
    const jerry_value_t jthis, const jerry_value_t jargv[],
    const jerry_value_t jargc,
    const iotjs_mqtt_control_packet_type packet_type) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(2, any, number);

  DJS_CHECK(packet_type == SUBSCRIBE || packet_type == UNSUBSCRIBE);

  iotjs_tmp_buffer_t topic;
  iotjs_jval_as_tmp_buffer(JS_GET_ARG(0, any), &topic);

  if (jerry_value_is_error(topic.jval)) {
    return topic.jval;
  }

  if (topic.buffer == NULL || topic.length >= UINT16_MAX) {
    iotjs_free_tmp_buffer(&topic);

    return JS_CREATE_ERROR(COMMON, "Topic for SUBSCRIBE is empty or too long.");
  }

  // header bits: |2 bit qos | 16 bit packet id |
  // qos is only available in case of subscribe
  uint32_t header = (uint32_t)JS_GET_ARG(1, number);
  uint32_t packet_identifier = (header & 0xFFFF);

  // Low 4 bits must be 0,0,1,0
  uint8_t header_byte = (packet_type << 4) | (1 << 1);

  size_t payload_len = topic.length + IOTJS_MQTT_LSB_MSB_SIZE;

  if (packet_type == SUBSCRIBE) {
    // Add place for the SUBSCRIBE QoS data.
    payload_len += sizeof(uint8_t);
  }

  size_t variable_header_len = IOTJS_MQTT_LSB_MSB_SIZE;
  uint32_t remaining_length = payload_len + variable_header_len;
  size_t full_len = sizeof(header_byte) +
                    get_remaining_length_size(remaining_length) +
                    remaining_length;

  jerry_value_t jbuff = iotjs_bufferwrap_create_buffer(full_len);
  iotjs_bufferwrap_t *buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuff);

  uint8_t *buff_ptr = (uint8_t *)buffer_wrap->buffer;

  *buff_ptr++ = header_byte;

  buff_ptr = iotjs_encode_remaining_length(buff_ptr, remaining_length);

  // Packet id format is MSB / LSB.
  *buff_ptr++ = (uint8_t)(packet_identifier >> 8);
  *buff_ptr++ = (uint8_t)(packet_identifier & 0x00FF);

  buff_ptr = iotjs_mqtt_string_serialize(buff_ptr, &topic);

  if (packet_type == SUBSCRIBE) {
    uint8_t qos = ((header >> 16) & 0x3);
    buff_ptr[0] = qos;
  }

  iotjs_free_tmp_buffer(&topic);
  return jbuff;
}


JS_FUNCTION(mqtt_unsubscribe) {
  return iotjs_mqtt_subscribe_handler(jthis, jargv, jargc, UNSUBSCRIBE);
}


JS_FUNCTION(mqtt_subscribe) {
  return iotjs_mqtt_subscribe_handler(jthis, jargv, jargc, SUBSCRIBE);
}


JS_FUNCTION(mqtt_ping) {
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


JS_FUNCTION(mqtt_disconnect) {
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

JS_FUNCTION(mqtt_send_ack) {
  DJS_CHECK_THIS();
  DJS_CHECK_ARGS(2, number, number);

  uint8_t ack_type = (uint8_t)JS_GET_ARG(0, number);
  uint16_t packet_id = (uint16_t)JS_GET_ARG(1, number);

  uint8_t header_byte = (ack_type << 4);

  if (ack_type == PUBREL) {
    header_byte |= 0x2;
  }

  uint8_t packet_identifier_msb = (uint8_t)(packet_id >> 8);
  uint8_t packet_identifier_lsb = (uint8_t)(packet_id & 0x00FF);

  size_t full_len = sizeof(uint8_t) * 2 + sizeof(packet_id);

  jerry_value_t jbuff = iotjs_bufferwrap_create_buffer(full_len);
  iotjs_bufferwrap_t *buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuff);

  uint8_t *buff_ptr = (uint8_t *)buffer_wrap->buffer;

  buff_ptr[0] = header_byte;
  buff_ptr[1] = 2; /* length */
  buff_ptr[2] = packet_identifier_msb;
  buff_ptr[3] = packet_identifier_lsb;

  return jbuff;
}

jerry_value_t iotjs_init_mqtt(void) {
  jerry_value_t jmqtt = jerry_create_object();
  iotjs_jval_set_method(jmqtt, IOTJS_MAGIC_STRING_CONNECT, mqtt_connect);
  iotjs_jval_set_method(jmqtt, IOTJS_MAGIC_STRING_DISCONNECT, mqtt_disconnect);
  iotjs_jval_set_method(jmqtt, IOTJS_MAGIC_STRING_PING, mqtt_ping);
  iotjs_jval_set_method(jmqtt, IOTJS_MAGIC_STRING_PUBLISH, mqtt_publish);
  iotjs_jval_set_method(jmqtt, IOTJS_MAGIC_STRING_MQTTINIT, mqtt_init);
  iotjs_jval_set_method(jmqtt, IOTJS_MAGIC_STRING_MQTTRECEIVE, mqtt_receive);
  iotjs_jval_set_method(jmqtt, IOTJS_MAGIC_STRING_SENDACK, mqtt_send_ack);
  iotjs_jval_set_method(jmqtt, IOTJS_MAGIC_STRING_SUBSCRIBE, mqtt_subscribe);
  iotjs_jval_set_method(jmqtt, IOTJS_MAGIC_STRING_UNSUBSCRIBE,
                        mqtt_unsubscribe);

  return jmqtt;
}
