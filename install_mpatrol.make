# Install msw-native mpatrol with modifications suitable for lmi.
#
# Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares.
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
# email: <gchicares@sbcglobal.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

# $Id$

# Configurable settings ########################################################

# By default, 'prefix' is the toolchain's own directory, because
# mpatrol is really an auxiliary part of the toolchain and may be
# sensitive to ABI changes (e.g., C++ name mangling).

mpatrol_version := 1.4.8

mingw_dir       := /MinGW_

prefix          := $(mingw_dir)

cache_dir       := /cache_for_lmi/downloads

mpatrol_dir     := /opt/lmi/mpatrol-scratch

sf_mirror       := http://downloads.sourceforge.net

# URLs and archive md5sums #####################################################

mpatrol_1.4.8-md5 := ada423c49bc5bfa7c3e7a80a711c2a1a

mpatrol_archive        := mpatrol_$(mpatrol_version).tar.gz

$(mpatrol_archive)-url := $(sf_mirror)/mpatrol/$(mpatrol_archive)

$(mpatrol_archive)-md5 := $(mpatrol_$(mpatrol_version)-md5)

mpatrol_md5            := $(mpatrol_$(mpatrol_version)-md5)

# Variables that normally should be left alone #################################

mingw_bin_dir    := $(mingw_dir)/bin

build_dir        := $(mpatrol_dir)/mpatrol/build/windows

mpatrol_makefile := mpatrol-mingw-GNUmakefile

# SOMEDAY !! Replace $(mpatrol-mingw-GNUmakefile) with autotools.

config_options = \
  --prefix=$(prefix) \
  --build=i686-pc-cygwin \
  --host=i686-pc-mingw32 \
  --other_options \
       AR='$(mingw_bin_dir)/ar' \
       AS='$(mingw_bin_dir)/as' \
       CC='$(mingw_bin_dir)/gcc' \
      CPP='$(mingw_bin_dir)/cpp' \
      CXX='$(mingw_bin_dir)/g++' \
  DLLTOOL='$(mingw_bin_dir)/dlltool' \
       LD='$(mingw_bin_dir)/ld' \
       NM='$(mingw_bin_dir)/nm' \
  OBJDUMP='$(mingw_bin_dir)/objdump' \
   RANLIB='$(mingw_bin_dir)/ranlib' \
    STRIP='$(mingw_bin_dir)/strip' \
  WINDRES='$(mingw_bin_dir)/windres' \

# Utilities ####################################################################

CP     := cp
ECHO   := echo
MD5SUM := md5sum
MKDIR  := mkdir
PATCH  := patch
RM     := rm
TAR    := tar
WGET   := wget

# Targets ######################################################################

# Patch 'mpatrol_1.4.8.patch' is equivalent to:
#   http://sourceforge.net/tracker/download.php?group_id=19456&atid=319456&file_id=117735&aid=1112376
# It suppresses diagnostics for zero-byte moves, whose behavior is
# actually undefined according to C99 7.21.1/2; but wxWidgets and
# libstdc++ rely on that behavior being benign.

source_archives := $(mpatrol_archive)
patchset        := mpatrol_$(mpatrol_version).patch

.PHONY: all
all: clobber $(source_archives)
	-[ -e $(patchset) ] && $(PATCH) --directory=$(mpatrol_dir) --strip=1 <$(patchset)
	$(CP) --preserve $(mpatrol_makefile) $(build_dir)
	export PATH=$(mingw_bin_dir):$$PATH ; $(MAKE) --file=$(mpatrol_makefile) --directory=$(build_dir) --jobs=1 prefix=$(prefix) all install

# Simulated order-only prerequisites.
$(source_archives): initial_setup
initial_setup: clobber

.PHONY: initial_setup
initial_setup:
	@$(MKDIR) --parents $(prefix)
	@$(MKDIR) --parents $(cache_dir)
	@$(MKDIR) --parents $(mpatrol_dir)

WGETFLAGS :=

TARFLAGS := --keep-old-files
%.tar.bz2: TARFLAGS += --bzip2
%.tar.gz:  TARFLAGS += --gzip

.PHONY: %.tar.bz2 %.tar.gz
%.tar.bz2 %.tar.gz:
	cd $(cache_dir) && [ -e $@ ] || $(WGET) $(WGETFLAGS) $($@-url)
	cd $(cache_dir) && $(ECHO) "$($@-md5) *$@" | $(MD5SUM) --check
	-$(TAR) --extract $(TARFLAGS) --directory=$(mpatrol_dir) --file=$(cache_dir)/$@

.PHONY: mpatrol
mpatrol:
# SOMEDAY !! An autotools build system might be invoked here. E.g.:
#   ../configure $(config_options) && $(MAKE) && $(MAKE) install

.PHONY: clobber
clobber:
	-$(RM) --force --recursive $(mpatrol_dir)

