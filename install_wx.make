# Install msw-native wx with options suitable for lmi.
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

# $Id$

this_makefile := $(abspath $(lastword $(MAKEFILE_LIST)))

# Configurable settings ########################################################

wx_version    := 3.0.0

mingw_dir     := /MinGW_

prefix        := /opt/lmi/local

cache_dir     := /cache_for_lmi/downloads

wx_dir        := /opt/lmi/wx-scratch

# URLs and archive md5sums #####################################################

wx-2.8.6-md5  := 8a130e5b25448a17454a6b957a5e075c
wx-2.8.7-md5  := e3455083afdf6404a569a8bf0701cf13
wx-2.8.9-md5  := b0b2d0f6915a21ca6f33896ee8f50387
wx-2.8.10-md5 := 0461c2085ac1ad7e648aa84c4ba51dd1
wx-2.9.0-md5  := 09058928eeb72853142c062bdec056ce
wx-2.9.2-md5  := d6cec5bd331ba90b74c1e2fcb0563620
wx-2.9.3-md5  := 6b6003713289ea4d3cd9b49c5db5b721
wx-2.9.5-md5  := e98c5f92805493f150656403ffef3bb0
wx-3.0.0-md5  := 241998efc12205172ed24c18788ea2cd

wx_md5            := $(wx-$(wx_version)-md5)

wx_archive        := wxWidgets-$(wx_version).tar.bz2

$(wx_archive)-md5 := $(wx_md5)

$(wx_archive)-url := ftp://ftp.wxwidgets.org/pub/$(wx_version)/$(wx_archive)

# Enable this conditional section to use a github archive as of a
# particular commit by specifying its sha1sum--the "latest commit"
# shown here, for example:
#   https://github.com/wxWidgets/wxWidgets

use_git := Y

ifneq ($(use_git), N)

  wx_commit_sha     := 0b821adf903872b6d8b56630d2191c5b9c3362e5
  wx_md5            := ac28a959aabe36e26ea039ed78a51b54

  wx_version        := $(wx_commit_sha)

  wx_archive        := wxWidgets-$(wx_commit_sha).zip

  $(wx_archive)-md5 := $(wx_md5)

  $(wx_archive)-url := https://github.com/wxWidgets/wxWidgets/archive/$(wx_commit_sha).zip

endif

# Variables that normally should be left alone #################################

mingw_bin_dir := $(mingw_dir)/bin

compiler      := gcc-$(shell $(mingw_bin_dir)/gcc -dumpversion)
vendor        := $(subst .,,$(compiler))-$(wx_md5)

build_dir     := $(wx_dir)/wxWidgets-$(wx_version)/$(vendor)

# Configuration reference:
#   http://lists.nongnu.org/archive/html/lmi/2007-11/msg00001.html
#
# Pass gcc options in $CC and $CXX, not $*FLAGS--explanation here:
#   http://lists.nongnu.org/archive/html/lmi/2013-07/msg00001.html

ggc_flags := --param ggc-min-expand=25 --param ggc-min-heapsize=32768

config_options = \
  --prefix=$(prefix) \
  --build=i686-pc-cygwin \
  --host=i686-pc-mingw32 \
  --disable-apple_ieee \
  --disable-compat24 \
  --disable-fswatcher \
  --disable-gif \
  --disable-aui \
  --disable-mediactrl \
  --disable-precomp-headers \
  --disable-propgrid \
  --disable-ribbon \
  --disable-richtext \
  --disable-stc \
  --disable-webview \
  --disable-threads \
  --enable-monolithic \
  --enable-shared \
  --enable-std_iostreams \
  --enable-stl \
  --enable-vendor='$(vendor)' \
  --without-libjpeg \
  --without-libtiff \
  --without-opengl \
  --without-subdirs \
       AR='$(mingw_bin_dir)/ar' \
       AS='$(mingw_bin_dir)/as' \
       CC='$(mingw_bin_dir)/gcc $(ggc_flags)' \
      CPP='$(mingw_bin_dir)/cpp' \
      CXX='$(mingw_bin_dir)/g++ $(ggc_flags)' \
  DLLTOOL='$(mingw_bin_dir)/dlltool' \
       LD='$(mingw_bin_dir)/ld' \
       NM='$(mingw_bin_dir)/nm' \
  OBJDUMP='$(mingw_bin_dir)/objdump' \
   RANLIB='$(mingw_bin_dir)/ranlib' \
    STRIP='$(mingw_bin_dir)/strip' \
  WINDRES='$(mingw_bin_dir)/windres' \

# Utilities ####################################################################

CHMOD  := chmod
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

.PHONY: all
all: clobber $(source_archives)
	-[ -e $(patchset) ] && $(PATCH) --directory=$(wx_dir) --strip=1 <$(patchset)
	$(MAKE) --file=$(this_makefile) --directory=$(build_dir) wx
	$(MAKE) --file=$(this_makefile) --directory=$(prefix)/bin portable_script

# Simulated order-only prerequisites.
$(source_archives): initial_setup
initial_setup: clobber

.PHONY: initial_setup
initial_setup:
	@$(MKDIR) --parents $(prefix)
	@$(MKDIR) --parents $(cache_dir)
	@$(MKDIR) --parents $(build_dir)

WGETFLAGS :=

TARFLAGS := --keep-old-files
%.tar.bz2: TARFLAGS += --bzip2
%.tar.gz:  TARFLAGS += --gzip

