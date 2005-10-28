# Set up build environment.
#
# Copyright (C) 2005 Gregory W. Chicares.
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
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# http://savannah.nongnu.org/projects/lmi
# email: <chicares@cox.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

# $Id: setup.make,v 1.19 2005-10-28 18:26:06 wboutin Exp $

.PHONY: all
all: setup

src_dir := $(CURDIR)

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

sf_mirror := http://umn.dl.sourceforge.net/sourceforge

.PHONY: setup
setup: \
  dummy_libraries \
  frozen_cgicc \
  frozen_xmlwrapp \
  frozen_boost \
  frozen_libxml2 \
  mingw_current \
  test_setup \

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

# Create dummy libraries.

# At least with gnu tools, a zero-byte file may be linked with no
# diagnostic (and no effect, which is the intention).

# Create dummy libraries for boost and xmlwrapp: see discussion of
# 'third-party' libraries in other makefiles. For boost, derive
# library names from the platform-dependent linker option, because
# the names of boost libraries incorporate a platform suffix.

# Prevent license issues by providing dummy versions of problematic
# libraries that wx builds by default.
#
# TODO ?? Apparently libregex.a is no longer a problem since wx-2.5.0,
# and it's not clear why zlib was ever on this list. Consider removing
# those libraries from this list after carefully reauditing the
# licenses of all libraries wx uses.

dummy_library_names := \
  $(patsubst -l%,lib%.a,$(platform_boost_libraries)) \
  libjpeg.a \
  libpng.a \
  libregex.a \
  libtiff.a \
  libxmlwrapp.a \
  libzlib.a \

.PHONY: dummy_libraries
dummy_libraries: $(third_party_bin_dir) $(third_party_lib_dir)
	@$(foreach z, $(dummy_library_names), \
	  $(TOUCH) --date=20000101 $(third_party_lib_dir)/$(z); \
	  )

###############################################################################

# Install and patch cgicc-3.1.4 .

# TODO ?? Prefer to define $(TMPDIR) elsewhere and use the definition here.
# Can the definition in 'GNUmakefile' be shared?

# REVIEW: Can 'frozen_.*' rules be combined? Untested idea:
# frozen%: $(third_party_directories)
# 	$(MAKE) \
# 	  -C /tmp \
# 	  -f $(src_dir)/setup.make \
# 	                    src_dir='$(src_dir)' \
# 	    third_party_include_dir='$(third_party_include_dir)' \
# 	     third_party_source_dir='$(third_party_source_dir)' \
# [similarly pass other '.*_dir' variables?]
# 	  install_$@_from_tmp_dir

.PHONY: frozen_cgicc
frozen_cgicc:
	$(MAKE) \
	  -C /tmp \
	  -f $(src_dir)/setup.make \
	                    src_dir='$(src_dir)' \
	    third_party_include_dir='$(third_party_include_dir)' \
	     third_party_source_dir='$(third_party_source_dir)' \
	  install_frozen_cgicc_from_tmp_dir check_cgicc_md5sums

# TODO ?? Make this target abend if it's not run in /tmp/ ?

