###########################################################################
#
# Copyright 2016 Samsung Electronics All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
# either express or implied. See the License for the specific
# language governing permissions and limitations under the License.
#
###########################################################################
############################################################################
# apps/examples/iotjs/Makefile
#
#   Copyright (C) 2008, 2010-2013 Gregory Nutt. All rights reserved.
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

EXTRA_LIBS += libhttpparser.a libiotjs.a libjerrycore.a libtuv.a libjerry-libm.a
LINKLIBS=$(EXTRA_LIBS)

-include $(TOPDIR)/.config
-include $(TOPDIR)/Make.defs
include $(APPDIR)/Make.defs

ifeq ($(IOTJS_ROOT_DIR),)
  IOTJS_ROOT_DIR = ../../iotjs
endif
IOTJS_ABSOLUTE_ROOT_DIR := $(shell cd $(TOPDIR) && cd $(IOTJS_ROOT_DIR) && pwd)

ifeq ($(IOTJS_LIB_DIR),)
  ifeq ($(CONFIG_DEBUG),y)
    IOTJS_LIB_DIR = $(IOTJS_ABSOLUTE_ROOT_DIR)/build/arm-tizenrt/debug/lib
  else
    IOTJS_LIB_DIR = $(IOTJS_ABSOLUTE_ROOT_DIR)/build/arm-tizenrt/release/lib
  endif
endif
EXTRA_LIBPATHS += -L$(IOTJS_LIB_DIR)


# IoT.js application
CONFIG_IOTJS_PRIORITY ?= SCHED_PRIORITY_DEFAULT
CONFIG_IOTJS_STACKSIZE ?= 16384
IOTJS_LIB_DIR ?= n

APPNAME = iotjs
CFLAGS += -I$(IOTJS_ABSOLUTE_ROOT_DIR)/deps/jerry/jerry-core/include
CFLAGS += -I$(IOTJS_ABSOLUTE_ROOT_DIR)/deps/jerry/jerry-ext/include
PRIORITY = $(CONFIG_IOTJS_PRIORITY)
STACKSIZE = $(CONFIG_IOTJS_STACKSIZE)
HEAPSIZE = $(CONFIG_IOTJS_HEAPSIZE)

ASRCS =
CSRCS = jerry_port.c
MAINSRC = iotjs_main.c

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))
MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS)


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

CONFIG_IOTJS_PROGNAME ?= iotjs$(EXEEXT)
PROGNAME = $(CONFIG_IOTJS_PROGNAME)

ROOTDEPPATH = --dep-path .

# Common build

VPATH =

all: .built
.PHONY: clean depend distclean check_iotjs

$(AOBJS): %$(OBJEXT): %.S
	$(call ASSEMBLE, $<, $@)

$(COBJS) $(MAINOBJ): %$(OBJEXT): %.c
	$(call COMPILE, $<, $@)

.built: $(OBJS) check_iotjs
	$(call ARCHIVE, $(BIN), $(OBJS))
	@touch .built

ifeq ($(CONFIG_BUILD_KERNEL),y)
$(BIN_DIR)$(DELIM)$(PROGNAME): $(OBJS) $(MAINOBJ) check_iotjs
	$(Q) $(LD) $(LDELFFLAGS) $(LDLIBPATH) -o $(INSTALL_DIR)$(DELIM)$(PROGNAME) $(ARCHCRT0OBJ) $(MAINOBJ) $(LDLIBS)
	$(Q) $(NM) -u  $(INSTALL_DIR)$(DELIM)$(PROGNAME)

install: $(BIN_DIR)$(DELIM)$(PROGNAME)

else
install:

endif

check_iotjs:
ifeq ($(IOTJS_LIB_DIR),n)
	@echo "ERROR: IOTJS_LIB_DIR not set! Aborting..."
	@exit 1
endif
	@echo IOTJS_LIB_DIR set!
	@echo "$(LDLIBPATH), $(IOTJS_LIB_DIR) $(TOPDIR)"
	@cp $(IOTJS_LIB_DIR)/lib* $(TOPDIR)/../build/output/libraries/
	@cp $(IOTJS_LIB_DIR)/../deps/jerry/lib/libjerry-libm.a $(TOPDIR)/../build/output/libraries/

context:

.depend: Makefile $(SRCS)
	@$(MKDEP) $(ROOTDEPPATH) "$(CC)" -- $(CFLAGS) -- $(SRCS) >Make.dep
	@touch $@

depend: .depend

clean:
	$(call DELFILE, .built)
	$(call CLEAN)

distclean: clean
	$(call DELFILE, Make.dep)
	$(call DELFILE, .depend)

-include Make.dep
.PHONY: preconfig
preconfig:
