/* Copyright 2016 Samsung Electronics Co., Ltd.
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

#include "iotjs_def.h"
#include "iotjs_objectwrap.h"
#include "iotjs_module_pwm.h"


namespace iotjs {


Pwm::Pwm(const iotjs_jval_t* jpwm)
    : JObjectWrap(jpwm) {
}


Pwm::~Pwm() {
}


const iotjs_jval_t* Pwm::GetJPwm() {
  return iotjs_module_get(MODULE_PWM);
}


Pwm* Pwm::GetInstance() {
  Pwm* pwm = reinterpret_cast<Pwm*>(
          iotjs_jval_get_object_native_handle(Pwm::GetJPwm()));
  IOTJS_ASSERT(pwm != NULL);

  return pwm;
}


JHANDLER_FUNCTION(Export) {
  IOTJS_ASSERT(!"Not implemented");

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(SetDutyCycle) {
  IOTJS_ASSERT(!"Not implemented");

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(SetPeriod) {
  IOTJS_ASSERT(!"Not implemented");

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Enable) {
  IOTJS_ASSERT(!"Not implemented");

  iotjs_jhandler_return_null(jhandler);
}


JHANDLER_FUNCTION(Unexport) {
  IOTJS_ASSERT(!"Not implemented");

  iotjs_jhandler_return_null(jhandler);
}


iotjs_jval_t InitPwm() {

  iotjs_jval_t jpwm = iotjs_jval_create_object();

  iotjs_jval_set_method(&jpwm, "export", Export);
  iotjs_jval_set_method(&jpwm, "setDutyCycle", SetDutyCycle);
  iotjs_jval_set_method(&jpwm, "setPeriod", SetPeriod);
  iotjs_jval_set_method(&jpwm, "enable", Enable);
  iotjs_jval_set_method(&jpwm, "unexport", Unexport);

  // TODO: Need to implement Create for each platform.
  // Pwm* pwm = Pwm::Create(*jpwm);
  // IOTJS_ASSERT(pwm == reinterpret_cast<Pwm*>(jpwm->GetNative()));

  return jpwm;
}


} // namespace iotjs


extern "C" {

iotjs_jval_t InitPwm() {
  return iotjs::InitPwm();
}

} // extern "C"
