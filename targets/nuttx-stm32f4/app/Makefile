# Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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

############################################################################
#   Copyright (C) 2009, 2011-2013 Gregory Nutt. All rights reserved.
#   Author: Gregory Nutt <gnutt@nuttx.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Neither the name NuttX nor the names of its contributors may be
#    used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
############################################################################

# TODO, this makefile should run make under the app dirs, instead of
# sourcing the Make.defs!

-include $(TOPDIR)/.config
-include $(TOPDIR)/Make.defs
include $(APPDIR)/Make.defs


CONFIG_IOTJS_PRIORITY ?= SCHED_PRIORITY_DEFAULT
CONFIG_IOTJS_STACKSIZE ?= 16384
# NSH sysinfo command

APPNAME = iotjs
IOTJS_ROOT_DIR = ../../../iotjs
PRIORITY = $(CONFIG_IOTJS_PRIORITY)
STACKSIZE = $(CONFIG_IOTJS_STACKSIZE)
HEAPSIZE = $(CONFIG_IOTJS_HEAPSIZE)

ASRCS =
CSRCS =
CXXSRCS =
MAINSRC = iotjs_main.cxx
LIBS = libhttpparser.a libiotjs.a libjerrycore.a libtuv.a libjerry-libm.a libjerry-libc.a

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))
CXXOBJS = $(CXXSRCS:.cxx=$(OBJEXT))
MAINOBJ = $(MAINSRC:.cxx=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(CXXSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS) $(CXXOBJS)

ifeq ($(R),1)
  BUILD_TYPE = release
else
  BUILD_TYPE = debug
endif

ifneq ($(CONFIG_BUILD_KERNEL),y)
  OBJS += $(MAINOBJ)
endif

ifeq ($(CONFIG_WINDOWS_NATIVE),y)
  BIN = ..\..\libapps$(LIBEXT)
else
ifeq ($(WINTOOL),y)
  BIN = ..\\..\\libapps$(LIBEXT)
else
  BIN = ../../libapps$(LIBEXT)
endif
endif

ifeq ($(WINTOOL),y)
  INSTALL_DIR = "${shell cygpath -w $(BIN_DIR)}"
else
  INSTALL_DIR = $(BIN_DIR)
endif

CONFIG_XYZ_PROGNAME ?= iotjs$(EXEEXT)
PROGNAME = $(CONFIG_XYZ_PROGNAME)

ROOTDEPPATH = --dep-path .

# Common build

VPATH =

all: .built
.PHONY: context depend clean distclean

chkcxx:
ifneq ($(CONFIG_HAVE_CXX),y)
  @echo ""
  @echo "In order to use this example, you toolchain must support must"
  @echo ""
  @echo "  (1) Explicitly select CONFIG_HAVE_CXX to build in C++ support"
  @echo "  (2) Define CXX, CXXFLAGS, and COMPILEXX in the Make.defs file"
  @echo "      of the configuration that you are using."
  @echo ""
  @exit 1
endif


$(AOBJS): %$(OBJEXT): %.S
	$(call ASSEMBLE, $<, $@)

$(COBJS): %$(OBJEXT): %.c
	$(call COMPILE, $<, $@)

$(CXXOBJS) $(MAINOBJ): %$(OBJEXT): %.cxx
	$(call COMPILEXX, $<, $@)

copylibs :
	cp $(IOTJS_ROOT_DIR)/build/arm-nuttx/$(BUILD_TYPE)/lib/lib* .
	make -C $(IOTJS_ROOT_DIR)/build/arm-nuttx/$(BUILD_TYPE)/deps/jerry jerry-libm
	make -C $(IOTJS_ROOT_DIR)/build/arm-nuttx/$(BUILD_TYPE)/deps/jerry jerry-libc
	cp $(IOTJS_ROOT_DIR)/build/arm-nuttx/$(BUILD_TYPE)/deps/jerry/lib/libjerry-libc.a .
	cp $(IOTJS_ROOT_DIR)/build/arm-nuttx/$(BUILD_TYPE)/deps/jerry/lib/libjerry-libm.a .

$(LIBS) : copylibs
	$(firstword $(AR)) x $@

.built: chkcxx $(LIBS) $(OBJS)
	$(eval OBJS += $(shell find . -name "*.obj"))
	$(call ARCHIVE, $(BIN), $(OBJS))
	$(Q) touch .built

ifeq ($(CONFIG_BUILD_KERNEL),y)
$(BIN_DIR)$(DELIM)$(PROGNAME): $(OBJS) $(MAINOBJ)
	@echo "LD: $(PROGNAME)"
	$(Q) $(LD) $(LDELFFLAGS) $(LDLIBPATH) -o $(INSTALL_DIR)$(DELIM)$(PROGNAME) $(ARCHCRT0OBJ) $(MAINOBJ) $(LDLIBS)
	$(Q) $(NM) -u  $(INSTALL_DIR)$(DELIM)$(PROGNAME)

install: $(BIN_DIR)$(DELIM)$(PROGNAME)

else
install:

endif

# Register application

ifeq ($(CONFIG_NSH_BUILTIN_APPS),y)
$(BUILTIN_REGISTRY)$(DELIM)iotjs.bdat: $(DEPCONFIG) Makefile
	$(call REGISTER,"iotjs",$(PRIORITY),$(STACKSIZE),iotjs_main)

context: $(BUILTIN_REGISTRY)$(DELIM)iotjs.bdat
else
context:
endif

# Create dependencies

.depend: Makefile $(SRCS)
	$(Q) $(MKDEP) $(ROOTDEPPATH) "$(CXX)" -- $(CXXFLAGS) -- $(SRCS) >Make.dep
	$(Q) touch $@

depend: .depend

clean:
	$(eval OBJS += $(shell find . -name "*.obj"))
	$(call DELFILE, $(OBJS))
	$(call DELFILE, .built)
	$(call CLEAN)

distclean: clean
	$(call DELFILE, Make.dep)
	$(call DELFILE, .depend)

-include Make.dep
.PHONY: preconfig
preconfig:
