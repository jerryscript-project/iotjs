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


Pwm::Pwm(JObject& jpwm)
    : JObjectWrap(jpwm) {
}


Pwm::~Pwm() {
}


JObject* Pwm::GetJPwm() {
  Module* module = GetBuiltinModule(MODULE_PWM);
  JObject* jpwm = module->module;
  IOTJS_ASSERT(jpwm != NULL);

  return jpwm;
}


Pwm* Pwm::GetInstance() {
  Pwm* pwm = reinterpret_cast<Pwm*>(Pwm::GetJPwm()->GetNative());
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


JObject* InitPwm() {
  Module* module = GetBuiltinModule(MODULE_PWM);
  JObject* jpwm = module->module;

  if (jpwm == NULL) {
    jpwm = new JObject();

    jpwm->SetMethod("export", Export);
    jpwm->SetMethod("setDutyCycle", SetDutyCycle);
    jpwm->SetMethod("setPeriod", SetPeriod);
    jpwm->SetMethod("enable", Enable);
    jpwm->SetMethod("unexport", Unexport);

    // TODO: Need to implement Create for each platform.
    // Pwm* pwm = Pwm::Create(*jpwm);
    // IOTJS_ASSERT(pwm == reinterpret_cast<Pwm*>(jpwm->GetNative()));

    module->module = jpwm;
  }

  return jpwm;
}


} // namespace iotjs
