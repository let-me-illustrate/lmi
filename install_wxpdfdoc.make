# Installer for wxPdfDocument library.
#
# Copyright (C) 2015, 2016 Gregory W. Chicares.
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

this_makefile := $(abspath $(lastword $(MAKEFILE_LIST)))

# Configurable settings ########################################################

mingw_dir     := /MinGW_

prefix        := /opt/lmi/local

cache_dir     := /cache_for_lmi/downloads

wxpdfdoc_dir  := /opt/lmi/wxpdfdoc-scratch

# URLs and archive md5sums #####################################################

# Unlike the other similar makefiles, we can't make the URL to end with the
# archive name because we don't want to use the bare commit SHA-1 as the file
# name, so we need a separate "urlbase" variable.

wxpdfdoc_commit             := 5ac28a73e74916cf44d0ce286976e21d948e9bd8
wxpdfdoc_basename           := wxpdfdoc-$(wxpdfdoc_commit)
wxpdfdoc_archive            := $(wxpdfdoc_basename).zip
$(wxpdfdoc_archive)-urlbase := $(wxpdfdoc_commit).zip
$(wxpdfdoc_archive)-root    := https://github.com/vadz/wxpdfdoc/archive
$(wxpdfdoc_archive)-url     := $($(wxpdfdoc_archive)-root)/$($(wxpdfdoc_archive)-urlbase)
$(wxpdfdoc_archive)-md5     := 8e3c4d6cd1df9c7f91426c8c4723cb6e

# Variables that normally should be left alone #################################

mingw_bin_dir := $(mingw_dir)/bin

compiler      := gcc-$(shell $(mingw_bin_dir)/gcc -dumpversion)

config_options = \
  --prefix=$(prefix) \
  --build=i686-pc-cygwin \
  --host=i686-pc-mingw32 \
  --disable-dependency-tracking \
  --with-wx-config=$(prefix)/bin/wx-config-portable \
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

ECHO   := echo
MD5SUM := md5sum
MKDIR  := mkdir
RM     := rm
RMDIR  := rmdir
UNZIP  := unzip
WGET   := wget

WGETFLAGS :=

# Targets ######################################################################

.PHONY: all
all: clobber
	$(MAKE) --file=$(this_makefile) wxpdfdoc

.PHONY: initial_setup
initial_setup:
	@$(MKDIR) --parents $(prefix)
	@$(MKDIR) --parents $(cache_dir)
	@$(MKDIR) --parents $(wxpdfdoc_dir)

.PHONY: %.zip
%.zip: initial_setup
	cd $(cache_dir) && [ -e $@ ] || $(WGET) $(WGETFLAGS) --output-document=$@ $($@-url)
	cd $(cache_dir) && $(ECHO) "$($@-md5) *$@" | $(MD5SUM) --check
	-$(UNZIP) $(cache_dir)/$@ -d $(wxpdfdoc_dir)

.PHONY: wxpdfdoc
wxpdfdoc: $(wxpdfdoc_archive)
	cd $(wxpdfdoc_dir)/$(wxpdfdoc_basename) && \
	    ./configure $(config_options) && \
	    $(MAKE) install_pdfdoc_dll install_pdfdoc_dll_headers

.PHONY: clobber
clobber:
# The 'uninstall_pdfdoc_dll_headers' target doesn't remove the (now empty)
# directory where the headers are installed, so do it separately ourselves.
	-cd $(wxpdfdoc_dir)/$(wxpdfdoc_basename) && \
	    $(MAKE) uninstall_pdfdoc_dll uninstall_pdfdoc_dll_headers
	-$(RMDIR) $(prefix)/include/wx
	-$(RM) --force --recursive $(wxpdfdoc_dir)
