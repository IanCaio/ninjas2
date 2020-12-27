#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#
# Modified by Clearly Broken Software
#

include dpf/Makefile.base.mk

all: libs plugins gen

define MISSING_SUBMODULES_ERROR

Cannot find DGL! Please run "make submodules" to clone the missing submodules, then retry building the plugin.

endef

# --------------------------------------------------------------
submodules: 
	git submodule update --init --recursive

libs:
ifeq (,$(wildcard dpf/dgl))
	$(error $(MISSING_SUBMODULES_ERROR))
endif

ifneq ($(USE_SYSTEM_AUBIO),true)
	$(MAKE) -C aubio
endif

ifeq ($(HAVE_OPENGL),true)
	$(MAKE) -C dpf/dgl
endif

plugins: libs
	$(MAKE) all -C plugins/Ninjas2

gen: plugins dpf/utils/lv2_ttl_generator
	@$(CURDIR)/dpf/utils/generate-ttl.sh
ifeq ($(MACOS),true)
	@$(CURDIR)/dpf/utils/generate-vst-bundles.sh
endif

dpf/utils/lv2_ttl_generator:
	$(MAKE) -C dpf/utils/lv2-ttl-generator

# --------------------------------------------------------------

clean:
ifneq ($(USE_SYSTEM_AUBIO),true)
	$(MAKE) clean -C aubio
endif
ifeq ($(HAVE_OPENGL),true)
	$(MAKE) clean -C dpf/dgl
endif
	$(MAKE) clean -C dpf/utils/lv2-ttl-generator
	$(MAKE) clean -C plugins/Ninjas2

# --------------------------------------------------------------

install:
	install -d $(DESTDIR)$(PREFIX)/lib/lv2/
	install -d $(DESTDIR)$(PREFIX)/lib/vst/
	install -d $(DESTDIR)$(PREFIX)/bin/

ifeq ($(MACOS),true)
	cp -r bin/*.vst            $(DESTDIR)$(PREFIX)/lib/vst/
else
	install -m 644 bin/*-vst.* $(DESTDIR)$(PREFIX)/lib/vst/
endif
	cp -r bin/*.lv2            $(DESTDIR)$(PREFIX)/lib/lv2/

# --------------------------------------------------------------

.PHONY: plugins

