# MinGW installation.
#

# REVIEW: We don't need to copyright this file. Instead, use this:
#
# This is public domain. It's not 'Copyright 2005', but including that
# quoted string keeps our cvs style-conformity script from complaining.

# REVIEW: This shouldn't be GPL, so these license terms can be deleted:

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

# REVIEW: [End of block to be deleted.] Now you can leave the webpage
# as is, but change the email address to your own, or to the MinGW
# mailing list. No snail-mail address is needed.

# http://savannah.nongnu.org/projects/lmi
# email: <chicares@cox.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

# $Id: mingw_setup.make,v 1.3 2005-11-04 13:44:14 chicares Exp $

# REVIEW: Here, say exactly what this makefile does, and what its
# prerequisites are; and write some excruciatingly clear instructions
# for running it.

# This makefile is self-contained, which is intended to make a MinGW
# community review easy. Some overlap now exists with the unified lmi
# makefile system, but that seems justified for the purpose of
# providing a MinGW installation for those not involved with lmi.

# REVIEW: Is there any actual need for 'setup' in addition to 'all'?
# The default target is the first encountered, and by convention it's
# normally named 'all'.

.PHONY: all
all: setup

src_dir := $(CURDIR)

# REVIEW: The following variables are customizable, so perhaps you
# ought to say that and group them all in one section.

system_root := C:

###############################################################################

# Path to compiler's root directory, i.e., bin/.. .
mingw_dir  := $(system_root)/MinGW

###############################################################################

# Standard utilities.

# REVIEW: First, in our own local toolset, let's copy 'wget.exe' to
# /usr/bin/ and, I guess, /gnu/ too. Then we don't need
# $(msys_utils_dir), right? At least we don't need it here, so let's
# remove it hence. Then this file would look for all utilities in
# /gnu/ ; let's use /usr/bin/ instead, because it's standard and
# /gnu/ is just plain weird. (The same applies to all lmi makefiles
# I'd guess.)
#
# Then, consider how someone on the mailing list might use this.
# Would they have MSYS installed already? Probably--or else they
# could easily install it. That's the likeliest environment for
# them to have, so let's default to that: that means setting the
# root directory to '/c'.

gnu_utils_dir  := $(system_root)/gnu
msys_utils_dir := $(system_root)/msys/1.0/bin

# REVIEW: I don't think these FHS distinctions are relevant to
# the intended audience, so a single alphabetized list seems
# preferable.

# REVIEW: What happens if a required utility is unavailable?
# Would it make sense to test each, e.g.,
#   $(CP) --version >/dev/null 2>&1
# ?

# Required in /bin (if anywhere) by FHS-2.2 .

CP     := $(gnu_utils_dir)/cp
ECHO   := $(gnu_utils_dir)/echo
MKDIR  := $(gnu_utils_dir)/mkdir
MV     := $(gnu_utils_dir)/mv
RM     := $(gnu_utils_dir)/rm

# FHS-2.2 would put these in /usr/bin .

BZIP2  := $(gnu_utils_dir)/bzip2
GZIP   := $(gnu_utils_dir)/gzip
# REVIEW: $(MD5SUM) is never used, but should be, right?
MD5SUM := $(gnu_utils_dir)/md5sum
TAR    := $(gnu_utils_dir)/tar
WGET   := $(msys_utils_dir)/wget

###############################################################################

# REVIEW: This is configurable. Prefer to assign configurable
# variables in one marked section, with appropriate instructions.

sf_mirror := http://umn.dl.sourceforge.net/sourceforge

.PHONY: setup
setup: \
  mingw_current \
  human_interactive_setup \

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

# REVIEW: Shouldn't all issues marked 'TODO \?\?' be resolved before
# this is shared with the MinGW community?

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

# This target must be kept synchronized with the latest packages
# available from http://www.mingw.org/download.shtml .
#
# REVIEW: "latest": does that mean "current" or "candidate"?

# REVIEW: Why not use the self-documenting long forms of '-C' and '-f'?

# REVIEW: How do 'mingw_current' and 'mingw_20050827' differ? Would
# it be better to have a series of targets like
#   mingw_[some later date]
#   mingw_20050827
#   mingw_[some earlier date]
#   mingw_[some even earlier date]
# and make 'mingw_current' an alias for the latest? Could the
# differences be factored out so that there are multiple dated
# versions of $(mingw_requirements) but only one target?

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

# REVIEW: Doesn't the following comment belong at the top of this section?
# And is lmi at all relevant to the intended audience?

# Minimal installation for building lmi with MinGW.

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

# Installation programs that require human interaction.

human_interactive_tools = \
  gdb-5.2.1-1.exe \
  MSYS-1.0.10.exe \

# REVIEW: In what directory would the downloaded file be saved?
# Can't we prevent downloading it if it was already downloaded?

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

# This target uses wget to install wget. That may seem silly on the
# face of it, but it's actually useful for upgrading to a later
# version of wget.

wget_mingwport = wget-1.9.1

.PHONY: wget_mingwport
wget_mingwport:
	$(MAKE) \
	  -C /tmp \
	  -f $(src_dir)/setup.make \
	    mingw_dir='$(mingw_dir)' \
	      src_dir='$(src_dir)' \
	  install_wget_mingwport_from_tmp_dir

# REVIEW: Would the intended audience wish to copy this binary to
# two different directories?

.PHONY: install_wget_mingwport_from_tmp_dir
install_wget_mingwport_from_tmp_dir:
	$(WGET) --non-verbose $(sf_mirror)/mingw/$(wget_mingwport)-mingwPORT.tar.bz2
	$(BZIP2) --decompress --force --keep $(wget_mingwport)-mingwPORT.tar.bz2
	$(TAR) --extract --file $(wget_mingwport)-mingwPORT.tar
	$(CP) --preserve $(wget_mingwport)/mingwPORT/wget.exe /usr/bin/
	$(CP) --preserve $(wget_mingwport)/mingwPORT/wget.exe /msys/1.0/bin/

# REVIEW: How have you tested this makefile? How do you know that it's
# correct? How could you tell that the installation has succeeded? Can
# things like this be tested automatically? How do you know that no
# existing file is downloaded unnecessarily?

