/* Copyright 2015 Samsung Electronics Co., Ltd.
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


#if defined(__LINUX__)

 #if defined(USING_MRAA)

#include "../iotjs_module_gpio-linux-mraa.inl.h"


namespace iotjs {


Gpio* Gpio::Create(JObject& jgpio) {
  return new GpioLinuxMraa(jgpio);
}


} // namespace iotjs

 #else

#include "../iotjs_module_gpio-linux-general.inl.h"


namespace iotjs {


Gpio* Gpio::Create(JObject& jgpio) {
  return new GpioLinuxGeneral(jgpio);
}


} // namespace iotjs

 #endif

#endif
