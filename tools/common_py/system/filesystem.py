# Copyright 2016-present Samsung Electronics Co., Ltd. and other contributors
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

# Copyright (C) 2010 Google Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimecd r.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

""" Wrapper object for the file system """


import codecs
import errno
import filecmp
import glob
import hashlib
import os
import shutil
import sys
import tempfile
import time
try:
    import exceptions
except ImportError:
    class exceptions(object):
        OSError = OSError


class FileSystem(object):
    """FileSystem interface for IoT.js.

    Unless otherwise noted, all paths are allowed to be either absolute
    or relative."""
    sep = os.sep
    pardir = os.pardir

    @staticmethod
    def abspath(path):
        # FIXME: This gross hack is needed while we transition from Cygwin
        # to native Windows, because we have some mixing of file conventions
        # from different tools:
        if sys.platform == 'cygwin':
            path = os.path.normpath(path)
            path_components = path.split(os.sep)
            if (path_components and len(path_components[0]) == 2
                    and path_components[0][1] == ':'):
                path_components[0] = path_components[0][0]
                path = os.path.join('/', 'cygdrive', *path_components)

        return os.path.abspath(path)

    @staticmethod
    def realpath(path):
        return os.path.realpath(path)

    @staticmethod
    def path_to_module(module_name):
        """A wrapper for all calls to __file__ to allow easy unit testing."""
        # FIXME: This is the only use of sys in this file. It's possible this
        # function should move elsewhere.
        # __file__ is always an absolute path.
        return sys.modules[module_name].__file__

    @staticmethod
    def expanduser(path):
        return os.path.expanduser(path)

    @staticmethod
    def basename(path):
        return os.path.basename(path)

    @staticmethod
    def chdir(path):
        return os.chdir(path)

    @staticmethod
    def copy(source, destination):
        shutil.copy(source, destination)

    @staticmethod
    def copyfile(source, destination):
        shutil.copyfile(source, destination)

    @staticmethod
    def dirname(path):
        return os.path.dirname(path)

    @staticmethod
    def exists(path):
        return os.path.exists(path)

    @staticmethod
    def dirs_under(path, dir_filter=None):
        """Return the list of all directories under the given path in
        topdown order.

        Args:
            dir_filter: if not None, the filter will be invoked
                with the filesystem object and the path of each dirfound.
                The dir is included in the result if the callback returns True.
        """
        def filter_all(dirpath):
            return True
        dir_filter = dir_filter or filter_all

        dirs = []
        for (dirpath, dirnames, filenames) in os.walk(path):
            if dir_filter(dirpath):
                dirs.append(dirpath)
        return dirs

    @staticmethod
    def files_under(path, dirs_to_skip=[], file_filter=None):
        """Return the list of all files under the given path in topdown order.

        Args:
            dirs_to_skip: a list of directories to skip over during the
                traversal (e.g., .svn, resources, etc.)
            file_filter: if not None, the filter will be invoked
                with the filesystem object and the dirname and basename of
                each file found. The file is included in the result if the
                callback returns True.
        """
        def filter_all(dirpath, basename):
            return True

        file_filter = file_filter or filter_all
        files = []
        if FileSystem.isfile(path):
            if file_filter(dirname(path), FileSystem.basename(path)):
                files.append(path)
            return files

        if FileSystem.basename(path) in dirs_to_skip:
            return []

        for (dirpath, dirnames, filenames) in os.walk(path):
            for d in dirs_to_skip:
                if d in dirnames:
                    dirnames.remove(d)

            for filename in filenames:
                if file_filter(dirpath, filename):
                    files.append(FileSystem.join(dirpath, filename))
        return files

    @staticmethod
    def getcwd():
        return os.getcwd()

    @staticmethod
    def glob(path):
        return glob.glob(path)

    @staticmethod
    def isabs(path):
        return os.path.isabs(path)

    @staticmethod
    def isfile(path):
        return os.path.isfile(path)

    @staticmethod
    def getsize(path):
        return os.path.getsize(path)

    @staticmethod
    def isdir(path):
        return os.path.isdir(path)

    @staticmethod
    def join(*comps):
        return os.path.join(*comps)

    @staticmethod
    def listdir(path):
        return os.listdir(path)

    @staticmethod
    def mkdtemp(**kwargs):
        """Create and return a uniquely named directory.

        This is like tempfile.mkdtemp, but if used in a with statement
        the directory will self-delete at the end of the block (if the
        directory is empty; non-empty directories raise errors). The
        directory can be safely deleted inside the block as well, if so
        desired.

        Note that the object returned is not a string and does not support
        all of the string methods. If you need a string, coerce the object
        to a string and go from there.
        """
        class TemporaryDirectory(object):
            def __init__(self, **kwargs):
                self._kwargs = kwargs
                self._directory_path = tempfile.mkdtemp(**self._kwargs)

            def __str__(self):
                return self._directory_path

            def __enter__(self):
                return self._directory_path

            def __exit__(self, type, value, traceback):
                # Only self-delete if necessary.

                # FIXME: Should we delete non-empty directories?
                if os.path.exists(self._directory_path):
                    os.rmdir(self._directory_path)

        return TemporaryDirectory(**kwargs)

    @staticmethod
    def maybe_make_directory(*path):
        """Create the specified directory if it doesn't already exist."""
        try:
            os.makedirs(FileSystem.join(*path))
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise

    @staticmethod
    def move(source, destination):
        shutil.move(source, destination)

    @staticmethod
    def mtime(path):
        return os.stat(path).st_mtime

    @staticmethod
    def normpath(path):
        return os.path.normpath(path)

    @staticmethod
    def open_binary_tempfile(suffix):
        """Create, open, and return a binary temp file. Returns a tuple of
        the file and the name."""
        temp_fd, temp_name = tempfile.mkstemp(suffix)
        f = os.fdopen(temp_fd, 'wb')
        return f, temp_name

    @staticmethod
    def open_binary_file_for_reading(path):
        return codecs.open(path, 'rb')

    @staticmethod
    def read_binary_file(path):
        """Return the contents of the file at the given path as a
        byte string."""
        with file(path, 'rb') as f:
            return f.read()

    @staticmethod
    def write_binary_file(path, contents):
        with file(path, 'wb') as f:
            f.write(contents)

    @staticmethod
    def open_text_file_for_reading(path, errors='strict'):
        # Note: There appears to be an issue with the returned file objects
        # not being seekable. See http://stackoverflow.com/questions/1510188/
        # can-seek-and-tell-work-with-utf-8-encoded-documents-in-python .
        return codecs.open(path, 'r', 'utf8', errors)

    @staticmethod
    def open_text_file_for_writing(path):
        return codecs.open(path, 'w', 'utf8')

    @staticmethod
    def open_stdin():
        return codecs.StreamReaderWriter(sys.stdin,
                                         codecs.getreader('utf8'),
                                         codecs.getwriter('utf8'),
                                         'replace')

    @staticmethod
    def read_text_file(path):
        """Return the contents of the file at the given path as
        a Unicode string.

        The file is read assuming it is a UTF-8 encoded file with no BOM."""
        with codecs.open(path, 'r', 'utf8') as f:
            return f.read()

    @staticmethod
    def write_text_file(path, contents):
        """Write the contents to the file at the given location.

        The file is written encoded as UTF-8 with no BOM."""
        with codecs.open(path, 'w', 'utf-8') as f:
            f.write(contents.decode('utf-8') if type(contents) == str
                    else contents)

    @staticmethod
    def sha1(path):
        contents = FileSystem.read_binary_file(path)
        return hashlib.sha1(contents).hexdigest()

    @staticmethod
    def relpath(path, start='.'):
        return os.path.relpath(path, start)

    class _WindowsError(exceptions.OSError):
        """Fake exception for Linux and Mac."""
        pass

    @staticmethod
    def remove(path, osremove=os.remove):
        """On Windows, if a process was recently killed and it held on to a
        file, the OS will hold on to the file for a short while.  This makes
        attempts to delete the file fail.  To work around that, this method
        will retry for a few seconds until Windows is done with the file."""
        try:
            exceptions.WindowsError
        except AttributeError:
            exceptions.WindowsError = FileSystem._WindowsError

        retry_timeout_sec = 3.0
        sleep_interval = 0.1
        while True:
            try:
                osremove(path)
                return True
            except exceptions.WindowsError as e:
                time.sleep(sleep_interval)
                retry_timeout_sec -= sleep_interval
                if retry_timeout_sec < 0:
                    raise e

    @staticmethod
    def rmtree(path):
        """Delete the directory rooted at path, whether empty or not."""
        shutil.rmtree(path, ignore_errors=True)

    @staticmethod
    def copytree(source, destination):
        shutil.copytree(source, destination)

    @staticmethod
    def split(path):
        """Return (dirname, basename + '.' + ext)"""
        return os.path.split(path)

    @staticmethod
    def splitext(path):
        """Return (dirname + os.sep + basename, '.' + ext)"""
        return os.path.splitext(path)

    @staticmethod
    def compare(path1, path2):
        return filecmp.cmp(path1, path2)
