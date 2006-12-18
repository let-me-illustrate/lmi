# Set up build environment.
#
# Copyright (C) 2005, 2006 Gregory W. Chicares.
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

# $Id: setup.make,v 1.30 2006-12-18 22:32:40 wboutin Exp $

.PHONY: all
all: setup

src_dir   := $(CURDIR)
tools_dir := $(src_dir)/tools

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

sf_mirror := http://downloads.sourceforge.net

.PHONY: setup
setup: \
  dummy_libraries \
  frozen_boost \
  frozen_cgicc \
  frozen_libxml2 \
  frozen_make \
  frozen_sed \
  frozen_xmlwrapp \
  mingw_20050827 \
  msys_make \
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

# Install boost-1.33.0 .

.PHONY: frozen_boost
frozen_boost:
	$(MAKE) \
	  --directory=/tmp \
	  --file=$(src_dir)/setup.make \
	                    src_dir='$(src_dir)' \
	    third_party_include_dir='$(third_party_include_dir)' \
	     third_party_source_dir='$(third_party_source_dir)' \
	  install_frozen_boost_from_tmp_dir

.PHONY: install_frozen_boost_from_tmp_dir
install_frozen_boost_from_tmp_dir:
	[ -e boost_1_33_0.tar.bz2 ] \
	  || $(WGET) --non-verbose \
	  $(sf_mirror)/boost/boost_1_33_0.tar.bz2
	$(ECHO) "43d87bbd827a8299f408df5efe5f0fd8  boost_1_33_0.tar.bz2" \
	  |$(MD5SUM) --check
	$(BZIP2) --decompress --keep boost_1_33_0.tar.bz2
	$(TAR) --extract --file=boost_1_33_0.tar
