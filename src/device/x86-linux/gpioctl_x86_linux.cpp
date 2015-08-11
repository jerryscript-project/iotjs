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
struct DelayedCallBase {
  DelayedCallType type;
  uv_timer_t timer;
};


// setspin delayed callback class
struct DelayedCallSetpin : public DelayedCallBase {
  GpioCbDataSetpin* data;
  GpioSetpinCb aftersetpin;
  int result;
};

struct DelayedCallRWpin : public DelayedCallBase {
  GpioCbDataRWpin* data;
  GpioRWpinCb afterrwpin;
  int result;
};


static void timerHandleTimeout(uv_timer_t* handle) {
  uv_timer_stop(handle);

  DelayedCallBase* base;
  base = reinterpret_cast<DelayedCallBase*>(handle->data);
  switch (base->type) {
    case DELAYEDCALL_SETPIN : {
      DelayedCallSetpin* dcsp =
                            reinterpret_cast<DelayedCallSetpin*>(base);
      GpioCbDataSetpin* setpin =
                            reinterpret_cast<GpioCbDataSetpin*>(dcsp->data);

      DDDLOG("x86 linux gpio dummy setpin w/cb pin(%d), dir(%d), mode(%d)",
             setpin->pin, setpin->dir, setpin->mode);

      dcsp->result = (setpin->pin >= 0) ? 0 : GPIO_ERR_INVALIDPARAM;
      dcsp->aftersetpin(dcsp->data, dcsp->result);
      delete dcsp;
      break;
    }
    case DELAYEDCALL_WRITEPIN : {
      DelayedCallRWpin* dcrwp =
                            reinterpret_cast<DelayedCallRWpin*>(base);
      GpioCbDataRWpin* rwpin =
                            reinterpret_cast<GpioCbDataRWpin*>(dcrwp->data);

      DDDLOG("x86 linux gpio dummy writepin w/cb pin(%d), value(%d)",
             rwpin->pin, rwpin->value ? 1 : 0);

      dcrwp->result = (rwpin->pin >= 0) ? 0 : GPIO_ERR_INVALIDPARAM;
      dcrwp->afterrwpin(dcrwp->data, dcrwp->result);
      delete dcrwp;
      break;
    }
    case DELAYEDCALL_READPIN : {
      DelayedCallRWpin* dcrwp =
                            reinterpret_cast<DelayedCallRWpin*>(base);
      GpioCbDataRWpin* rwpin =
                            reinterpret_cast<GpioCbDataRWpin*>(dcrwp->data);

      DDDLOG("x86 linux gpio dummy readpin w/cb pin(%d)", rwpin->pin);

      rwpin->value = (rwpin->pin & 0x01) ? true : false;

      dcrwp->result = (rwpin->pin >= 0) ? 0 : GPIO_ERR_INVALIDPARAM;
      dcrwp->afterrwpin(dcrwp->data, dcrwp->result);
      delete dcrwp;
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
  virtual int SetPin(GpioCbDataSetpin* setpin, GpioSetpinCb cb);
  virtual int SetPin(int32_t pin, int32_t dir, int32_t mode);
  virtual int WritePin(GpioCbDataRWpin* data, GpioRWpinCb cb);
  virtual int WritePin(int32_t pin, bool value);
  virtual int ReadPin(GpioCbDataRWpin* data, GpioRWpinCb cb);
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
int GpioControlImpl::SetPin(GpioCbDataSetpin* setpin_data, GpioSetpinCb cb) {
  Environment* env = Environment::GetEnv();
  DelayedCallSetpin* delay = new DelayedCallSetpin;
  delay->type = DELAYEDCALL_SETPIN;
  delay->data = setpin_data;
  delay->aftersetpin = cb;
  delay->result = 0;

  uv_timer_init(env->loop(), &delay->timer);
  delay->timer.data = delay;
  uv_timer_start(&delay->timer, timerHandleTimeout, 1, 0);

  return 0;
}


// callback is omiited for SetPin
int GpioControlImpl::SetPin(int32_t pin, int32_t dir, int32_t mode) {
  DDDLOG("x86 linux gpio dummy setpin pin(%d), dir(%d), mode(%d)",
         pin, dir, mode);

  // return result of SetPin
  return (pin >= 0) ? 0 : GPIO_ERR_INVALIDPARAM;
}


int GpioControlImpl::WritePin(GpioCbDataRWpin* rw_data, GpioRWpinCb cb) {
  Environment* env = Environment::GetEnv();
  DelayedCallRWpin* delay = new DelayedCallRWpin;
  delay->type = DELAYEDCALL_WRITEPIN;
  delay->data = rw_data;
  delay->afterrwpin = cb;
  delay->result = 0;

  uv_timer_init(env->loop(), &delay->timer);
  delay->timer.data = delay;
  uv_timer_start(&delay->timer, timerHandleTimeout, 1, 0);

  return 0;
}


int GpioControlImpl::WritePin(int32_t pin, bool value) {
  DDDLOG("x86 linux gpio dummy writepin pin(%d), value(%d)",
         pin, value ? 1 : 0);

  // return result of WritePin
  return (pin >= 0) ? 0 : GPIO_ERR_INVALIDPARAM;
}


int GpioControlImpl::ReadPin(GpioCbDataRWpin* rw_data, GpioRWpinCb cb) {
  Environment* env = Environment::GetEnv();
  DelayedCallRWpin* delay = new DelayedCallRWpin;
  delay->type = DELAYEDCALL_READPIN;
  delay->data = rw_data;
  delay->afterrwpin = cb;
  delay->result = 0;

  uv_timer_init(env->loop(), &delay->timer);
  delay->timer.data = delay;
  uv_timer_start(&delay->timer, timerHandleTimeout, 1, 0);

  return 0;
}


} // namespace iotjs
