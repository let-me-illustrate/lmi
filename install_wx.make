# Install msw-native wx with options suitable for lmi.
#
# Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 Gregory W. Chicares.
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

wx_dir        := /opt/lmi/wx-scratch

# git sha1sum and archive md5sum ###############################################

# Use a string distinct from any official wxwidgets.org release name
# and from any such string previously used in this makefile.

wx_version        := 3.1.0-p1

# Use a github archive as of a particular commit by specifying its
# sha1sum--the "latest commit" shown here, for example:
#   https://github.com/wxWidgets/wxWidgets
# It is generally useful to retain only the last historical version.

# wx_commit_sha     := 4c0e272589667c7cf57407d99f1810e2e83348e4
# wx_md5            := 5fd8da132214bb973133d574fde5cbee
wx_commit_sha     := 41045df7ea5f93e4c07c1bd846d7127a372705bd
wx_md5            := 89775012799fe5c9bd8ea61e5fa43da7

wx_archive        := wxWidgets-$(wx_commit_sha).zip

$(wx_archive)-md5 := $(wx_md5)

wx_server         := https://github.com/wxWidgets/wxWidgets/archive
$(wx_archive)-url := $(wx_server)/$(wx_commit_sha).zip

# Variables that normally should be left alone #################################

# Specify $(build_type) explicitly, depending on `uname`. It would
# seem cleaner to deduce it this way:
#   build_type  := $(shell /path/to/config.guess)
# but that script is not easy to find. There's a copy in the wx
# archive, but that hasn't yet been extracted at this point. Debian
# GNU/Linux provides it in /usr/share/misc/ and recommends:
#   https://wiki.debian.org/AutoTools/autoconf
#   "In general you are much better off building against the current
#   versions of these files than the ones shipped with the tarball"
# but that directory is not on $PATH . Alternatively, RH provides it
# in /usr/lib/rpm/redhat/ .

mingw_bin_dir :=
build_type    := x86_64-unknown-linux-gnu
host_type     := i686-w64-mingw32

uname := $(shell uname -s 2>/dev/null)
ifeq (CYGWIN,$(findstring CYGWIN,$(uname)))
  mingw_bin_dir := $(mingw_dir)/bin/
  build_type    := i686-pc-cygwin
  host_type     := i686-w64-mingw32
endif

compiler       := gcc-$(shell $(mingw_bin_dir)$(host_type)-gcc -dumpversion)
vendor         := $(compiler)-$(wx_md5)

source_dir     := $(wx_dir)/wxWidgets-$(wx_version)
build_dir      := $(source_dir)/$(vendor)

# Configuration reference:
#   http://lists.nongnu.org/archive/html/lmi/2007-11/msg00001.html

wx_cc_flags    := -fno-omit-frame-pointer
wx_cxx_flags   := -fno-omit-frame-pointer -std=c++11

config_options = \
  --prefix=$(prefix) \
  --exec-prefix=$(exec_prefix) \
  --build=$(build_type) \
  --host=$(host_type) \
  --disable-apple_ieee \
  --disable-aui \
  --disable-compat30 \
  --disable-dependency-tracking \
  --disable-fswatcher \
  --disable-gif \
  --disable-mediactrl \
  --disable-precomp-headers \
  --disable-propgrid \
  --disable-ribbon \
  --disable-richtext \
  --disable-stc \
  --disable-webview \
  --enable-monolithic \
  --enable-option-checking \
  --enable-shared \
  --enable-stl \
  --enable-vendor='$(vendor)' \
  --with-cxx=11 \
  --without-opengl \
  --without-subdirs \
  CFLAGS='$(wx_cc_flags)' \
  CXXFLAGS='$(wx_cxx_flags)' \

# Utilities ####################################################################

ECHO   := echo
MD5SUM := md5sum
MKDIR  := mkdir
PATCH  := patch
RM     := rm
TAR    := tar
UNZIP  := unzip
WGET   := wget

# Targets ######################################################################

source_archives := $(wx_archive)
patchset        := wx-$(wx_version).patch
PATCHFLAGS      := --directory=$(source_dir) --strip=1

.PHONY: all
all: clobber_exec_prefix_only $(source_archives)
	[ ! -e $(patchset) ] || $(PATCH) $(PATCHFLAGS) <$(patchset)
	$(MKDIR) --parents $(build_dir)
	$(MAKE) --file=$(this_makefile) --directory=$(build_dir) wx

# Simulated order-only prerequisites.
$(source_archives): initial_setup
initial_setup: clobber_exec_prefix_only

.PHONY: initial_setup
initial_setup:
	$(MKDIR) --parents $(prefix)
	$(MKDIR) --parents $(exec_prefix)
	$(MKDIR) --parents $(cache_dir)
	$(MKDIR) --parents $(wx_dir)

TARFLAGS := --keep-old-files
%.tar.bz2: TARFLAGS += --bzip2
%.tar.gz:  TARFLAGS += --gzip

UNZIPFLAGS := -q

WGETFLAGS :=

.PHONY: %.tar.bz2 %.tar.gz
%.tar.bz2 %.tar.gz:
	cd $(cache_dir) && [ -e $@ ] || $(WGET) $(WGETFLAGS) $($@-url)
	cd $(cache_dir) && $(ECHO) "$($@-md5) *$@" | $(MD5SUM) --check
	-$(TAR) --extract $(TARFLAGS) --directory=$(wx_dir) --file=$(cache_dir)/$@

# Used only for retrieval from github by sha1sum.
#
# This archive is dynamically created by github, as of a commit
# specified by the sha1sum embedded in the URL. Not being a static
# file, it doesn't bear a historical timestamp corresponding to the
# commit date--see:
#   http://lists.nongnu.org/archive/html/lmi/2015-08/msg00012.html
# By default, the cached archive's name would be just the sha1sum plus
# a '.zip' extension, which does not obviously have anything to do
# with wx, so use '--output-document' to prepend "wxWidgets-" to its
# name (while this is not that option's intended purpose, it does the
# right thing in this case). The resulting filename is appropriate for
# caching, but the name of the directory into which it extracts is
# inconvenient for actual use, so rename that directory immediately.

%.zip: WGETFLAGS += '--output-document=$@'

.PHONY: %.zip
%.zip:
	cd $(cache_dir) && [ -e $@ ] || $(WGET) $(WGETFLAGS) $($@-url)
	cd $(cache_dir) && $(ECHO) "$($@-md5) *$@" | $(MD5SUM) --check
	-$(UNZIP) $(UNZIPFLAGS) $(cache_dir)/$@ -d $(wx_dir)
	cp -a $(wx_dir)/$(basename $@) $(source_dir)

.PHONY: wx
wx:
	export PATH="$(mingw_bin_dir):${PATH}" \
	  && ../configure $(config_options) && $(MAKE) && $(MAKE) install \

# This incidentally removes wxPdfDoc, but it's probably a good idea
# to rebuild that whenever wx is upgraded anyway.

.PHONY: clobber_exec_prefix_only
clobber_exec_prefix_only:
	-$(RM) --force --recursive $(exec_prefix)/bin/wx*
	-$(RM) --force --recursive $(exec_prefix)/include/wx*
	-$(RM) --force --recursive $(exec_prefix)/lib/wx*
	-$(RM) --force --recursive $(exec_prefix)/lib/libwx*
	-$(RM) --force --recursive $(wx_dir)

