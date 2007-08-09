# Install msw-native libxml2 and libxslt with options suitable for lmi.
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

# $Id: install_libxml2_libxslt.make,v 1.1 2007-08-09 15:13:39 chicares Exp $

# Configurable settings ########################################################

# Libraries are ordered by dependency, rather than alphabetically.
source_archives := \
  libxml2/2.6/libxml2-2.6.26.tar.bz2 \
  libxslt/1.1/libxslt-1.1.17.tar.bz2 \

host          := ftp://ftp.gnome.org
host_path     := pub/GNOME/sources

prefix        := /usr/local

xml_dir       := xml-scratch

# Variables that normally should be left alone #################################

date           = $(shell date -u +'%Y%m%dT%H%MZ')

ifeq (3.81,$(firstword $(sort $(MAKE_VERSION) 3.81)))
  this_makefile := $(abspath $(lastword $(MAKEFILE_LIST)))
else
  $(error Upgrade to make-3.81 .)
endif

tools_dir := $(dir $(this_makefile))/tools

common_options := \
  --disable-static \
  --enable-shared \
  --with-debug \
  --without-python \
  CC='gcc -mno-cygwin' \
  LDFLAGS='-mno-cygwin -lws2_32' \

libxml2-2.6.26_options := \
  $(common_options) \
  --with-schemas \
  --without-iconv \
  --without-modules \
  --without-schematron \

libxslt-1.1.17_options := \
  $(common_options) \
  --with-libxml-prefix=$(prefix) \
  --without-crypto \

# Utilities ####################################################################

CP     := cp
DATE   := date
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
  "\nautomated downloads. Install it on your PATH." \
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

# Patches ######################################################################

# libxslt-1.1.17.patch

.PHONY: patch_libxslt
patch_libxslt: libxslt/1.1/libxslt-1.1.17.tar.bz2
	cd $(xml_dir)/libxslt-1.1.17; \
	  $(PATCH) --batch --forward --strip=1 < $(tools_dir)/libxslt/1_1_17_patch; \

.PHONY: clobber
clobber:
	-for z in $(notdir $(libraries)); \
	  do cd $(xml_dir)/$$z && make uninstall maintainer-clean; \
	  done;
	-for z in $(notdir $(libraries)); \
	  do \
	    shopt -s extglob; \
	    echo "Removing non-log files in $$z"; \
	    { for f in $(xml_dir)/$$z/'!(log-*)'; \
	      do \
	        rm --recursive $$f; \
	      done; \
	    }; \
	  done;

