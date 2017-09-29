# Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import paramiko
import socket

from common_py.system.executor import TimeoutException


class Connection(object):
    """
    The serial communication wrapper.
    """
    def __init__(self, username, address, timeout):
        self.username = username
        self.address = address
        self.timeout = timeout

        # Note: add your SSH key to the known host file
        # to avoid getting password.
        self.ssh = paramiko.client.SSHClient()
        self.ssh.load_system_host_keys()
        self.ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())

    def open(self):
        """
        Open the serial port.
        """
        self.ssh.connect(self.address, username=self.username)

    def close(self):
        """
        Close the serial port.
        """
        self.ssh.close()

    def exec_command(self, cmd):
        """
        Send command over the serial port.
        """
        try:
            _, stdout, _ = self.ssh.exec_command(cmd, timeout=self.timeout)

            return stdout.readline()

        except socket.timeout:
            raise TimeoutException

    def send_file(self, lpath, rpath):
        """
        Send file over the Secure File Transfer Protocol.
        """
        sftp = self.ssh.open_sftp()

        sftp.put(lpath, rpath)
        sftp.close()