.PHONY: %.tar.bz2 %.tar.gz
%.tar.bz2 %.tar.gz:
	cd $(cache_dir) && [ -e $@ ] || $(WGET) $(WGETFLAGS) $($@-url)
	cd $(cache_dir) && $(ECHO) "$($@-md5) *$@" | $(MD5SUM) --check
	-$(TAR) --extract $(TARFLAGS) --directory=$(wx_dir) --file=$(cache_dir)/$@

# This archive is dynamically created by github, as of a commit
# specified by the sha1sum embedded in the URL; '--output-document'
# is used to add 'wxWidgets-' to its name. Not being a static file,
# it doesn't bear a historical timestamp corresponding to the commit
# date. See:
#   http://lists.nongnu.org/archive/html/lmi/2015-08/msg00012.html

.PHONY: %.zip
%.zip:
	cd $(cache_dir) && [ -e $@ ] || $(WGET) $(WGETFLAGS) --output-document=$@ $($@-url)
	cd $(cache_dir) && $(ECHO) "$($@-md5) *$@" | $(MD5SUM) --check
	-$(UNZIP) $(cache_dir)/$@ -d $(wx_dir)

.PHONY: wx
wx:
	../configure $(config_options) && $(MAKE) && $(MAKE) install

# 'wx-config' is not portable. For example, it uses 'printf(1)', which
# zsh supports only in versions after 4.0.1 . Far worse, it underlies
# a problem discussed in these messages
#   http://lists.gnu.org/archive/html/lmi/2006-04/msg00010.html
#   http://lists.gnu.org/archive/html/lmi/2006-05/msg00001.html
#   http://lists.gnu.org/archive/html/lmi/2006-05/msg00019.html
#   http://lists.gnu.org/archive/html/lmi/2006-05/msg00021.html
# and extensive offline discussions, which has consumed person-weeks
# of our time; though we can't pinpoint the exact cause, we have never
# encountered any such problem except with 'wx-config'. Therefore, we
# run 'wx-config' only here (in bash, in the present makefile) and
# write the results of the only commands actually used during lmi build
# into a portable script.
#
# Even if a forgiving shell is used, this portable script runs an
# order of magnitude faster than the one wx creates.
# WX !! Is any useful advantage lost?

.PHONY: portable_script
portable_script:
	$(ECHO) '#!/bin/sh'                                              >wx-config-portable
	$(ECHO) 'last_with_arg=0'                                       >>wx-config-portable
	$(ECHO) 'while [ $$# -gt 0 ]; do'                               >>wx-config-portable
	$(ECHO) '    case "$$1" in'                                     >>wx-config-portable
	$(ECHO) '        --basename)'                                   >>wx-config-portable
	$(ECHO) "            echo `./wx-config --basename`"             >>wx-config-portable
	$(ECHO) '            ;;'                                        >>wx-config-portable
	$(ECHO) '        --cflags | --cppflags | --cxxflags)'           >>wx-config-portable
	$(ECHO) "            echo `./wx-config --cxxflags`"             >>wx-config-portable
	$(ECHO) '            this_with_arg=1'                           >>wx-config-portable
	$(ECHO) '            ;;'                                        >>wx-config-portable
	$(ECHO) '        --host*)'                                      >>wx-config-portable
	$(ECHO) '            ;;'                                        >>wx-config-portable
	$(ECHO) '        --libs)'                                       >>wx-config-portable
	$(ECHO) "            echo `./wx-config --libs`"                 >>wx-config-portable
	$(ECHO) '            this_with_arg=1'                           >>wx-config-portable
	$(ECHO) '            ;;'                                        >>wx-config-portable
	$(ECHO) '        --rescomp)'                                    >>wx-config-portable
	$(ECHO) "            echo `./wx-config --rescomp`"              >>wx-config-portable
	$(ECHO) '            ;;'                                        >>wx-config-portable
	$(ECHO) '        --selected_config)'                            >>wx-config-portable
	$(ECHO) "            echo `./wx-config --selected_config`"      >>wx-config-portable
	$(ECHO) '            ;;'                                        >>wx-config-portable
	$(ECHO) '        --version)'                                    >>wx-config-portable
	$(ECHO) "            echo `./wx-config --version`"              >>wx-config-portable
	$(ECHO) '            ;;'                                        >>wx-config-portable
	$(ECHO) '        *)'                                            >>wx-config-portable
	$(ECHO) '            if [ "$$last_with_arg" -ne 1 ]; then'      >>wx-config-portable
	$(ECHO) '                echo Bad argument $$1'                 >>wx-config-portable
	$(ECHO) '                exit 1'                                >>wx-config-portable
	$(ECHO) '            fi'                                        >>wx-config-portable
	$(ECHO) '    esac'                                              >>wx-config-portable
	$(ECHO) '    last_with_arg=$$this_with_arg'                     >>wx-config-portable
	$(ECHO) '    shift'                                             >>wx-config-portable
	$(ECHO) 'done'                                                  >>wx-config-portable
	$(CHMOD) 755 wx-config-portable

.PHONY: clobber
clobber:
# WX !! The 'uninstall' target doesn't remove quite everything.
	-cd $(build_dir) && $(MAKE) uninstall distclean
	-$(RM) --force --recursive $(prefix)/include/wx-$(basename $(wx_version))
	-$(RM) --force --recursive $(prefix)/lib/wx
	-$(RM) --force --recursive $(wx_dir)

