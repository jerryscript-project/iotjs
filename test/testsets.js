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

function testsets() {
  var tests = {
    'run_pass': [
      'test_adc.js',
      'test_assert.js',
      'test_ble.js',
      'test_buffer_builtin.js',
      'test_buffer.js',
      'test_console.js',
      'test_cwd.js',
      'test_dgram_1_server_1_client.js',
      'test_dgram_1_server_n_clients.js',
      'test_dgram_address.js',
      'test_dgram_broadcast.js',
      'test_dgram_multicast_membership.js',
      'test_dgram_multicast_set_multicast_loop.js',
      'test_dns.js',
      'test_events.js',
      'test_exit.js',
      'test_fs_exists.js',
      'test_fs_existssync.js',
      'test_fs_mkdir_rmdir.js',
      'test_fs_open_close.js',
      'test_fs_readfile.js',
      'test_fs_readfilesync.js',
      'test_fs_readdir.js',
      'test_fs_rename.js',
      'test_fs_rename_sync.js',
      'test_fs_stat.js',
      'test_fs_writefile_unlink.js',
      'test_fs_writefile_unlink_sync.js',
      'test_fs.js',
      'test_fs1.js',
      'test_fs2.js',
      'test_gpio1.js',
      'test_gpio2.js',
      'test_http_get.js',
      'test_http_header.js',
      'test_httpclient_timeout.js',
      'test_httpclient_timeout2.js',
      'test_httpserver_timeout.js',
      'test_httpserver.js',
      'test_i2c.js',
      'test_module_cache.js',
      'test_net1.js',
      'test_net2.js',
      'test_net3.js',
      'test_net4.js',
      'test_net5.js',
      'test_net6.js',
      'test_net7.js',
      'test_net8.js',
      'test_net9.js',
      'test_net10.js',
      'test_next_tick.js',
      'test_process.js',
      'test_pwm.js',
      'test_stream.js',
      'test_timers.js',
      'test_timers2.js',
      'test_uart.js',
      'test_uncaught1.js',
      'test_uncaught2.js',
      'test_uncaught_error1.js',
      'test_uncaught_error2.js',
      ],
      'run_pass/issue': [
        'issue-133.js',
        'issue-137.js',
        'issue-198.js',
        'issue-223.js',
        'issue-266.js',
        'issue-323.js',
      ],
      'run_fail': [
        'assert_fail1.js',
        'assert_fail2.js',
        'assert_fail3.js',
        'process_exit1.js',
        'process_exit2.js',
        'process_exitcode1.js',
        'process_exitcode2.js',
        'require_fail1.js',
        'runtime_error1.js',
        'syntax_fail1.js',
        'uncaught_error_event.js',
      ]
  };

  return tests;
}

module.exports = testsets;
