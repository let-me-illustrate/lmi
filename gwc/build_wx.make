# Build wx, in MSYS, using MinGW gcc, with options suitable for lmi.
#
# Copyright (C) 2006 Gregory W. Chicares.
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

# $Id: build_wx.make,v 1.4 2006-12-06 17:12:24 chicares Exp $

# This makefile is designed to be run in MSYS: the native zsh port
# we customarily use can't handle 'configure'. Care is taken to
# ensure that the resulting binaries, however, are suitable for use
# with that zsh port or any other shell. For instance, the default
# $(prefix) installs the wx dll to 'C:\usr\local\bin\' instead of some
# directory in MSYS's own filesystem.
#
# Typical usage:
#   /c/any/directory/at/all $make --file=/c/lmi/src/gwc/build_wx.make

# Configurable settings ########################################################

prefix        := /c/usr/local/bin

# 20060605 cvs snapshot for now.
wx_dir        := /c/wx20060605/wxWidgets

mingw_root    := /c
mingw_dir     := /MinGW-20050120

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
  --enable-debug_info \
  --enable-commondlg \
  --disable-compat24 \
  VENDOR='$(vendor)' \

# Utilities ####################################################################

CP     := cp
MKDIR  := mkdir
SED    := sed
TR     := tr

# Dependent variables ##########################################################

date           = $(shell date -u +'%Y%m%dT%H%MZ')

mingw_bin_dir := $(mingw_root)/$(mingw_dir)/bin

vendor        := $(shell $(mingw_bin_dir)/g++ -dumpversion)
vendor        := $(subst .,,$(vendor))

build_dir     := $(wx_dir)/gcc$(vendor)

# MSYS !! Get rid of the hardcoded name when MSYS 'make' is upgraded.
self          := /c/lmi/src/gwc/build_wx.make
ifeq (3.81,$(firstword $(sort $(MAKE_VERSION) 3.81)))
  self        := $(lastword $(MAKEFILE_LIST))
endif

# Portability workaround #######################################################

# 'wx-config' uses 'printf(1)', which zsh supports only in versions
# after 4.0.1 . Replacing it with 'echo' makes 'wx-config' usable with
# older versions of zsh.
#
# When wx is built with MSYS, 'wx-config' hardcodes MSYS-specific
# paths like "/c/wxWidgets/...". Replacing drive letters like '/c/'
# with 'c:/' makes 'wx-config' usable with other shells. Using a
# relative path instead would further increase portability, but would
# fail if 'wx-config' were moved. It would be better to use a facility
# like 'cygpath' if MSYS had one, but it doesn't.
#
# The 'tr' call wouldn't be necessary if msw 'sed' ports could be
# relied upon never to use carriage returns in line endings.

wx_config_fix = \
  $(CP) --preserve $(1) $(2); \
  <$(2) \
    $(SED) \
      -e 's|printf|echo|' \
      -e 's|check_dirname "/\([A-Za-z]\)/|check_dirname "\1:/|' \
    | $(TR) --delete '\r' \
  >$(1)

# Targets ######################################################################

.PHONY: all
all:
	$(MKDIR) --parents $(build_dir)
	export PATH=$(mingw_bin_dir):$$PATH ; \
	$(MAKE) --file=$(self) --directory=$(build_dir) wx

.PHONY: wx
wx:
	../configure $(config_options) >config_log_$(date) 2>config_err_$(date)
	$(MAKE) >build_log_$(date) 2>build_err_$(date)
	$(CP) --interactive --preserve lib/*.dll $(prefix)
	$(call wx_config_fix,wx-config,wx-config-original)

