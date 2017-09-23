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

import serial
import time
import xmodem

from common_py.system.executor import TimeoutException


class Connection(object):
    """
    The serial communication wrapper.
    """
    def __init__(self, port, baud, timeout, prompt):
        self.port = port
        self.baud = baud
        self.timeout = timeout

        # Defines the end of the stdout.
        self.prompt = prompt

    def get_prompt(self):
        """
        Get the prompt.
        """
        return self.prompt

    def open(self):
        """
        Open the serial port.
        """
        self.serial = serial.Serial(self.port, self.baud, timeout=self.timeout)
        self.xmodem = xmodem.XMODEM1k(self.getc, self.putc)

    def close(self):
        """
        Close the serial port.
        """
        self.serial.close()

    def getc(self, size, timeout=1):
        """
        Recevice data from the serial port.
        """
        time.sleep(2)

        return self.serial.read(size) or None

    def putc(self, data, timeout=1):
        """
        Send data to the serial port.
        """
        return self.serial.write(data)

    def readline(self):
        """
        Read line from the serial port.
        """
        return self.serial.readline()

    def exec_command(self, cmd):
        """
        Send command over the serial port.
        """
        self.serial.write(cmd + "\n")

        receive = self.serial.read_until(self.prompt)

        # Throw exception when timeout happens.
        if self.prompt not in receive:
            raise TimeoutException

        # Note: since the received data format is
        #
        #   [command][CRLF][stdout][CRFL][prompt],
        #
        # we should process the output for the stdout.
        return "\n".join(receive.split("\r\n")[1:-1])

    def read_until(self, *args):
        """
        Read data until it contains args.
        """
        line = bytearray()
        while True:
            c = self.serial.read(1)
            if c:
                line += c
                for stdout in args:
                    if line[-len(stdout):] == stdout:
                        return stdout, bytes(line)
            else:
                # raise utils.TimeoutException
                raise Exception("use TimeoutException")

        return False, False

    def send_file(self, lpath, rpath):
        """
        Send file over the serial port.
        Note: `lrzsz` package should be installed on the device.
        """
        self.serial.write("rm " + rpath + "\n")
        self.serial.write("rx " + rpath + "\n")

        # Receive all the data from the device except the last
        # \x15 (NAK) byte that is needed by the xmodem protocol.
        self.serial.read(self.serial.in_waiting - 1)

        with open(lpath, "rb") as file:
            self.xmodem.send(file)
