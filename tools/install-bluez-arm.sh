#!/bin/bash

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

# Get path.
DIR="$( unset CDPATH && cd "$( dirname "${BASH_SOURCE[0]}")" && pwd )"

# Make temporary folder.
mkdir bluez-arm
cd bluez-arm

# Install some packages.
sudo apt-get update -q
sudo apt-get install -q -y \
    g++-arm-linux-gnueabihf cmake libglib2.0-dev

# Download & install zlib for GLib.
wget http://zlib.net/zlib-1.2.11.tar.gz
tar -xzf zlib-1.2.11.tar.gz
cd zlib-1.2.11/
./configure --prefix=/usr/arm-linux-gnueabihf
patch -p0 < $DIR/zlib-Makefile.patch
make
sudo make install
cd ..

# Download & install libffi for GLib
wget ftp://sourceware.org/pub/libffi/libffi-3.0.13.tar.gz
tar -xzf libffi-3.0.13.tar.gz
cd libffi-3.0.13/
./configure --host=arm-linux-gnueabihf --prefix=/usr/arm-linux-gnueabihf \
    CC="arm-linux-gnueabihf-gcc"
make
sudo make install
cd ..

# Download & install GLib
wget http://ftp.gnome.org/pub/gnome/sources/glib/2.40/glib-2.40.0.tar.xz
tar -xJf glib-2.40.0.tar.xz
cd glib-2.40.0
./configure --host=arm-linux-gnueabihf --prefix=/usr/arm-linux-gnueabihf \
    CC="arm-linux-gnueabihf-gcc" \
    PKG_CONFIG_PATH=/usr/arm-linux-gnueabihf/lib/pkgconfig  \
    glib_cv_stack_grows=no glib_cv_uscore=yes ac_cv_func_posix_getpwuid_r=yes \
    ac_cv_func_posix_getgrgid_r=yes
make
sudo make install
cd ..

# Download & install Expat XML parser for D-BUS
wget http://sourceforge.net/projects/expat/files/expat/2.1.0/expat-2.1.0.tar.gz
tar -xzf expat-2.1.0.tar.gz
cd expat-2.1.0/
./configure --host=arm-linux-gnueabihf --prefix=/usr/arm-linux-gnueabihf \
    CC="arm-linux-gnueabihf-gcc"
make
sudo make install
cd ..

# Download & install D-Bus
wget http://dbus.freedesktop.org/releases/dbus/dbus-1.8.0.tar.gz
tar -xzf dbus-1.8.0.tar.gz
cd dbus-1.8.0/
./configure --host=arm-linux-gnueabihf --prefix=/usr/arm-linux-gnueabihf \
    CC="arm-linux-gnueabihf-gcc"
make
sudo make install
cd ..

# Download & install libical
wget http://downloads.sourceforge.net/freeassociation/libical-1.0.tar.gz
tar -xzf libical-1.0.tar.gz
cd libical-1.0/
export CC=arm-linux-gnueabihf-gcc
export CXX=arm-linux-gnueabihf-g++
cmake -DCMAKE_INSTALL_PREFIX=/usr/arm-linux-gnueabihf
make
sudo make install
cd ..

# Download & install ncurses for Readline
wget http://ftp.gnu.org/pub/gnu/ncurses/ncurses-5.9.tar.gz
tar -xzf ncurses-5.9.tar.gz
cd ncurses-5.9
./configure --host=arm-linux-gnueabihf --prefix=/usr/arm-linux-gnueabihf \
    CXX="arm-linux-gnueabihf-g++" CPPFLAGS="-fPIC"
make
sudo make install
cd ..

# Download & install Readline
wget ftp://ftp.gnu.org/pub/gnu/readline/readline-6.3.tar.gz
tar -xzf readline-6.3.tar.gz
cd readline-6.3/
./configure --host=arm-linux-gnueabihf --prefix=/usr/arm-linux-gnueabihf \
    bash_cv_wcwidth_broken=yes
make SHLIB_LIBS=-lncurses
sudo make install
cd ..

# Download & install Bluez
wget http://www.kernel.org/pub/linux/bluetooth/bluez-5.18.tar.xz
tar -xJf bluez-5.18.tar.xz
cd bluez-5.18
./configure --host=arm-linux-gnueabihf \
    --prefix= PKG_CONFIG_PATH=/usr/arm-linux-gnueabihf/lib/pkgconfig \
    --disable-systemd --disable-udev --disable-cups --disable-obex \
    --enable-library
make
sudo make install DESTDIR=/usr/arm-linux-gnueabihf
cd ..

# Remove temporary folder.
cd ..
rm -rf bluez-arm

