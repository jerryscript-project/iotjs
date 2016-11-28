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

#ifndef IOTJS_MODULE_BLECENTRAL_H
#define IOTJS_MODULE_BLECENTRAL_H

#include "iotjs_def.h"
#include "iotjs_objectwrap.h"
#include "iotjs_reqwrap.h"

typedef enum {
  kBleCentralOpStartScanning,
  kBleCentralOpStopScanning,
  kBleCentralOpConnect,
  kBleCentralOpDisconnect,
  kBleCentralOpDiscoverServices,
  kBleCentralOpReadHandle,
  kBleCentralOpWriteHandle,
  kBleCentralOpDiscoverIncludedServices,
  kBleCentralOpDiscoverCharacteristics,
  kBleCentralOpRead,
  kBleCentralOpWrite,
  kBleCentralOpBroadcast,
  kBleCentralOpNotify,
  kBleCentralOpDiscoverDescriptors,
  kBleCentralOpReadValue,
  kBleCentralOpWriteValue,
} BleCentralOp;

typedef enum {
  kBleCentralErrNone = 0,
  kBleCentralErrFail = -1,
} BleCentralErr;

typedef struct {
  BleCentralOp op;
  BleCentralErr err;
} iotjs_blecentral_reqdata_t;

typedef struct {
  uv_work_t req;
  iotjs_blecentral_reqdata_t req_data;
} IOTJS_VALIDATED_STRUCT(iotjs_blecentral_reqwrap_t);

typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;
} IOTJS_VALIDATED_STRUCT(iotjs_blecentral_t);

iotjs_blecentral_t *iotjs_blecentral_create(const iotjs_jval_t *jble_central);
const iotjs_jval_t *iotjs_blecentral_get_jblecentral();
iotjs_blecentral_t *iotjs_blecentral_get_instance();

void binding_start_scanning(const char *svc_uuid, int allow_duplicates);
void binding_stop_scanning(void);
void binding_connect(const char *perip_uuid);
void binding_disconnect(const char *perip_uuid);
void binding_discover_service(const char *perip_uuid);
void binding_read_handle(const char *perip_uuid, const char *handle);
void binding_write_handle(const char *perip_uuid, const char *handle,
                          const char *buffer, int len, int without_response);
void binding_discover_included_services(const char *perip_uuid,
                                        const char *svc_uuid);
void binding_discover_characteristics(const char *perip_uuid,
                                      const char *svc_uuid);
void binding_read(const char *perip_uuid, const char *svc_uuid,
                  const char *char_uuid);
void binding_write(const char *perip_uuid, const char *svc_uuid,
                   const char *char_uuid, const char *buffer, int len,
                   int without_response);
void binding_broadcast(const char *perip_uuid, const char *svc_uuid,
                       const char *char_uuid, int broadcast);
void binding_notify(const char *perip_uuid, const char *svc_uuid,
                    const char *char_uuid, int notify);
void binding_discover_descriptors(const char *perip_uuid, const char *svc_uuid,
                                  const char *char_uuid);
void binding_read_value(const char *perip_uuid, const char *svc_uuid,
                        const char *char_uuid, const char *desc_uuid);
void binding_write_value(const char *perip_uuid, const char *svc_uuid,
                         const char *char_uuid, const char *desc_uuid,
                         const char *buffer, int len);
void binding_value(void);
void binding_listen(void);

#endif /* IOTJS_MODULE_BLECENTRAL_H */
