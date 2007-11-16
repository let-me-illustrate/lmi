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

# $Id: install_wx.make,v 1.7 2007-11-16 01:52:30 chicares Exp $

# Configurable settings ########################################################

mingw_root      := /cygdrive/c
mingw_dir       := MinGW-20050827

prefix          := /opt/lmi/local

# TODO ?? wget...

wx_dir          := /opt/lmi/wx-scratch/wxWidgets-2006-12-04

# Variables that normally should be left alone #################################

date           = $(shell date -u +'%Y%m%dT%H%MZ')

mingw_bin_dir := $(mingw_root)/$(mingw_dir)/bin

vendor        := $(shell $(mingw_bin_dir)/gcc -dumpversion)
vendor        := $(subst .,,$(vendor))

build_dir     := $(wx_dir)/gcc$(vendor)

ifeq (3.81,$(firstword $(sort $(MAKE_VERSION) 3.81)))
  this_makefile := $(abspath $(lastword $(MAKEFILE_LIST)))
else
  $(error Upgrade to make-3.81 .)
endif

# Configuration reference:
#   http://lists.nongnu.org/archive/html/lmi/2007-11/msg00001.html

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
  --enable-debug \
  --enable-commondlg \
  --disable-compat24 \
  --disable-vararg_macros \
  VENDOR='$(vendor)' \
  CPPFLAGS='-DNO_GCC_PRAGMA' \
  CC='$(mingw_bin_dir)/gcc' \
  CXX='$(mingw_bin_dir)/g++' \

# Utilities ####################################################################

CP     := cp
ECHO   := echo
MKDIR  := mkdir
SED    := sed
TAR    := tar
TR     := tr
WGET   := wget

# Portability workaround #######################################################

# TODO ?? Either expunge this, or provide a rationale for keeping it.

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
# When wx is built with MSYS, 'wx-config' hardcodes MSYS-specific
# paths like "/c/wxWidgets/...", which are not usable outside of MSYS.
# Apparently the tidiest way to address that problem is to modify
# 'wx-config' to use the MSYS `cd some/directory && pwd -W` idiom,
# and run only that modified script. WX !! Propose this change for
# inclusion in the wx sources.
#
# The 'tr' call wouldn't be necessary if the MSYS 'sed' port could be
# relied upon never to use carriage returns in line endings.

##wx_config_fix = \
##  <$(1) $(SED) -e 's|pwd|pwd -W|' | $(TR) --delete '\r' >$(2)

# Targets ######################################################################

.PHONY: all
all:
	$(MKDIR) --parents $(build_dir)
# TODO ?? wget...
#	$(CP) /cygdrive/c/wx20061204/wxWidgets-2006-12-04
	$(MAKE) --file=$(this_makefile) --directory=$(build_dir) wx

##	export PATH=$(mingw_bin_dir):$$PATH ; \

# TODO ?? wget...needs work
# ftp://ftp.wxwidgets.org/pub/2.8.4/wxWidgets-2.8.4.tar.bz2

TARFLAGS := --keep-old-files
%.tar.bz2: TARFLAGS += --bzip2
%.tar.gz:  TARFLAGS += --gzip

WGETFLAGS := '--timestamping'

.PHONY: %.tar.bz2 %.tar.gz
%.tar.bz2 %.tar.gz:
	[ -e $@ ] || $(WGET) $(WGETFLAGS) $($@-url)
	$(ECHO) "$($@-md5) *$@" | $(MD5SUM) --check
	-$(TAR) --extract $(TARFLAGS) --directory=$(wx_dir) --file=$@

.PHONY: wx
wx:
	../configure $(config_options) >config_log_$(date) 2>config_err_$(date)
	$(MAKE) >build_log_$(date) 2>build_err_$(date)
# TODO ?? Avoid '--interactive' so that this can run unattended. Use '--force'?
	$(CP) --interactive --preserve lib/*.dll $(prefix)/bin
##	$(call wx_config_fix,wx-config,wx-config-msys)
	$(ECHO) '#!/bin/sh'                          >wx-config-portable
	$(ECHO) 'if   [ "--cxxflags" = $$1 ]; then' >>wx-config-portable
	$(ECHO) "echo `./wx-config --cxxflags`"     >>wx-config-portable
	$(ECHO) 'elif [ "--libs"     = $$1 ]; then' >>wx-config-portable
	$(ECHO) "echo `./wx-config --libs`"         >>wx-config-portable
	$(ECHO) 'else'                              >>wx-config-portable
	$(ECHO) 'echo Bad argument $$1'             >>wx-config-portable
	$(ECHO) 'fi'                                >>wx-config-portable

