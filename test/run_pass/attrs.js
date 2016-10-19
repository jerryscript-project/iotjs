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

(function() {
  var attrs = {
    'test_cwd.js': {
      skip: ['all'],
      reason: "this test had @STDOUT=COMMAND[pwd], but there's no way to " +
      "check out current directory path with js driver. So it skips " +
      "temoporalily."
    },
    'test_dns.js': {
      exit: true,
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
      exit: true,
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_fs_readfile.js': {
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
    'test_gpio1.js': {
      skip: ['all'],
      reason: "not implemented"
    },
    'test_gpio2.js': {
      skip: ['all'],
      reason: "not implemented"
    },
    'test_gpio3.js': {
      skip: ['all'],
      reason: "not implemented"
    },
    'test_http_get.js': {
      timeout: {
        all: 20
      },
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_http_header.js': {
      exit: true,
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_httpclient_timeout.js': {
      exit: true,
      timeout: {
        all: 10
      },
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_httpclient_timeout2.js': {
      exit: true,
      timeout: {
        all: 15
      },
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_httpserver_timeout.js': {
      exit: true,
      timeout: {
        all: 10
      },
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_httpserver.js': {
      exit: true,
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
    'test_net1.js': {
      exit: true,
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_net2.js': {
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_net3.js': {
      exit: true,
      timeout: {
        all: 20
      },
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_net4.js': {
      exit: true,
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_net5.js': {
      exit: true,
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_net6.js': {
      exit: true,
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_net7.js': {
      exit: true,
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_net8.js': {
      exit: true,
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_net9.js': {
      exit: true,
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_net10.js': {
      exit: true,
      skip: ['nuttx'],
      reason: "not implemented for nuttx"
    },
    'test_next_tick.js': {
      skip: ['all'],
      reason:
        "driver can not run with test which checks out process.nextTick events"
    },
    'test_timers.js': {
      exit: true,
      timeout: {
        all: 10
      },
    },
    'test_timers2.js': {
      exit: true
    },
    'test_uncaught1.js': {
      uncaught: true,
      exit: true
    },
    'test_uncaught2.js': {
      uncaught: true,
      exit: true
    },
    'test_uncaught_errpr1.js': {
      uncaught: true,
      exit: true
    },
    'test_uncaught_error2.js': {
      uncaught: true,
      exit: true
    },
  };

  return attrs;
})();
