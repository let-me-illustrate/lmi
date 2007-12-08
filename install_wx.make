# Install msw-native wx with options suitable for lmi.
#
# Copyright (C) 2006, 2007 Gregory W. Chicares.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
#
# http://savannah.nongnu.org/projects/lmi
# email: <chicares@cox.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

# $Id: install_wx.make,v 1.5 2007-12-08 03:13:25 chicares Exp $

# Configurable settings ########################################################

wx_version    := 2.8.6

mingw_dir     := /MinGW_

prefix        := /opt/lmi/local

cache_dir     := /tmp/lmi_cache

wx_dir        := /opt/lmi/wx-scratch

# Variables that normally should be left alone #################################

date           = $(shell date -u +'%Y%m%dT%H%MZ')

mingw_bin_dir := $(mingw_dir)/bin

vendor        := $(shell $(mingw_bin_dir)/gcc -dumpversion)
vendor        := $(subst .,,$(vendor))

build_dir     := $(wx_dir)/wxWidgets-$(wx_version)/gcc$(vendor)

ifeq (3.81,$(firstword $(sort $(MAKE_VERSION) 3.81)))
  this_makefile := $(abspath $(lastword $(MAKEFILE_LIST)))
else
  $(error Upgrade to make-3.81 .)
endif

# Configuration reference:
#   http://lists.nongnu.org/archive/html/lmi/2007-11/msg00001.html
# SOMEDAY !! But see the last paragraph of
#   http://lists.nongnu.org/archive/html/lmi/2007-11/msg00004.html
# and override wxApp::OnAssertFailure() before experimenting with
# '--enable-debug_flag'.

config_options = \
  --disable-gif \
  --disable-apple_ieee \
  --without-libjpeg \
  --without-libtiff \
  --without-regex \
  --enable-monolithic \
  --enable-shared \
  --disable-threads \
  --enable-stl \
  --enable-std_iostreams \
  --enable-commondlg \
  --disable-compat24 \
  --disable-vararg_macros \
  --prefix=$(prefix) \
  VENDOR='$(vendor)' \
  CPPFLAGS='-DNO_GCC_PRAGMA' \
  CC='$(mingw_bin_dir)/gcc' \
  CXX='$(mingw_bin_dir)/g++' \

# URLs and archive md5sums #####################################################

wx-2.8.6-md5 := 8a130e5b25448a17454a6b957a5e075c

wx_archive        := wxWidgets-$(wx_version).tar.bz2

$(wx_archive)-url := ftp://ftp.wxwidgets.org/pub/$(wx_version)/$(wx_archive)

$(wx_archive)-md5 := $(wx-$(wx_version)-md5)

# Utilities ####################################################################

CP     := cp
ECHO   := echo
MD5SUM := md5sum
MKDIR  := mkdir
RM     := rm
TAR    := tar
WGET   := wget

# Targets ######################################################################

source_archives := $(wx_archive)
libraries       := $(source_archives:.tar.bz2=)

.PHONY: all
all: clobber $(source_archives) $(libraries)
	$(MAKE) --file=$(this_makefile) --directory=$(build_dir) wx
	$(MAKE) --file=$(this_makefile) --directory=$(prefix)/bin portable_script

# Simulated order-only prerequisites.
$(libraries): $(source_archives)
$(source_archives): initial_setup
initial_setup: clobber

.PHONY: initial_setup
initial_setup:
	@$(MKDIR) --parents $(prefix)
	@$(MKDIR) --parents $(cache_dir)
	@$(MKDIR) --parents $(build_dir)

TARFLAGS := --keep-old-files
%.tar.bz2: TARFLAGS += --bzip2
%.tar.gz:  TARFLAGS += --gzip

WGETFLAGS := '--timestamping'

.PHONY: %.tar.bz2 %.tar.gz
%.tar.bz2 %.tar.gz:
	cd $(cache_dir) && [ -e $@ ] || $(WGET) $(WGETFLAGS) $($@-url)
	cd $(cache_dir) && $(ECHO) "$($@-md5) *$@" | $(MD5SUM) --check
	-$(TAR) --extract $(TARFLAGS) --directory=$(wx_dir) --file=$(cache_dir)/$@

.PHONY: wx
wx:
	../configure $(config_options) && $(MAKE) && $(MAKE) install

# 'wx-config' is not portable. For example, it uses 'printf(1)', which
# zsh supports only in versions after 4.0.1 . Far worse, it underlies
# a problem discussed in these messages
#   http://lists.gnu.org/archive/html/lmi/2006-04/msg00010.html
#   http://lists.gnu.org/archive/html/lmi/2006-05/msg00001.html
#   http://lists.gnu.org/archive/html/lmi/2006-05/msg00019.html
#   http://lists.gnu.org/archive/html/lmi/2006-05/msg00021.html
# and extensive offline discussions, which has consumed person-weeks
# of our time; though we can't pinpoint the exact cause, we have never
# encountered any such problem except with 'wx-config'. Therefore, we
# run 'wx-config' only here (in bash, in the present makefile) and
# write the results of the only two commands we actually need:
#   wx-config --cxxflags
#   wx-config --libs
# into a portable script.
#
# Even if a forgiving shell is used, this portable script runs an
# order of magnitude faster than the one wx creates.
# WX !! Is any useful advantage lost?

.PHONY: portable_script
portable_script:
	$(ECHO) '#!/bin/sh'                          >wx-config-portable
	$(ECHO) 'if   [ "--cxxflags" = $$1 ]; then' >>wx-config-portable
	$(ECHO) "echo `./wx-config --cxxflags`"     >>wx-config-portable
	$(ECHO) 'elif [ "--libs"     = $$1 ]; then' >>wx-config-portable
	$(ECHO) "echo `./wx-config --libs`"         >>wx-config-portable
	$(ECHO) 'else'                              >>wx-config-portable
	$(ECHO) 'echo Bad argument $$1'             >>wx-config-portable
	$(ECHO) 'fi'                                >>wx-config-portable

.PHONY: clobber
clobber:
# WX !! The 'uninstall' target doesn't remove quite everything.
	-cd $(build_dir) && $(MAKE) uninstall distclean
	-$(RM) --force --recursive $(prefix)/include/wx-$(basename $(wx_version))
	-$(RM) --force --recursive $(prefix)/lib/wx
	-$(RM) --force --recursive $(wx_dir)

