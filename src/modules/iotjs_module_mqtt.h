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

#ifndef IOTJS_MODULE_MQTT_H
#define IOTJS_MODULE_MQTT_H

#include "iotjs_def.h"

#define IOTJS_MODULE_MQTT_MAX_REMAINING_LENGTH_BYTES 4
#define IOTJS_MQTT_LSB_MSB_SIZE 2

/*
 * The types of the control packet.
 * These values determine the aim of the message.
 */
typedef enum {
  CONNECT = 0x1,
  CONNACK = 0x2,
  PUBLISH = 0x3,
  PUBACK = 0x4,
  PUBREC = 0x5,
  PUBREL = 0x6,
  PUBCOMP = 0x7,
  SUBSCRIBE = 0x8,
  SUBACK = 0x9,
  UNSUBSCRIBE = 0xA,
  UNSUBACK = 0xB,
  PINGREQ = 0xC,
  PINGRESP = 0xD,
  DISCONNECT = 0xE
} iotjs_mqtt_control_packet_type;

// Packet error types
typedef enum {
  MQTT_ERR_UNACCEPTABLE_PROTOCOL = 1,
  MQTT_ERR_BAD_IDENTIFIER = 2,
  MQTT_ERR_SERVER_UNAVIABLE = 3,
  MQTT_ERR_BAD_CREDENTIALS = 4,
  MQTT_ERR_UNAUTHORISED = 5,
  MQTT_ERR_CORRUPTED_PACKET = 6,
  MQTT_ERR_UNALLOWED_PACKET = 7,
  MQTT_ERR_SUBSCRIPTION_FAILED = 8,
} iotjs_mqtt_packet_error_t;

/*
 * The values of the Quality of Service.
 */
enum {
  QoS0 = 0, // At most once delivery.
  QoS1 = 1, // At least once delivery.
  QoS2 = 2  // Exactly once delivery.
} iotjs_mqtt_quality_of_service;

/*
 * First byte of the message's fixed header.
 * Contains:
 * - MQTT Control Packet type,
 * - Specific flags to each MQTT Control Packet.
 */
typedef struct {
  uint8_t RETAIN : 1;    // PUBLISH Retain flag.
  unsigned char QoS : 2; // PUBLISH Quality of Service.
  uint8_t DUP : 1;       // Duplicate delivery of PUBLISH Control Packet.
  unsigned char packet_type : 4;
} iotjs_mqtt_control_packet_t;

/*
 * The fixed header of the MQTT message structure.
 */
typedef struct {
  iotjs_mqtt_control_packet_t packet;
  uint8_t remaining_length;
} iotjs_mqtt_fixed_header_t;

/*
 * Type of the MQTT CONNECT message.
 */

typedef union {
  unsigned char byte;

  struct {
    uint8_t retain : 1;
    uint8_t qos : 2;
    uint8_t dup : 1;
    uint8_t type : 4;
  } bits;
} mqtt_header_t;

enum {
  // Reserved bit, must be 0
  MQTT_FLAG_RESERVED = 1 << 0,
  // Clean session bit
  MQTT_FLAG_CLEANSESSION = 1 << 1,
  /**
   * If the will flag is set to 1 Will QoS and Will Retain flags must be
   * also set to 1, and be present in the payload. Otherwise, both must be set
   * to 0.
   */
  MQTT_FLAG_WILL = 1 << 2,
  /**
   * QoS can only be set, if the Will flag is set to 1. Otherwise it's 0x00.
   * QoS types are as follows:
   * Type 0: Both QoS bits are set to 0 (0x00)
   * Type 1: WILLQOS_1 is set to 1, WILLQOS_2 is set to 0 (0x01)
   * Type 2: WILLQOS_2 is set to 1, WILLQOS_1 is set to 0 (0x02)
   */
  MQTT_FLAG_WILLQOS_1 = 1 << 3,
  MQTT_FLAG_WILLQOS_2 = 1 << 4,
  /**
   * Will retain flag can only be set to 1 if Will flag is set to 1 as well.
   * If retain is set to 1, the server must publish will message as a retained
   * message.
   */
  MQTT_FLAG_WILLRETAIN = 1 << 5,
  // Whether password is sent by the user
  MQTT_FLAG_PASSWORD = 1 << 6,
  // Whether username is sent
  MQTT_FLAG_USERNAME = 1 << 7
} iotjs_mqtt_connect_flag_t;

typedef struct {
  char *buffer;
  uint32_t buffer_length;
} iotjs_mqttclient_t;

#endif /* IOTJS_MODULE_MQTT_H */
