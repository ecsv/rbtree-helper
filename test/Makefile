#!/usr/bin/make -f
# -*- makefile -*-
#
# Minimal red-black-tree helper functions test
#
# Copyright (c) 2012-2014, Sven Eckelmann <sven@narfation.org>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

TESTS_CXX_COMPATIBLE = \
 containerof \
 rb_entry \
 rb_init-explicit \
 rb_init-local \
 rb_init-global \
 rb_link_node \
 rb_erase_node \
 rb_first \
 rb_last \
 rb_next \
 rb_prev \
 rb_link_node-prioqueue \
 rb_erase_node-prioqueue \
 rb_insert_color \
 rb_erase \
 rb_insert-prioqueue \
 rb_erase-prioqueue \

TESTS_C_ONLY = \

TESTS_ALL = $(TESTS_CXX_COMPATIBLE) $(TESTS_C_ONLY)

# tests flags and options
CFLAGS += -g3 -pedantic -Wall -W -Werror -MD -MP
ifeq ("$(BUILD_CXX)", "1")
	CFLAGS += -std=c++98
	TESTS = $(TESTS_CXX_COMPATIBLE)
	COMPILER_NAME=$(CXX)
else
	CFLAGS += -std=c99
	TESTS += $(TESTS_ALL)
	COMPILER_NAME=$(CC)
endif

# disable verbose output
ifneq ($(findstring $(MAKEFLAGS),s),s)
ifndef V
	Q_CC = @echo '    $(COMPILER_NAME)' $@;
	export Q_CC
endif
endif

# standard build tools
CC ?= gcc
RM ?= rm -f
LINK.c = $(Q_CC)$(CC) -x c $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)
LINK.cc = $(Q_CC)$(CXX) -x c++ $(CFLAGS) $(CXXLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)

TESTS_OK = $(TESTS:=.ok)

# default target
all: $(TESTS_OK)

$(TESTS_OK): %.ok: %
	@echo "T:  $(COMPILER_NAME) $(TESTRUN_NAME) $<"
	@$(TESTRUN_WRAPPER) ./$<
	@touch $@

# standard build rules
.SUFFIXES: .c
.c:
ifeq ("$(BUILD_CXX)", "1")
	$(LINK.cc) $< $(LDLIBS) -o $@
else
	$(LINK.c) $< $(LDLIBS) -o $@
endif

clean:
	@$(RM) $(TESTS_ALL) $(DEP) $(TESTS_OK)

# load dependencies
DEP = $(TESTS:=.d)
-include $(DEP)

.PHONY: all clean
