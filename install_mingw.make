# Installer for MinGW-w64 native toolchain.
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

# Default version. There's no terse and unambiguous way to specify
# versions, as the MinGW-w64 project hosts other projects' builds
# rather than release its own; lmi uses i686 builds with native
# threads and SJLJ exceptions.

version   := MinGW-4_9_1

file_list  = $($(version))

# Prefer to set $(prefix) to anything but '/mingw', in order to avoid
# the problem described here:
#   http://gcc.gnu.org/ml/gcc-patches/2004-06/msg00703.html
# when multiple versions of MinGW gcc are installed, as discussed on
# the mingw-users mailing list in these messages:
#   http://article.gmane.org/gmane.comp.gnu.mingw.user/14747
#     [2005-01-17T16:30:44Z from Greg Chicares]
#   http://article.gmane.org/gmane.comp.gnu.mingw.user/14748
#     [2005-01-17T18:15:26Z from Aaron W. LaFramboise]

prefix    := /MinGW_

cache_dir := /cache_for_lmi/downloads

# In the past, it seemed necessary to specify a mirror, e.g.:
#  mirror := http://easynews.dl.sourceforge.net/sourceforge/mingw
# but as of about 2006-12 sf.net seems to select one automatically
# when this is passed to wget:
mirror    := http://downloads.sourceforge.net/mingw-w64

# File lists ###################################################################

# MinGW-w64 git c6f0d3d981c70ad31bb1c2bfc2850b827281e189
MinGW-4_9_1 := i686-4.9.1-release-win32-sjlj-rt_v3-rev3.7z

# Archive md5sums ##############################################################

$(MinGW-4_9_1)-md5 := b9911d63b9c4c57d17f356460a2b0135

# Utilities ####################################################################

BSDTAR := bsdtar
ECHO   := echo
MD5SUM := md5sum
MKDIR  := mkdir
MV     := mv
RM     := rm
RMDIR  := rmdir
WGET   := wget

# Error messages ###############################################################

wget_missing = \
  "\nError: Unable to find '$(WGET)', which is required for" \
  "\nautomated downloads. Download it manually from the MinGW" \
  "\nsite and install it on your PATH." \
  "\n"

prefix_exists = \
  "\nError: Prefix directory '$(prefix)' already exists." \
  "\nIt is generally unsafe to install one version of a program" \
  "\non top of another. Probably you ought to rename the old" \
  "\nversion in order to preserve it; if not, then remove it." \
  "\n"

scratch_exists = \
  "\nError: Scratch directory 'scratch' already exists." \
  "\nProbably it is left over from a previous failure." \
  "\nJust remove it unless you're sure you want whatever files" \
  "\nit might contain." \
  "\n"

# Targets ######################################################################

.PHONY: all
all: $(file_list)
	$(MV) scratch/mingw32 $(prefix)
	$(RMDIR) scratch

$(file_list): initial_setup

.PHONY: initial_setup
initial_setup:
	type "$(WGET)" >/dev/null || { $(ECHO) -e $(wget_missing)   && false; }
	[ ! -e $(prefix) ]        || { $(ECHO) -e $(prefix_exists)  && false; }
	[ ! -e scratch   ]        || { $(ECHO) -e $(scratch_exists) && false; }
	$(MKDIR) --parents $(prefix)
	$(RM) --force --recursive $(prefix)
	$(MKDIR) --parents scratch

BSDTARFLAGS := --keep-old-files

WGETFLAGS :=

.PHONY: %.7z
%.7z:
	cd $(cache_dir) && [ -e $@ ] || $(WGET) $(WGETFLAGS) $(mirror)/$@
	cd $(cache_dir) && $(ECHO) "$($@-md5) *$@" | $(MD5SUM) --check
	-$(BSDTAR) --extract $(BSDTARFLAGS) --directory=scratch --file=$(cache_dir)/$@

