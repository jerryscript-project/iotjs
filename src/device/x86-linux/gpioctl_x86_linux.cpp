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

#include "iotjs_def.h"
#include "iotjs_objectwrap.h"
#include "iotjs_module_gpioctl.h"

//
// this is a dummy emulator code for x86.
// please add new 'device' file for your purpose.
// use timer with 1msec delay to implement async callback.
// real gpio implementation would be better to use threading
//

namespace iotjs {


// type for delayed callback
typedef enum {
  DELAYEDCALL_SETPIN = 1,
  DELAYEDCALL_WRITEPIN,
  DELAYEDCALL_READPIN,
} DelayedCallType;


// base class for delayed callback
struct DelayedCall {
  DelayedCallType type;
  uv_timer_t timer;
  GpioCbData* data;
  GpioCb after;
  int result;
};


static void timerHandleTimeout(uv_timer_t* handle) {
  uv_timer_stop(handle);

  DelayedCall* base;
  base = reinterpret_cast<DelayedCall*>(handle->data);
  switch (base->type) {
    case DELAYEDCALL_SETPIN : {
      GpioCbData* data =
        reinterpret_cast<GpioCbData*>(base->data);

      DDDLOG("x86 linux gpio dummy setpin w/cb pin(%d), dir(%d), mode(%d)",
             data->pin, data->dir, data->mode);

      base->result = (data->pin >= 0) ? 0 : GPIO_ERR_INVALIDPARAM;
      base->after(data, base->result);
      delete base;
      break;
    }
    case DELAYEDCALL_WRITEPIN : {
      GpioCbData* data =
        reinterpret_cast<GpioCbData*>(base->data);

      DDDLOG("x86 linux gpio dummy writepin w/cb pin(%d), value(%d)",
             data->pin, data->value ? 1 : 0);

      base->result = (data->pin >= 0) ? 0 : GPIO_ERR_INVALIDPARAM;
      base->after(data, base->result);
      delete base;
      break;
    }
    case DELAYEDCALL_READPIN : {
      GpioCbData* data =
        reinterpret_cast<GpioCbData*>(base->data);

      DDDLOG("x86 linux gpio dummy readpin w/cb pin(%d)", data->pin);

      data->value = (data->pin & 0x01) ? true : false;

      base->result = (data->pin >= 0) ? 0 : GPIO_ERR_INVALIDPARAM;
      base->after(data, base->result);
      delete base;
      break;
    }
    default:
      IOTJS_ASSERT(false);
      break;
  }
}


//-----------------------------------------------------------------------------

class GpioControlImpl : public GpioControl {
public:
  explicit GpioControlImpl(JObject& jgpioctl);

  virtual int Initialize(void);
  virtual void Release(void);
  virtual int SetPin(GpioCbData* setpin, GpioCb cb);
  virtual int WritePin(GpioCbData* data, GpioCb cb);
  virtual int ReadPin(GpioCbData* data, GpioCb cb);
};


GpioControl* GpioControl::Create(JObject& jgpioctl)
{
  return new GpioControlImpl(jgpioctl);
}


GpioControlImpl::GpioControlImpl(JObject& jgpioctl)
    : GpioControl(jgpioctl) {
}


int GpioControlImpl::Initialize(void) {
  if (_fd > 0 )
    return GPIO_ERR_INITALIZE;

  DDDLOG("x86 linux gpio dummy initalize");
  _fd = 1;
  return _fd;
}


void GpioControlImpl::Release(void) {
  DDDLOG("x86 linux gpio dummy release");
  _fd = 0;
}


// callback is provided for SetPin
int GpioControlImpl::SetPin(GpioCbData* setpin_data, GpioCb cb) {
  Environment* env = Environment::GetEnv();
  if (cb != NULL) {
    DelayedCall* delay = new DelayedCall();
    delay->type = DELAYEDCALL_SETPIN;
    delay->data = setpin_data;
    delay->after = cb;
    delay->result = 0;

    uv_timer_init(env->loop(), &delay->timer);
    delay->timer.data = delay;
    uv_timer_start(&delay->timer, timerHandleTimeout, 1, 0);
    return 0;
  }
  else {
    return (setpin_data->pin >= 0) ? 0 : GPIO_ERR_INVALIDPARAM;
  }
}


int GpioControlImpl::WritePin(GpioCbData* rw_data, GpioCb cb) {
  Environment* env = Environment::GetEnv();
  if (cb != NULL) {
    DelayedCall* delay = new DelayedCall();
    delay->type = DELAYEDCALL_WRITEPIN;
    delay->data = rw_data;
    delay->after = cb;
    delay->result = 0;

    uv_timer_init(env->loop(), &delay->timer);
    delay->timer.data = delay;
    uv_timer_start(&delay->timer, timerHandleTimeout, 1, 0);
    return 0;
  }
  else {
    return (rw_data->pin >= 0) ? 0 : GPIO_ERR_INVALIDPARAM;
  }
}


int GpioControlImpl::ReadPin(GpioCbData* rw_data, GpioCb cb) {
  Environment* env = Environment::GetEnv();
  DelayedCall* delay = new DelayedCall();
  delay->type = DELAYEDCALL_READPIN;
  delay->data = rw_data;
  delay->after = cb;
  delay->result = 0;

  uv_timer_init(env->loop(), &delay->timer);
  delay->timer.data = delay;
  uv_timer_start(&delay->timer, timerHandleTimeout, 1, 0);

  return 0;
}


} // namespace iotjs
