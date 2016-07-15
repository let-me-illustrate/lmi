# Install libxml2 and libxslt with options suitable for lmi.
#
# Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

# Configurable settings ########################################################

# Archives and their md5sums

libxml2_version = libxml2-2.9.4
libxslt_version = libxslt-1.1.29

libxml2_archive := $(libxml2_version).tar.gz
libxslt_archive := $(libxslt_version).tar.gz

libxml2-2.9.4.tar.gz-md5  := ae249165c173b1ff386ee8ad676815f5
libxslt-1.1.29.tar.gz-md5 := a129d3c44c022de3b9dcf6d6f288d72e

# Libraries are ordered by dependency, rather than alphabetically.
source_archives := \
  $(libxml2_archive) \
  $(libxslt_archive) \

# libxslt requires a roughly contemporary libxml2
$(libxslt_version): $(libxml2_version)

# 'ftp://xmlsoft.org' has 'libxml2/' and 'libxslt/' subdirectories,
# but they seem to be identical: the latter is apparently a link to
# the former, so only the former is used here.

host          := ftp://xmlsoft.org
host_path     := libxml2

mingw_dir     := /MinGW_

prefix        := /opt/lmi/local

cache_dir     := /cache_for_lmi/downloads

xml_dir       := /opt/lmi/xml-scratch

# Variables that normally should be left alone #################################

mingw_bin_dir := $(mingw_dir)/bin

# For 'host' and 'build' configure options, see:
#   http://cygwin.com/ml/cygwin/2002-01/msg00837.html

# '--disable-dependency-tracking' is required with the MinGW toolchain
# in a Cygwin shell, to prevent a catastrophic dependency-tracking
# failure. Apparently the problem is colons in header paths, e.g.:
#   c:/MinGW-20050827/bin/../lib/gcc/mingw32/3.4.4/include/stddef.h:
# which elicit fatal errors such as this:
#   .deps/DOCBparser.Plo:1: *** multiple target patterns.  Stop.

common_options := \
  --build=i686-pc-cygwin \
  --host=i686-w64-mingw32 \
  --disable-dependency-tracking \
  --disable-static \
  --enable-shared \
  --with-debug \
  --without-python \
  LDFLAGS='-lws2_32' \
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

# Expunge '--without-threads' and '--without-zlib' soon.

$(libxml2_version)_options := \
  $(common_options) \
  --with-schemas \
  --without-iconv \
  --without-modules \
  --without-schematron \
  --without-threads \
  --without-zlib \

$(libxslt_version)_options := \
  $(common_options) \
  --with-libxml-prefix=$(prefix) \
  --without-crypto \

# Utilities ####################################################################

ECHO   := echo
GREP   := grep
MD5SUM := md5sum
MKDIR  := mkdir
PATCH  := patch
RM     := rm
TAR    := tar
WGET   := wget

# Targets ######################################################################

libraries := $(libxml2_version) $(libxslt_version)

.PHONY: all
all: clobber $(source_archives) $(libraries)

# Simulated order-only prerequisites.
$(libraries): $(source_archives)
$(source_archives): initial_setup
initial_setup: clobber

.PHONY: initial_setup
initial_setup:
	$(MKDIR) --parents $(prefix)
	$(MKDIR) --parents $(cache_dir)
	$(MKDIR) --parents $(xml_dir)

WGETFLAGS := \
  --cut-dirs=$(words $(subst /, ,$(host_path))) \
  --force-directories \
  --no-host-directories \

wget_whence := $(host)/$(host_path)

TARFLAGS := --keep-old-files
%.tar.bz2: TARFLAGS += --bzip2
%.tar.gz:  TARFLAGS += --gzip

.PHONY: %.tar.bz2 %.tar.gz
%.tar.bz2 %.tar.gz:
	cd $(cache_dir) && [ -e $@ ] || $(WGET) $(WGETFLAGS) $(wget_whence)/$@
	cd $(cache_dir)/$(dir $@) && \
	  $(ECHO) "$($@-md5) *$@" | $(MD5SUM) --check
	$(TAR) --extract $(TARFLAGS) --directory=$(xml_dir) --file=$(cache_dir)/$@

# Someday it may be necessary to add a line like this to the recipe:
#   export lt_cv_to_tool_file_cmd=func_convert_file_cygwin_to_w32
# but that's not necessary for now. See:
#   http://lists.nongnu.org/archive/html/lmi/2011-06/msg00025.html

.PHONY: $(libraries)
$(libraries):
	-[ -e $@-lmi.patch ] && $(PATCH) --directory=$(xml_dir) --strip=1 <$@-lmi.patch
	cd $(xml_dir)/$@ \
	  && export PATH="$(mingw_bin_dir):${PATH}" \
	  && ./configure --prefix=$(prefix) $($@_options) \
	  && $(MAKE) \
	  && $(MAKE) install \

.PHONY: clobber
clobber:
	-for z in $(libraries); \
	  do \
	    cd $(xml_dir)/$$z && $(MAKE) uninstall maintainer-clean; \
	    $(RM) --recursive $(xml_dir)/$$z; \
	  done;

