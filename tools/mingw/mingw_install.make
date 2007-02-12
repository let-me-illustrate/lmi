# Installer for MinGW.
#
# Copyright (C) 2006, 2007 Gregory W. Chicares.
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

# $Id: mingw_install.make,v 1.2 2007-02-12 17:05:22 wboutin Exp $

# Configurable settings #######################################################

file_list = $(mingw_20060119)

mirror = http://easynews.dl.sourceforge.net/sourceforge/mingw

prefix = /c/mingw

# File lists ##################################################################

mingw_20060119 = \
  binutils-2.16.91-20060119-1.tar.gz \
  gcc-core-3.4.5-20060117-1.tar.gz \
  gcc-g++-3.4.5-20060117-1.tar.gz \
  mingw-runtime-3.9.tar.gz \
  w32api-3.6.tar.gz \

# This, I think, is equivalent to the C++ portion of 'MinGW-5.0.0.exe'.

mingw_20050827 = \
  binutils-2.16.91-20050827-1.tar.gz \
  gcc-core-3.4.4-20050522-1.tar.gz \
  gcc-g++-3.4.4-20050522-1.tar.gz \
  mingw-runtime-3.8.tar.gz \
  w32api-3.3.tar.gz \

# TODO ?? Add file lists for earlier releases.

# Utilities ###################################################################

ECHO   = echo
GREP   = grep
MD5SUM = md5sum
MKDIR  = mkdir
MV     = mv
RM     = rm
TAR    = tar
WGET   = wget

# Error messages ##############################################################

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

# Targets #####################################################################

.PHONY: all
all: $(file_list)
	@$(MV) scratch $(prefix)

$(file_list): initial_setup

.PHONY: initial_setup
initial_setup:
	@type "$(WGET)" >/dev/null || { $(ECHO) -e $(wget_missing)   && false; }
	@[ ! -e $(prefix) ]        || { $(ECHO) -e $(prefix_exists)  && false; }
	@[ ! -e scratch   ]        || { $(ECHO) -e $(scratch_exists) && false; }
	@$(MKDIR) --parents $(prefix)
	@$(RM) --force --recursive $(prefix)
	@$(MKDIR) --parents scratch

%.tar.bz2: decompress = --bzip2
%.tar.gz:  decompress = --gzip

.PHONY: %.tar.bz2 %.tar.gz
%.tar.bz2 %.tar.gz:
	@[ -e $@ ] || $(WGET) --non-verbose --timestamping $(mirror)/$@
	@$(TAR) --extract $(decompress) --directory=scratch --file=$@

# Test ########################################################################

this_makefile = mingw_install.make

test_prefix = /eraseme

test_file_list = \
  gcc-response-file-2.tar.gz \
  zlib-1.2.3-mingwPORT.tar.bz2 \

test_overrides = \
  --no-print-directory \
     --file=$(this_makefile) \
     prefix='$(test_prefix)' \
  file_list='$(test_file_list)' \

.PHONY: test
test:
	@$(ECHO) "Expect some ignored errors, then a success message"
	@$(RM) --force $(test_file_list)
	@$(RM) --force --recursive $(test_prefix)
	@$(RM) --force --recursive scratch
	@$(MKDIR) scratch
	@$(ECHO) "  test 0: expect an error in 'initial_setup' "
	-@$(MAKE) $(test_overrides) >test_0
	@$(GREP) --silent \
	  "Error: Scratch directory 'scratch' already exists." test_0
	@$(RM) test_0
	@$(RM) --force --recursive scratch
	@$(MKDIR) $(test_prefix)
	@$(ECHO) "  test 1: expect another error in 'initial_setup' "
	-@$(MAKE) $(test_overrides) >test_1
	@$(GREP) --silent \
	  "Error: Prefix directory '$(test_prefix)' already exists." test_1
	@$(RM) test_1
	@$(RM) --force --recursive $(test_prefix)
	@$(ECHO) "  test 2: expect to get two files and install all files"
	@$(MAKE) $(test_overrides)
	@$(MD5SUM) --check --status test.md5
	@$(RM) --force $(firstword $(test_file_list))
	@$(RM) --force --recursive $(test_prefix)
	@$(ECHO) "  test 3: expect to get one file and install all files"
	@$(MAKE) $(test_overrides)
	@$(MD5SUM) --check --status test.md5
	@$(RM) --force --recursive $(test_prefix)
	@$(ECHO) "  test 4: expect to get no file and install all files"
	@$(MAKE) $(test_overrides)
	@$(MD5SUM) --check --status test.md5
	@$(RM) --force --recursive $(test_prefix)
	@$(ECHO) "All tests succeeded"

