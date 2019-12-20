#!/usr/bin/make -f
# SPDX-License-Identifier: MIT
# -*- makefile -*-
#
# Minimal red-black-tree helper functions test
#
# SPDX-FileCopyrightText: 2012-2016, Sven Eckelmann <sven@narfation.org>

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
	Q_LD = @echo '    $(COMPILER_NAME)' $@;
	export Q_CC
	export Q_LD
endif
endif

# standard build tools
CC ?= gcc
RM ?= rm -f
ifeq ("$(BUILD_CXX)", "1")
COMPILE.c = $(Q_CC)$(CXX) -x c++ $(CFLAGS) $(CXXLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
LINK.o = $(Q_LD)$(CXX) $(CFLAGS) $(CXXLAGS) $(LDFLAGS) $(TARGET_ARCH)
else
COMPILE.c = $(Q_CC)$(CC) -x c $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
LINK.o = $(Q_LD)$(CC) $(CFLAGS) $(LDFLAGS) $(TARGET_ARCH)
endif

TESTS_OK = $(TESTS:=.ok)

# default target
all: $(TESTS_OK)

$(TESTS_OK): %.ok: %
	@echo "T:  $(COMPILER_NAME) $(TESTRUN_NAME) $<"
	@$(TESTRUN_WRAPPER) ./$<
	@touch $@

# standard build rules
.SUFFIXES: .o .c
.c.o:
	$(COMPILE.c) -o $@ $<

rbtree.o: ../rbtree.c
	$(COMPILE.c) -o $@ $<

$(TESTS): %: %.o rbtree.o
	$(LINK.o) $^ $(LDLIBS) -o $@

clean:
	@$(RM) $(TESTS_ALL) $(DEP) $(TESTS_OK) $(TESTS:=.o) $(TESTS:=.d) rbtree.o rbtree.d

# load dependencies
DEP = $(TESTS:=.d) rbtree.d
-include $(DEP)

.PHONY: all clean