# This safeguards against any older files interfering with new ones
# installed to the same directory.
	$(RM) --force $(third_party_include_dir)/boost/
	$(RM) --force $(third_party_source_dir)/boost/
	$(MKDIR) --parents $(third_party_include_dir)/boost/
	$(MKDIR) --parents $(third_party_source_dir)/boost/
	-$(CP) --force --preserve --recursive boost_1_33_0/boost/* \
	  $(third_party_include_dir)/boost/
	-$(CP) --force --preserve --recursive boost_1_33_0/* \
	  $(third_party_source_dir)/boost/
	$(RM) --force boost_1_33_0.tar boost_1_33_0.tar.bz2

###############################################################################

# Install and patch cgicc-3.1.4 .

# TODO ?? Prefer to define $(TMPDIR) elsewhere and use the definition here.
# Can the definition in 'GNUmakefile' be shared?

# REVIEW: Can 'frozen_.*' rules be combined? Untested idea:
# frozen%: $(third_party_directories)
# 	$(MAKE) \
# 	  --directory=/tmp \
# 	  --file=$(src_dir)/setup.make \
# 	                    src_dir='$(src_dir)' \
# 	    third_party_include_dir='$(third_party_include_dir)' \
# 	     third_party_source_dir='$(third_party_source_dir)' \
# [similarly pass other '.*_dir' variables?]
# 	  install_$@_from_tmp_dir

.PHONY: frozen_cgicc
frozen_cgicc:
	$(MAKE) \
	  --directory=/tmp \
	  --file=$(src_dir)/setup.make \
	                    src_dir='$(src_dir)' \
	    third_party_include_dir='$(third_party_include_dir)' \
	     third_party_source_dir='$(third_party_source_dir)' \
                          tools_dir='$(tools_dir)' \
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
	$(PATCH) --strip=0 < $(tools_dir)/cgicc/cgicc_3_1_4_patch
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
	$(MD5SUM) --check $(tools_dir)/cgicc/cgicc_md5sums

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

###############################################################################

# Install make-3.81 .

.PHONY: frozen_make
frozen_make:
	$(MAKE) \
	  --directory=/tmp \
	  --file=$(src_dir)/setup.make \
            src_dir='$(src_dir)' \
	  install_frozen_make_from_tmp_dir

.PHONY: install_frozen_make_from_tmp_dir
install_frozen_make_from_tmp_dir:
	[ -e make-3.81.tar.bz2 ] \
	  || $(WGET) --non-verbose \
	  http://ftp.gnu.org/gnu/make/make-3.81.tar.bz2
	$(ECHO) "354853e0b2da90c527e35aabb8d6f1e6  make-3.81.tar.bz2" \
	  |$(MD5SUM) --check
	$(BZIP2) --decompress --keep make-3.81.tar.bz2
	$(TAR) --extract --file=make-3.81.tar
	cd make-3.81; \
	/msys/1.0/bin/sh.exe ./configure && /msys/1.0/bin/make
# TODO ?? Apparently, because make is being used for this, it cannot
# be overwritten, so an alternative should be considered.
	$(CP) --force --preserve $(CURDIR)/make-3.81/make.exe /usr/bin/
	$(RM) --force make-3.81.tar make-3.81.tar.bz2

# This updates make in MSYS.
.PHONY: msys_make
msys_make:
	$(MAKE) \
	  --directory=/tmp \
	  --file=$(src_dir)/setup.make \
            src_dir='$(src_dir)' \
	  install_msys_make_from_tmp_dir

.PHONY: install_msys_make_from_tmp_dir
install_msys_make_from_tmp_dir:
	[ -e make-3.81-MSYS-1.0.11-snapshot.tar.bz2 ] \
	  || $(WGET) --non-verbose \
	  $(sf_mirror)/mingw/make-3.81-MSYS-1.0.11-snapshot.tar.bz2
	$(ECHO) "d2540add01b0b232722b0d358c6ee1ef  make-3.81-MSYS-1.0.11-snapshot.tar.bz2" \
	  |$(MD5SUM) --check
	$(BZIP2) --decompress --keep make-3.81-MSYS-1.0.11-snapshot.tar.bz2
	$(TAR) --extract --file=make-3.81-MSYS-1.0.11-snapshot.tar
	$(CP) --force --preserve $(CURDIR)/make-3.81/bin/make.exe /msys/1.0/bin/
	$(RM) --force make-3.81-MSYS-1.0.11-snapshot.tar \
	  make-3.81-MSYS-1.0.11-snapshot.tar.bz2

###############################################################################

# Install sed-4.0.7 .

# There are several later versions. Building sed-4.1.4 in MSYS fails
# for several reasons:
#  - it expects some natural-language support that's not present
#      workaround: "./configure --disable-nls"
#  - it expects to find 'bcopy'
#      workaround: "make CPPFLAGS='-DHAVE_STRING_H'"
#  - it can't resolve multibyte-character functions
#      workaround: suppress "#define HAVE_MBRTOWC 1" in config.h
# Those seem to be problems with MSYS or autotools, except that
# the second one looks like a defect in the sed sources (a file
# that uses 'bcopy' doesn't seem to include config.h).

.PHONY: frozen_sed
frozen_sed:
	$(MAKE) \
	  --directory=/tmp/ \
	  --file=$(src_dir)/setup.make \
	  src_dir='$(src_dir)' \
          install_sed_from_tmp_dir

.PHONY: install_sed_from_tmp_dir
install_sed_from_tmp_dir:
	[ -e sed-4.0.7.tar.gz ] \
	  || $(WGET) --non-verbose \
	  http://ftp.gnu.org/gnu/sed/sed-4.0.7.tar.gz
	$(ECHO) " 005738e7f97bd77d95b6907156c8202a  sed-4.0.7.tar.gz " \
	  |$(MD5SUM) --check
	$(GZIP) --decompress --force sed-4.0.7.tar.gz
	$(TAR) --extract --file sed-4.0.7.tar
	cd /tmp/sed-4.0.7/ ; \
	/msys/1.0/bin/sh.exe ./configure && /msys/1.0/bin/make
	-$(CP) --force --preserve sed-4.0.7/sed/sed.exe /usr/bin/
	$(RM) --recursive sed-4.0.7.tar

###############################################################################

# Install and patch xmlwrapp-0.5.0 .

.PHONY: frozen_xmlwrapp
frozen_xmlwrapp:
	$(MAKE) \
	  --directory=/tmp \
	  --file=$(src_dir)/setup.make \
	                    src_dir='$(src_dir)' \
	    third_party_include_dir='$(third_party_include_dir)' \
	     third_party_source_dir='$(third_party_source_dir)' \
                          tools_dir='$(tools_dir)' \
	  install_frozen_xmlwrapp_from_tmp_dir

.PHONY: install_frozen_xmlwrapp_from_tmp_dir
install_frozen_xmlwrapp_from_tmp_dir:
	[ -e xmlwrapp-0.5.0.tar.gz ] \
	  || $(WGET) --non-verbose \
          http://www.mirrorservice.org/sites/ftp.freebsd.org/pub/FreeBSD/distfiles/xmlwrapp-0.5.0.tar.gz
	$(ECHO) "b8a07e77f8f8af9ca96bccab7d9dd310  xmlwrapp-0.5.0.tar.gz" \
	  |$(MD5SUM) --check
	$(GZIP) --decompress xmlwrapp-0.5.0.tar.gz
	$(TAR) --extract --verbose --file=xmlwrapp-0.5.0.tar
	$(RM) --force --recursive $(third_party_include_dir)/xmlwrapp/
	$(RM) --force --recursive $(third_party_source_dir)/libxml/
	$(MKDIR) --parents $(third_party_include_dir)/xmlwrapp/
	$(MKDIR) --parents $(third_party_source_dir)/libxml/
	$(CP) --preserve xmlwrapp-0.5.0/include/xmlwrapp/*.h \
	  $(third_party_include_dir)/xmlwrapp/
	$(CP) --preserve xmlwrapp-0.5.0/src/libxml/* \
	  $(third_party_source_dir)/libxml/
	$(RM) --force xmlwrapp-0.5.0.tar xmlwrapp-0.5.0.tar.gz

###############################################################################

# Install MinGW.

# The required packages contain some duplicate files, so the order of
# extraction is important. It follows these instructions:
#   http://groups.yahoo.com/group/mingw32/message/1145
# It is apparently fortuitous that the order is alphabetical.

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

# REVIEW: The $(CP) commands below place all original archives
# in $(mingw_dir). Aren't there some that belong elsewhere, at
# least if these targets are reused for other archives? And are
# the $(CP) flags optimal, or would it be better to use
# '--update'?

# TODO ?? Downloaded files should be validated before extracting.
%.tar.bz2:
	[ -e $@ ] || $(WGET) --non-verbose $(sf_mirror)/mingw/$@
	$(MD5SUM) --check $@.md5
	$(CP) --force --preserve $@ $(mingw_dir)
	$(BZIP2) --decompress --keep --force $@
	$(TAR) --extract --file=$*.tar

%.tar.gz:
	[ -e $@ ] || $(WGET) --non-verbose $(sf_mirror)/mingw/$@
	$(MD5SUM) --check $@.md5
	$(CP) --force --preserve $@ $(mingw_dir)
	$(GZIP) --decompress --force $@
	$(TAR) --extract --file=$*.tar

# This target must be kept synchronized with the latest packages
# available from http://www.mingw.org/download.shtml .
# REVIEW: "latest": does that mean "current" or "candidate"?

# REVIEW: It's nice to echo the advice to preserve a prior MinGW
# installation, but doesn't this print that message even if there is
# no prior installation? Did you intend the test on the immediately
# preceding line to control the echo command? If not, what does that
# test accomplish? And wouldn't it be better to detect a prior
# installation and either halt with an error message, or move the
# prior installation and echo its new location?

.PHONY: mingw_current
mingw_current:
	$(MKDIR) --parents /tmp/$@
	-@[ -e $(mingw_dir) ]
	@$(ECHO) "Preserving your existing MinGW installation is\
	  strongly recommended."
	$(MKDIR) $(mingw_dir)
	$(MAKE) \
	  --directory=/tmp/$@ \
	  --file=$(src_dir)/setup.make \
	    mingw_dir='$(mingw_dir)' \
	      src_dir='$(src_dir)' \
	  install_mingw_current_from_tmp_dir

.PHONY: install_mingw_current_from_tmp_dir
install_mingw_current_from_tmp_dir: $(mingw_requirements) $(mingw_extras)
	-$(RM) --recursive *.tar
	$(MV) --force * $(mingw_dir)

# REVIEW: Doesn't the following comment belong at the top of this section?

# Minimal installation for building lmi with MinGW.

.PHONY: mingw_20050827
mingw_20050827:
	$(MKDIR) --parents /tmp/$@
	$(MAKE) \
	  --directory=/tmp/$@ \
	  --file=$(src_dir)/setup.make \
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

# This target uses wget to install wget. That may seem silly on the
# face of it, but it's actually useful for upgrading to a later
# version of wget.

wget_mingwport = wget-1.9.1

.PHONY: wget_mingwport
wget_mingwport:
	$(MAKE) \
	  --directory=/tmp \
	  --file=$(src_dir)/setup.make \
	    mingw_dir='$(mingw_dir)' \
	      src_dir='$(src_dir)' \
	  install_wget_mingwport_from_tmp_dir

.PHONY: install_wget_mingwport_from_tmp_dir
install_wget_mingwport_from_tmp_dir:
	$(WGET) --non-verbose $(sf_mirror)/mingw/$(wget_mingwport)-mingwPORT.tar.bz2
	$(BZIP2) --decompress --force --keep $(wget_mingwport)-mingwPORT.tar.bz2
	$(TAR) --extract --file $(wget_mingwport)-mingwPORT.tar
	$(CP) --preserve $(wget_mingwport)/mingwPORT/wget.exe /usr/bin/
	$(CP) --preserve $(wget_mingwport)/mingwPORT/wget.exe /msys/1.0/local/bin/

# REVIEW: Can't the last command above fail if no local/bin/
# subdirectory already exists in /msys/1.0/ ? I think that can easily
# happen even though MSYS puts /usr/local/bin/ in $PATH. Is this the
# exact place where Earnie recommends installing it? It might help to
# explain why we're installing it in two places.

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
	  --directory=/tmp \
	  --file=$(src_dir)/setup.make \
	    src_dir='$(src_dir)' \
	  install_human_interactive_tools_from_tmp_dir

.PHONY: install_human_interactive_tools_from_tmp_dir
install_human_interactive_tools_from_tmp_dir: $(human_interactive_tools)

###############################################################################

# Validate setup files.

.PHONY: test_setup
test_setup: check_cgicc_md5sums

