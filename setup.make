# Set up build environment.
#
# Copyright (C) 2005, 2006, 2007 Gregory W. Chicares.
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

# $Id: setup.make,v 1.39 2007-12-06 05:28:26 chicares Exp $

.PHONY: all
all: setup

src_dir   := $(CURDIR)

################################################################################

# Other makefiles included; makefiles not to be remade.

# Don't remake this file.

setup.make:: ;

# Configuration.

include $(src_dir)/configuration.make
$(src_dir)/configuration.make:: ;

# TODO ?? Use these if we build libraries.

# Automatic dependencies.

include $(src_dir)/autodependency.make
$(src_dir)/autodependency.make:: ;

# Objects.

include $(src_dir)/objects.make
$(src_dir)/objects.make:: ;

###############################################################################

# TODO ?? Move these definitions elsewhere.

third_party_dir         := $(system_root)/opt/lmi/third_party
third_party_bin_dir     := $(third_party_dir)/bin
third_party_include_dir := $(third_party_dir)/include
third_party_lib_dir     := $(third_party_dir)/lib
third_party_source_dir  := $(third_party_dir)/src

.PHONY: setup
setup: \
  dummy_libraries \
  frozen_libxml2 \

# REVIEW: Could these be combined? Untested idea:
# third_party_directories := \
#   third_party_bin_dir \
#   third_party_include_dir \
#   third_party_lib_dir \
#   third_party_source_dir \
#
# $(third_party_directories):
# 	+@[ -d $@ ] || $(MKDIR) --parents $@

.PHONY: $(third_party_dir)
$(third_party_dir):
	+@[ -d $@ ] || $(MKDIR) --parents $@

.PHONY: $(third_party_bin_dir)
$(third_party_bin_dir):
	+@[ -d $@ ] || $(MKDIR) --parents $@

.PHONY: $(third_party_include_dir)
$(third_party_include_dir):
	+@[ -d $@ ] || $(MKDIR) --parents $@

.PHONY: $(third_party_lib_dir)
$(third_party_lib_dir):
	+@[ -d $@ ] || $(MKDIR) --parents $@

.PHONY: $(third_party_source_dir)
$(third_party_source_dir):
	+@[ -d $@ ] || $(MKDIR) --parents $@

###############################################################################
# This version has not been formally tested and released for production with
# lmi. The version actually used can't be easily rebuilt, so this target is
# intended to provide a stop-gap library until it can be formally tested.

# Install libxml2-2.6.19 .

.PHONY: frozen_libxml2
frozen_libxml2:
	$(MAKE) \
	  --directory=/tmp \
	  --file=$(src_dir)/setup.make \
	                    src_dir='$(src_dir)' \
	        third_party_bin_dir='$(third_party_bin_dir)' \
	    third_party_include_dir='$(third_party_include_dir)' \
	     third_party_source_dir='$(third_party_source_dir)' \
	  install_frozen_libxml2_from_tmp_dir

.PHONY: install_frozen_libxml2_from_tmp_dir
install_frozen_libxml2_from_tmp_dir:
	[ -e libxml2-2.6.19.tar.bz2 ] \
	  || $(WGET) --non-verbose \
	  http://ftp.gnome.org/pub/GNOME/sources/libxml2/2.6/libxml2-2.6.19.tar.bz2
	$(ECHO) "ed581732d586f86324ec46e572526ede  libxml2-2.6.19.tar.bz2" \
	  |$(MD5SUM) --check
	$(BZIP2) --decompress --keep libxml2-2.6.19.tar.bz2
	$(TAR) --extract --file=libxml2-2.6.19.tar
	cd libxml2-2.6.19; \
	/msys/1.0/bin/sh.exe ./configure && /msys/1.0/bin/make
	$(MKDIR) --parents $(third_party_include_dir)/libxml/
	-$(CP) --force --preserve --recursive libxml2-2.6.19/include/libxml/* \
	  $(third_party_include_dir)/libxml/ 2>/dev/null
	$(CP) --force --preserve libxml2-2.6.19/.libs/libxml2-2.dll \
	  $(third_party_bin_dir)
	$(CP) --force --preserve libxml2-2.6.19/.libs/libxml2.dll.a \
	  $(third_party_lib_dir)
	$(CP) --force --preserve libxml2-2.6.19/.libs/xmllint.exe /usr/bin/
	$(RM) --force libxml2-2.6.19.tar libxml2-2.6.19.tar.bz2

# REVIEW: Should we document the reason for using '.libs' in three of
# the last four commands? I think we had concluded that it was an odd
# usage, but that less odd ways (like 'make install'?) didn't work.

