# Install libxml++, in MSYS, using MinGW gcc, with options suitable for lmi.
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

# $Id: build_libxmlpp.make,v 1.1 2006-10-24 02:52:05 chicares Exp $

# This makefile is designed to be run in MSYS: the native zsh port
# we customarily use can't handle 'configure'. Care is taken to
# ensure that the resulting binaries, however, are suitable for use
# with that zsh port. For instance, the default $(prefix) installs
# binaries to 'C:\usr\local\bin\' instead of MSYS's '/usr/bin/',
# which by default maps to 'C:\msys\1.0\bin' ('C:\msys\1.0\usr\bin'
# does not actually exist).
#
# Typical usage:
#   /c/any/directory/at/all $make --file=/c/lmi/src/lmi/gwc/build_libxmlpp.make

# Configurable settings ########################################################

# Libraries are ordered by dependency, rather than alphabetically.
source_archives := \
  libxml2/2.6/libxml2-2.6.26.tar.bz2 \
  libxslt/1.1/libxslt-1.1.17.tar.bz2 \
  libxml++/2.14/libxml++-2.14.0.tar.bz2 \

host          := ftp://ftp.gnome.org
host_path     := pub/GNOME/sources

prefix        := /c/usr/local

xml_dir       := /c/xml

mingw_root    := /c
mingw_dir     := /MinGW-20050120

# Variables that normally should be left alone #################################

date           = $(shell date -u +'%Y%m%dT%H%MZ')

mingw_bin_dir := $(mingw_root)/$(mingw_dir)/bin

libxml2-2.6.26_options := \
  --enable-shared \
  --disable-static \
  --with-debug \
  --without-iconv \
  --without-modules \
  --with-schemas \
  --without-schematron \

libxslt-1.1.17_options := \
  --with-debug \
  --enable-shared \
  --disable-static \

libxml++-2.14.0_options := \
  --without-ustring \
  --enable-shared \
  --disable-static \

# Setting $(prefix) in the environment appears to have no effect on
# the autotools files, so this alternative seems to be necessary.

libxslt-1.1.17_exports := \
  export LIBXML_CFLAGS="-I$(prefix)/include/libxml2"; \
  export   LIBXML_LIBS="-L$(prefix)/lib -lxml2 -lws2_32"; \

libxml++-2.14.0_exports := \
  export PKG_CONFIG=pkg-config ; \
  export LIBXML_CFLAGS="-I$(prefix)/include/libxml2"; \
  export   LIBXML_LIBS="-L$(prefix)/lib/ -lxml2"; \

# Utilities ####################################################################

CP     := cp
ECHO   := echo
GREP   := grep
MD5SUM := md5sum
MKDIR  := mkdir
PATCH  := patch
RM     := rm
TAR    := tar
WGET   := wget

# Error messages ###############################################################

wget_missing = \
  "\nError: Unable to find '$(WGET)', which is required for" \
  "\nautomated downloads. Download it manually from the MinGW" \
  "\nsite and install it on your PATH." \
  "\n"

# Targets ######################################################################

libraries := $(source_archives:.tar.bz2=)

.PHONY: all
all: $(source_archives) $(libraries)

$(source_archives): initial_setup

.PHONY: initial_setup
initial_setup:
	@type "$(WGET)" >/dev/null || { $(ECHO) -e $(wget_missing) && false; }
	@$(MKDIR) --parents $(prefix)
	@$(MKDIR) --parents $(xml_dir)

WGETFLAGS := \
  --non-verbose \
  --timestamping \
  --force-directories \
  --no-host-directories \
  --cut-dirs=$(words $(subst /, ,$(host_path))) \

wget_whence := $(host)/$(host_path)

%.tar.bz2: decompress = --bzip2
%.tar.gz:  decompress = --gzip

.PHONY: %.tar.bz2 %.tar.gz
%.tar.bz2 %.tar.gz:
	@[ -e $*.md5sum ] || $(WGET) $(WGETFLAGS) $(wget_whence)/$*.md5sum
	@[ -e $@        ] || $(WGET) $(WGETFLAGS) $(wget_whence)/$@
	cd $(dir $@); \
	  $(GREP) $(notdir $@) $(notdir $*).md5sum | $(MD5SUM) --check --status -
	$(TAR) --extract $(decompress) --directory=$(xml_dir) --file=$@

.PHONY: $(libraries)
$(libraries):
	export PATH=$(mingw_bin_dir):$$PATH:$(prefix)/bin ; \
	export LOG=log-$(date); \
	$($(notdir $@)_exports) \
	$(RM) --force $$LOG; \
	cd $(xml_dir)/$(notdir $@); \
	  ./configure $($(notdir $@)_options) >>$$LOG 2>&1; \
	  $(MAKE)                             >>$$LOG 2>&1; \
	  $(MAKE) prefix=$(prefix) install    >>$$LOG 2>&1; \

.PHONY: libxslt/1.1/libxslt-1.1.17
libxslt/1.1/libxslt-1.1.17: patch_libxslt

.PHONY: libxml++/2.14/libxml++-2.14.0
libxml++/2.14/libxml++-2.14.0: patch_libxml++ setup_libxml++

# Patches ######################################################################

# TODO ?? Put these in cvs or ftp after checking copyright:
#
# libxslt-msys-shared-patch
# libxslt-enum-patch
# libxml++_replace_ustring_with_std_string.updated.patch
# libxml++_exclude_examples_sax_exception.patch
# pkg.m4
# stub.m4

# Patch libxslt.

.PHONY: patch_libxslt
patch_libxslt:
	cd $(xml_dir)/libxslt-1.1.17; \
	  $(PATCH) --batch --reverse --strip=1 < $(CURDIR)/libxslt-msys-shared-patch; \
	  $(PATCH) --batch --forward --strip=1 < $(CURDIR)/libxslt-enum-patch; \

# Patch libxml++.

.PHONY: patch_libxml++
patch_libxml++:
	cd $(xml_dir)/libxml++-2.14.0; \
	  $(PATCH) --batch --forward --strip=1 < $(CURDIR)/libxml++_replace_ustring_with_std_string.updated.patch; \
	  $(PATCH) --batch --forward --strip=1 < $(CURDIR)/libxml++_exclude_examples_sax_exception.patch; \
	  $(CP) --preserve --update $(CURDIR)/pkg.m4 .; \
	  $(CP) --preserve --update $(CURDIR)/stub.m4 .; \

.PHONY: setup_libxml++
setup_libxml++:
	cd $(xml_dir)/libxml++-2.14.0; \
	  aclocal -I .; \
	  autoconf; \
	  automake --add-missing --copy; \
	  $(ECHO) '#!/bin/sh' > pkg-config; \

.PHONY: clobber
clobber:
	$(ECHO) $(notdir $(libraries))
	for z in $(notdir $(libraries)); \
	  do cd $(xml_dir)/$$z && make prefix=$(prefix) clean uninstall; \
	  done;

