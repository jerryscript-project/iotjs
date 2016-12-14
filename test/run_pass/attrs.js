/* Copyright 2016-present Samsung Electronics Co., Ltd. and other contributors
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

(function() {
  var attrs = {
    'test_adc.js': {
      skip: ['all'],
      reason: "need to setup test environment"
    },
    'test_ble.js': {
      skip: ['all'],
      reason: "need to setup test environment"
     },
    'test_cwd.js': {
      skip: ['all'],
      reason: "this test had @STDOUT=COMMAND[pwd], but there's no way to " +
      "check out current directory path with js driver. So it skips " +
      "temporarily."
    },
    'test_dgram_address.js': {
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_dgram_broadcast.js': {
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_dgram_multicast_membership.js': {
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_dgram_multicast_set_multicast_loop.js': {
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_dns.js': {
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_events.js': {
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_exit.js': {
      skip: ['all'],
      reason:
        "driver can not run with test which checks out process.exit events"
    },
    'test_fs_exists.js': {
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_fs_existssync.js': {
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_fs_mkdir_rmdir.js': {
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_fs_open_close.js': {
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_fs_readfile.js': {
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_fs_readdir.js': {
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_fs_readfilesync.js': {
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_fs_stat.js': {
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_fs.js': {
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_fs1.js': {
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_fs2.js': {
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_gpio2.js': {
      skip: ['all'],
      reason: "need user input"
    },
    'test_i2c.js': {
      skip: ['all'],
      reason: "need to setup test environment"
    },
    'test_http_get.js': {
      timeout: {
        all: 20
      },
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_http_header.js': {
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_httpclient_timeout.js': {
      timeout: {
        all: 10
      },
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_httpclient_timeout2.js': {
      timeout: {
        all: 15
      },
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_httpserver_timeout.js': {
      timeout: {
        all: 10
      },
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_httpserver.js': {
      timeout: {
        all: 20
      },
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_module_cache.js': {
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_net3.js': {
      timeout: {
        all: 20
      },
      skip: ['nuttx'],
      reason: "too many socket descriptors, too long buffers are in need"
    },
    'test_net7.js': {
      skip: ['nuttx'],
      reason: "too many socket descriptors are in need"
    },
    'test_pwm.js': {
      skip: ['all'],
      reason: "need to setup test environment"
    },
    'test_timers.js': {
      timeout: {
        all: 10
      },
    },
    'test_uart.js': {
      skip: ['all'],
      reason: "need to setup test environment"
    },
    'test_uncaught1.js': {
      uncaught: true,
    },
    'test_uncaught2.js': {
      uncaught: true,
    },
    'test_uncaught_errpr1.js': {
      uncaught: true,
    },
    'test_uncaught_error2.js': {
      uncaught: true,
    },
  };

  return attrs;
})();