.PHONY: install_frozen_cgicc_from_tmp_dir
install_frozen_cgicc_from_tmp_dir:
	[ -e cgicc-3.1.4.tar.bz2 ] \
          || $(WGET) --non-verbose \
          ftp://ftp.gnu.org/pub/gnu/cgicc/cgicc-3.1.4.tar.bz2
	$(ECHO) "6cb5153fc9fa64b4e50c7962aa557bbe  cgicc-3.1.4.tar.bz2" \
	  |$(MD5SUM) --check
	$(BZIP2) --decompress --keep cgicc-3.1.4.tar.bz2
	$(TAR) --extract --file=cgicc-3.1.4.tar
	$(PATCH) --strip=0 < $(src_dir)/cgicc_3_1_4_patch
	$(MKDIR) --parents $(third_party_include_dir)/cgicc/
	$(MKDIR) --parents $(third_party_source_dir)/cgicc/
	$(CP) --preserve cgicc-3.1.4/cgicc/*.h \
	  $(third_party_include_dir)/cgicc/
	$(CP) --preserve cgicc-3.1.4/cgicc/*.cpp \
	  $(third_party_source_dir)/cgicc/
	$(RM) --force cgicc-3.1.4.tar cgicc-3.1.4.tar.bz2

.PHONY: check_cgicc_md5sums
check_cgicc_md5sums: $(third_party_dir)
	cd $(third_party_dir); \
	$(MD5SUM) --check $(src_dir)/cgicc_md5sums

###############################################################################

# Install and patch xmlwrapp-0.2.0 .

.PHONY: frozen_xmlwrapp
frozen_xmlwrapp:
	$(MAKE) \
	  -C /tmp \
	  -f $(src_dir)/setup.make \
	                    src_dir='$(src_dir)' \
	    third_party_include_dir='$(third_party_include_dir)' \
	     third_party_source_dir='$(third_party_source_dir)' \
	  install_frozen_xmlwrapp_from_tmp_dir check_xmlwrapp_md5sums

.PHONY: install_frozen_xmlwrapp_from_tmp_dir
install_frozen_xmlwrapp_from_tmp_dir:
# This command won't work because there's no longer a valid URL for the
# version currently used in production:
# 	|| $(WGET) --non-verbose \
# 	http://pmade.org/software/xmlwrapp/download/xmlwrapp-0.2.0.tar.gz

# The following assumes 'xmlwrapp-0.2.0.tar.gz' exists in '/tmp/' already.
	[ -e xmlwrapp-0.2.0.tar.gz ]
	$(ECHO) "f142e8bc349597ecbaebb4a8e246b65a  xmlwrapp-0.2.0.tar.gz" \
	  |$(MD5SUM) --check
	$(GZIP) --decompress xmlwrapp-0.2.0.tar.gz
	$(TAR) --extract --verbose --file=xmlwrapp-0.2.0.tar
	$(PATCH) --strip=0 < $(src_dir)/xmlwrapp_0_2_0_patch
	$(MKDIR) --parents $(third_party_include_dir)/xmlwrapp/
	$(MKDIR) --parents $(third_party_source_dir)/libxml/
	$(CP) --preserve xmlwrapp-0.2.0/include/xmlwrapp/*.h \
	  $(third_party_include_dir)/xmlwrapp/
	$(CP) --preserve xmlwrapp-0.2.0/src/libxml/* \
	  $(third_party_source_dir)/libxml/
	$(RM) --force xmlwrapp-0.2.0.tar xmlwrapp-0.2.0.tar.gz

.PHONY: check_xmlwrapp_md5sums
check_xmlwrapp_md5sums: $(third_party_dir)
	cd $(third_party_dir) ; \
	$(MD5SUM) --check $(src_dir)/xmlwrapp_md5sums

###############################################################################

# Install boost-1.31.0 .

.PHONY: frozen_boost
frozen_boost:
	$(MAKE) \
	  -C /tmp \
	  -f $(src_dir)/setup.make \
	                    src_dir='$(src_dir)' \
	    third_party_include_dir='$(third_party_include_dir)' \
	     third_party_source_dir='$(third_party_source_dir)' \
	  install_frozen_boost_from_tmp_dir

.PHONY: install_frozen_boost_from_tmp_dir
install_frozen_boost_from_tmp_dir:
	[ -e boost_1_31_0.tar.bz2 ] \
	  || $(WGET) --non-verbose \
	  $(sf_mirror)/boost/boost_1_31_0.tar.bz2
	$(ECHO) "8cc183538eaa5cfc53d88d0e94bd2fd4  boost_1_31_0.tar.bz2" \
	  |$(MD5SUM) --check
	$(BZIP2) --decompress --keep boost_1_31_0.tar.bz2
	$(TAR) --extract --file=boost_1_31_0.tar
	$(MKDIR) --parents $(third_party_include_dir)/boost/
	$(MKDIR) --parents $(third_party_source_dir)/boost/
	-$(CP) --force --preserve --recursive boost_1_31_0/boost/* \
	  $(third_party_include_dir)/boost/
	-$(CP) --force --preserve --recursive boost_1_31_0/* \
	  $(third_party_source_dir)/boost/
	$(RM) --force boost_1_31_0.tar boost_1_31_0.tar.bz2

###############################################################################
# This version has not been formally tested and released for production with
# lmi. The version actually used can't be easily rebuilt, so this target is
# intended to provide a stop-gap library until it can be formally tested.

# Install libxml2-2.6.19 .

.PHONY: frozen_libxml2
frozen_libxml2:
	$(MAKE) \
	  -C /tmp \
	  -f $(src_dir)/setup.make \
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
	cd libxml2-2.6.19; ./configure && $(MAKE)
	$(MKDIR) --parents $(third_party_include_dir)/libxml/
	-$(CP) --force --preserve --recursive libxml2-2.6.19/include/libxml/* \
	  $(third_party_include_dir)/libxml/ 2>/dev/null
	$(CP) --force --preserve libxml2-2.6.19/.libs/libxml2-2.dll \
	  $(third_party_bin_dir)
	$(CP) --force --preserve libxml2-2.6.19/.libs/libxml2.dll.a \
	  $(third_party_lib_dir)
	$(RM) --force libxml2-2.6.19.tar libxml2-2.6.19.tar.bz2

###############################################################################

# Install MinGW.

# Keep this order for best results, as discussed here:
# http://groups.yahoo.com/group/mingw32/message/1145

mingw_requirements = \
  binutils-2.16.91-20050827-1.tar.gz \
  gcc-core-3.4.4-20050522-1.tar.gz \
  gcc-g++-3.4.4-20050522-1.tar.gz \
  mingw-runtime-3.8.tar.gz \
  w32api-3.3.tar.gz \

# These aren't necessary for compiling a C++ program with <windows.h>.
# Although, they're included here in an attempt to reproduce a C++-only
# version of MinGW-5.0.0 .

mingw_extras = \
  mingw32-make-3.80.0-3.tar.gz \
  mingw-utils-0.3.tar.gz \

# Download archives if they're out of date, then extract them.

# TODO ?? Downloaded files should be validated before extracting.
%.tar.bz2:
	[ -e $@ ] || $(WGET) --non-verbose $(sf_mirror)/mingw/$@
	$(CP) --force --preserve $@ $(mingw_dir)
	$(BZIP2) --decompress --keep --force $@
	$(TAR) --extract --file=$*.tar

%.tar.gz:
	[ -e $@ ] || $(WGET) --non-verbose $(sf_mirror)/mingw/$@
	$(CP) --force --preserve $@ $(mingw_dir)
	$(GZIP) --decompress --force $@
	$(TAR) --extract --file=$*.tar

# This target is intended to always be synchronized with the latest
# packages found at http://www.mingw.org/download.shtml .

.PHONY: mingw_current
mingw_current:
	$(MKDIR) --parents /tmp/$@
	-@[ -e $(mingw_dir) ]
	@$(ECHO) "Preserving your existing MinGW installation is\
	strongly recommended."
	$(MKDIR) $(mingw_dir)
	$(MAKE) \
	  -C /tmp/$@ \
	  -f $(src_dir)/setup.make \
	    mingw_dir='$(mingw_dir)' \
	      src_dir='$(src_dir)' \
	  install_mingw_current_from_tmp_dir

.PHONY: install_mingw_current_from_tmp_dir
install_mingw_current_from_tmp_dir: $(mingw_requirements) $(mingw_extras)
	-$(RM) --recursive *.tar
	$(MV) --force * $(mingw_dir)

# This provides the minimum requirements for building lmi with this compiler.

.PHONY: mingw_20050827
mingw_20050827:
	$(MKDIR) --parents /tmp/$@
	$(MAKE) \
	  -C /tmp/$@ \
	  -f $(src_dir)/setup.make \
	    mingw_dir='$(mingw_dir)' \
	      src_dir='$(src_dir)' \
	  install_mingw_20050827_from_tmp_dir

.PHONY: install_mingw_20050827_from_tmp_dir
install_mingw_20050827_from_tmp_dir: $(mingw_requirements)
	$(RM) --recursive *.tar
	$(MKDIR) --parents $(mingw_dir)
	-$(CP) --force --parents --preserve --recursive * $(mingw_dir)
# TODO ?? Keep this command blocked during testing to allow comparison
# with sibling target.
#	$(RM) --recursive *

###############################################################################

# Upgrade wget-1.9.1.tar.bz2 .

# This is only for users to upgrade this tool because it relies on itself
# to fetch the new archive.

wget_mingwport = wget-1.9.1

.PHONY: wget_mingwport
wget_mingwport:
	$(MAKE) \
	  -C /tmp \
	  -f $(src_dir)/setup.make \
	    mingw_dir='$(mingw_dir)' \
	      src_dir='$(src_dir)' \
	  install_wget_mingwport_from_tmp_dir

.PHONY: install_wget_mingwport_from_tmp_dir
install_wget_mingwport_from_tmp_dir:
	$(WGET) --non-verbose $(sf_mirror)/mingw/$(wget_mingwport)-mingwPORT.tar.bz2
	$(BZIP2) --decompress --force --keep $(wget_mingwport)-mingwPORT.tar.bz2
	$(TAR) --extract --file $(wget_mingwport)-mingwPORT.tar
	$(CP) --preserve $(wget_mingwport)/mingwPORT/wget.exe /usr/bin/
	$(CP) --preserve $(wget_mingwport)/mingwPORT/wget.exe /msys/1.0/bin/

###############################################################################

# Installation tools requiring human interaction.

human_interactive_tools = \
  gdb-5.2.1-1.exe \
  MSYS-1.0.10.exe

%.exe:
	$(WGET) --non-verbose $(sf_mirror)/mingw/$@
	./$@

.PHONY: human_interactive_setup
human_interactive_setup:
	$(MAKE) \
	  -C /tmp \
	  -f $(src_dir)/setup.make \
	    src_dir='$(src_dir)' \
	  install_human_interactive_tools_from_tmp_dir

.PHONY: install_human_interactive_tools_from_tmp_dir
install_human_interactive_tools_from_tmp_dir: $(human_interactive_tools)

###############################################################################

# Validate setup files.

.PHONY: test_setup
test_setup: check_cgicc_md5sums check_xmlwrapp_md5sums

