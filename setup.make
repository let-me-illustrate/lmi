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

# $Id: setup.make,v 1.36 2007-11-06 02:20:16 chicares Exp $

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

# Current versions used in production.
# TODO ?? Test these thoroughly before moving above. For now, they
# show tools' versions at a glance.
#
# gdb      := gdb-5.2.1-1
# libxml2  := libxml2-2.6.19
# make     := make-3.81
# msys     := MSYS-1.0.10
# sed      := sed-4.0.7
# xmlwrapp := xmlwrapp-0.5.0
# wget     := wget-1.9.1

.PHONY: setup
setup: \
  dummy_libraries \
  frozen_libxml2 \
  frozen_make \
  frozen_sed \
  frozen_xmlwrapp \
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

# Prevent license issues by providing dummy versions of problematic
# libraries that wx builds by default.
#
# TODO ?? Apparently libregex.a is no longer a problem since wx-2.5.0,
# and it's not clear why zlib was ever on this list. Consider removing
# those libraries from this list after carefully reauditing the
# licenses of all libraries wx uses.

dummy_library_names := \
  libjpeg.a \
  libpng.a \
  libregex.a \
  libtiff.a \
  libzlib.a \

.PHONY: dummy_libraries
dummy_libraries: $(third_party_bin_dir) $(third_party_lib_dir)
	@$(foreach z, $(dummy_library_names), \
	  $(TOUCH) --date=20000101 $(third_party_lib_dir)/$(z); \
	  )

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

# Install xmlwrapp-0.5.0 .

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

