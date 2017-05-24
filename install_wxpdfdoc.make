# Installer for wxPdfDocument library.
#
# Copyright (C) 2015, 2016, 2017 Gregory W. Chicares.
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

this_makefile := $(abspath $(lastword $(MAKEFILE_LIST)))

# Configurable settings ########################################################

mingw_dir     := /MinGW_

prefix        := /opt/lmi/local
exec_prefix   := $(prefix)

cache_dir     := /cache_for_lmi/downloads

wxpdfdoc_dir  := /opt/lmi/wxpdfdoc-scratch

# URLs and archive md5sums #####################################################

# Unlike the other similar makefiles, we can't make the URL to end with the
# archive name because we don't want to use the bare commit SHA-1 as the file
# name, so we need a separate "urlbase" variable.

wxpdfdoc_commit             := 5ac28a73e74916cf44d0ce286976e21d948e9bd8
wxpdfdoc_archive            := wxpdfdoc-$(wxpdfdoc_commit).zip
$(wxpdfdoc_archive)-urlbase := $(wxpdfdoc_commit).zip
$(wxpdfdoc_archive)-root    := https://github.com/vadz/wxpdfdoc/archive
$(wxpdfdoc_archive)-url     := $($(wxpdfdoc_archive)-root)/$($(wxpdfdoc_archive)-urlbase)
$(wxpdfdoc_archive)-md5     := 8e3c4d6cd1df9c7f91426c8c4723cb6e

# Variables that normally should be left alone #################################

mingw_bin_dir :=
build_type    := x86_64-unknown-linux-gnu
host_type     := i686-w64-mingw32

uname := $(shell uname -s 2>/dev/null)
ifeq (CYGWIN,$(findstring CYGWIN,$(uname)))
  mingw_bin_dir := $(mingw_dir)/bin/
  build_type    := i686-pc-cygwin
  host_type     := i686-w64-mingw32
endif

compiler         := gcc-$(shell $(mingw_bin_dir)$(host_type)-gcc -dumpversion)

wxpdfdoc_version := 0.9.4
source_dir       := $(wxpdfdoc_dir)/wxPdfDoc-$(wxpdfdoc_version)

wx_cc_flags      := -fno-omit-frame-pointer
wx_cxx_flags     := -fno-omit-frame-pointer -std=c++11

config_options = \
  --prefix=$(prefix) \
  --exec-prefix=$(exec_prefix) \
  --with-wx-prefix=$(prefix) \
  --with-wx-exec-prefix=$(exec_prefix) \
  --build=$(build_type) \
  --host=$(host_type) \
  --disable-dependency-tracking \
  CFLAGS='$(wx_cc_flags)' \
  CXXFLAGS='$(wx_cxx_flags)' \

# Utilities ####################################################################

ECHO   := echo
MD5SUM := md5sum
MKDIR  := mkdir
RM     := rm
RMDIR  := rmdir
UNZIP  := unzip
WGET   := wget

# Targets ######################################################################

.PHONY: all
all: clobber
	$(MAKE) --file=$(this_makefile) wxpdfdoc

.PHONY: initial_setup
initial_setup:
	$(MKDIR) --parents $(prefix)
	$(MKDIR) --parents $(exec_prefix)
	$(MKDIR) --parents $(cache_dir)
	$(MKDIR) --parents $(wxpdfdoc_dir)

UNZIPFLAGS := -q

WGETFLAGS :=
%.zip: WGETFLAGS += '--output-document=$@'

.PHONY: %.zip
%.zip: initial_setup
	cd $(cache_dir) && [ -e $@ ] || $(WGET) $(WGETFLAGS) $($@-url)
	cd $(cache_dir) && $(ECHO) "$($@-md5) *$@" | $(MD5SUM) --check
	-$(UNZIP) $(UNZIPFLAGS) $(cache_dir)/$@ -d $(wxpdfdoc_dir)
	mv $(wxpdfdoc_dir)/$(basename $@) $(source_dir)

.PHONY: wxpdfdoc
wxpdfdoc: $(wxpdfdoc_archive)
	cd $(source_dir) \
	  && export PATH="$(mingw_bin_dir):${PATH}" \
	  && ./configure $(config_options) \
	  && $(MAKE) install_pdfdoc_dll install_pdfdoc_dll_headers \

.PHONY: clobber
clobber:
# The 'uninstall_pdfdoc_dll_headers' target doesn't remove the (now empty)
# directory where the headers are installed, so do it separately ourselves.
	-cd $(source_dir) && \
	    $(MAKE) uninstall_pdfdoc_dll uninstall_pdfdoc_dll_headers
	-$(RMDIR) $(prefix)/include/wx
	-$(RM) --force --recursive $(wxpdfdoc_dir)
