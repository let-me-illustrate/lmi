# MinGW installation.
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

# $Id: mingw_setup.make,v 1.2 2005-10-31 15:03:52 wboutin Exp $

# This makefile is self-contained, which is intended to make a MinGW
# community review easy. Some overlap now exists with the unified lmi
# makefile system, but that seems justified for the purpose of
# providing a MinGW installation for those not involved with lmi.

.PHONY: all
all: setup

src_dir := $(CURDIR)

system_root := C:

###############################################################################

# Path to compiler's root directory, i.e. bin/.. .
mingw_dir  := $(system_root)/MinGW

###############################################################################

# Standard utilities.

gnu_utils_dir  := $(system_root)/gnu
msys_utils_dir := $(system_root)/msys/1.0/bin

# Required in /bin (if anywhere) by FHS-2.2 .

CP     := $(gnu_utils_dir)/cp
ECHO   := $(gnu_utils_dir)/echo
MKDIR  := $(gnu_utils_dir)/mkdir
MV     := $(gnu_utils_dir)/mv
RM     := $(gnu_utils_dir)/rm

# FHS-2.2 would put these in /usr/bin .

BZIP2  := $(gnu_utils_dir)/bzip2
GZIP   := $(gnu_utils_dir)/gzip
MD5SUM := $(gnu_utils_dir)/md5sum
TAR    := $(gnu_utils_dir)/tar
WGET   := $(msys_utils_dir)/wget

###############################################################################

sf_mirror := http://umn.dl.sourceforge.net/sourceforge

.PHONY: setup
setup: \
  mingw_current \
  human_interactive_setup \

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
	  -f $(src_dir)/mingw_setup.make \
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
	-@[ -e $(mingw_dir) ]
	@$(ECHO) "Preserving your existing MinGW installation is\
	strongly recommended."
	$(MKDIR) $(mingw_dir)
	$(MAKE) \
	  -C /tmp/$@ \
	  -f $(src_dir)/mingw_setup.make \
	    mingw_dir='$(mingw_dir)' \
	      src_dir='$(src_dir)' \
	  install_mingw_20050827_from_tmp_dir

.PHONY: install_mingw_20050827_from_tmp_dir
install_mingw_20050827_from_tmp_dir: $(mingw_requirements)
	-$(RM) --recursive *.tar
	$(MV) --force * $(mingw_dir)

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
	  -f $(src_dir)/mingw_setup.make \
	    src_dir='$(src_dir)' \
	  install_human_interactive_tools_from_tmp_dir

.PHONY: install_human_interactive_tools_from_tmp_dir
install_human_interactive_tools_from_tmp_dir: $(human_interactive_tools)

###############################################################################

# Upgrade wget-1.9.1.tar.bz2 .

# This is only for users to upgrade this tool because it relies on itself to
# fetch the new archive.

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

