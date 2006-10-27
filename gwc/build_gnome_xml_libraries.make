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

# $Id: build_gnome_xml_libraries.make,v 1.4 2006-10-27 22:28:05 chicares Exp $

# This makefile is designed to be run in MSYS: the native zsh port
# we customarily use can't handle 'configure'. Care is taken to
# ensure that the resulting binaries, however, are suitable for use
# with that zsh port. For instance, the default $(prefix) installs
# binaries to 'C:\usr\local\bin\' instead of MSYS's '/usr/bin/',
# which by default maps to 'C:\msys\1.0\bin' ('C:\msys\1.0\usr\bin'
# does not actually exist).
#
# Typical usage:
#   $make --file=/c/lmi/src/lmi/gwc/build_gnome_xml_libraries.make

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
  --disable-static \
  --enable-shared \
  --with-debug \
  --with-schemas \
  --without-iconv \
  --without-modules \
  --without-schematron \

libxslt-1.1.17_options := \
  --disable-static \
  --enable-shared \
  --with-debug \

libxml++-2.14.0_options := \
  --disable-static \
  --enable-debug \
  --enable-shared \
  --without-ustring \

# Setting $(prefix) in the environment appears to have no effect on
# the autotools files, so this alternative seems to be necessary.

libxml++-2.14.0_exports := \
  export LIBXML_CFLAGS="-I$(prefix)/include/libxml2"; \
  export   LIBXML_LIBS="-L$(prefix)/lib/ -lxml2"; \
  export PKG_CONFIG=pkg-config ; \

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
all: clobber $(source_archives) $(libraries)

# Simulated order-only prerequisites.
$(libraries): $(source_archives)
$(source_archives): initial_setup
initial_setup: clobber

.PHONY: initial_setup
initial_setup:
	@type "$(WGET)" >/dev/null || { $(ECHO) -e $(wget_missing) && false; }
	@$(MKDIR) --parents $(prefix)
	@$(MKDIR) --parents $(xml_dir)

WGETFLAGS := \
  --cut-dirs=$(words $(subst /, ,$(host_path))) \
  --force-directories \
  --no-host-directories \
  --non-verbose \
  --timestamping \

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
	export PATH=$(mingw_bin_dir):$(prefix)/bin:$$PATH ; \
	export LOG=log-$(date); \
	$($(notdir $@)_exports) \
	$(RM) --force $$LOG; \
	cd $(xml_dir)/$(notdir $@); \
	  ./configure --prefix=$(prefix) $($(notdir $@)_options) >>$$LOG 2>&1; \
	  $(MAKE)                                                >>$$LOG 2>&1; \
	  $(MAKE) install                                        >>$$LOG 2>&1; \

.PHONY: libxslt/1.1/libxslt-1.1.17
libxslt/1.1/libxslt-1.1.17: patch_libxslt
libxslt/1.1/libxslt-1.1.17: libxml2/2.6/libxml2-2.6.26

.PHONY: libxml++/2.14/libxml++-2.14.0
libxml++/2.14/libxml++-2.14.0: patch_libxml++ setup_libxml++
libxml++/2.14/libxml++-2.14.0: libxml2/2.6/libxml2-2.6.26
libxml++/2.14/libxml++-2.14.0: libxslt/1.1/libxslt-1.1.17

# Patches ######################################################################

# TODO ?? Put these in cvs after checking copyright for '*.m4':
#
# libxslt-1.1.17.patch
# libxml++-2.14.0.patch
# pkg.m4
# stub.m4

.PHONY: patch_libxslt
patch_libxslt: libxslt/1.1/libxslt-1.1.17.tar.bz2
	cd $(xml_dir)/libxslt-1.1.17; \
	  $(PATCH) --batch --forward --strip=1 < $(CURDIR)/libxslt-1.1.17.patch; \

.PHONY: patch_libxml++
patch_libxml++: libxml++/2.14/libxml++-2.14.0.tar.bz2
	cd $(xml_dir)/libxml++-2.14.0; \
	  $(PATCH) --batch --forward --strip=1 < $(CURDIR)/libxml++-2.14.0.patch; \
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
	-for z in $(notdir $(libraries)); \
	  do cd $(xml_dir)/$$z && make clean uninstall; \
	  done;
	-for z in $(notdir $(libraries)); \
	  do \
	    shopt -s extglob; \
	    { for f in '$(xml_dir)/$$z/!(log-*)'; do rm --recursive $$f; done; }; \
	  done;

