# Legacy installer for MinGW-w64 32-bit msw-native toolchain.
#
# Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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
# https://savannah.nongnu.org/projects/lmi
# email: <gchicares@sbcglobal.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

this_makefile := $(abspath $(lastword $(MAKEFILE_LIST)))

# Configurable settings ########################################################

# Default version. There's no terse and unambiguous way to specify
# versions, as the MinGW-w64 project hosts other projects' builds
# rather than release its own; lmi uses i686 builds with native
# threads and SJLJ exceptions.

version   := MinGW-8_1_0

file_list   = $($(version))

# Prefer to set $(prefix) to anything but '/mingw', in order to avoid
# the problem described here:
#   http://gcc.gnu.org/ml/gcc-patches/2004-06/msg00703.html
# when multiple versions of MinGW gcc are installed, as discussed on
# the mingw-users mailing list in these messages:
#   https://sourceforge.net/p/mingw/mailman/message/15864073/
#     [2005-01-17T16:30:44Z from Greg Chicares]
#   https://sourceforge.net/p/mingw/mailman/message/15864075/
#     [2005-01-17T18:15:26Z from Aaron W. LaFramboise]

mingw_dir  := /opt/lmi/${LMI_COMPILER}_${LMI_TRIPLET}/gcc_msw
prefix     := $(mingw_dir)

cache_dir  := /srv/cache_for_lmi/downloads

ad_hoc_dir := /opt/lmi/zzz/mingw-ad_hoc/ad_hoc

# In the past, it seemed necessary to specify a mirror, e.g.:
#  mirror := http://easynews.dl.sourceforge.net/sourceforge/mingw
# but as of about 2006-12 sf.net seems to select one automatically
# when this is passed to wget:
mirror     := http://downloads.sourceforge.net/mingw-w64

# File lists ###################################################################

#MinGW-6_3_0 := i686-6.3.0-release-win32-sjlj-rt_v5-rev1.7z
MinGW-6_3_0 := i686-6.3.0-release-win32-sjlj-rt_v5-rev2.7z
MinGW-7_2_0 := i686-7.2.0-release-win32-sjlj-rt_v5-rev0.7z
MinGW-7_3_0 := i686-7.3.0-release-win32-sjlj-rt_v5-rev0.7z
MinGW-8_1_0 := i686-8.1.0-release-win32-sjlj-rt_v6-rev0.7z

# Archive md5sums ##############################################################

#$(MinGW-6_3_0)-md5 := b92e8480cf8d5904da78ab6d94f1a047
$(MinGW-6_3_0)-md5 := 6e15de993400279c24b40b1f978e9380
$(MinGW-7_2_0)-md5 := f34ff6eca4aa7a645f60c977b107c5d2
$(MinGW-7_3_0)-md5 := 37d964d08ce48dc170cc95a84679cc4f
$(MinGW-8_1_0)-md5 := 28ec1e65ab85a9e1043998516045ab62

# Utilities ####################################################################

BSDTAR := bsdtar
CHMOD  := chmod
CP     := cp
ECHO   := echo
MD5SUM := md5sum
MKDIR  := mkdir
MV     := mv
RM     := rm
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

ad_hoc_dir_exists = \
  "\nError: Nonce directory '$(ad_hoc_dir)' already exists." \
  "\nProbably it is left over from a previous failure." \
  "\nJust remove it unless you're sure you want whatever files" \
  "\nit might contain." \
  "\n"

# Targets ######################################################################

.PHONY: all
all: $(file_list)
	$(CP) --archive $(ad_hoc_dir)/mingw32/* $(prefix)
	$(RM) --force --recursive $(ad_hoc_dir)

$(file_list): initial_setup

.PHONY: initial_setup
initial_setup:
	[   -n "$$LMI_COMPILER" ] || { printf '%s\n' "no LMI_COMPILER"  && false; }
	[   -n "$$LMI_TRIPLET"  ] || { printf '%s\n' "no LMI_TRIPLET"   && false; }
	type "$(WGET)" >/dev/null || { printf '%b' $(wget_missing)      && false; }
	[ ! -e $(prefix)        ] || { printf '%b' $(prefix_exists)     && false; }
	[ ! -e $(ad_hoc_dir)    ] || { printf '%b' $(ad_hoc_dir_exists) && false; }
	$(MKDIR) --parents $(prefix)
	$(MKDIR) --parents $(ad_hoc_dir)

BSDTARFLAGS := --keep-old-files

WGETFLAGS := --no-verbose

.PHONY: %.7z
%.7z:
	cd $(cache_dir) && [ -e $@ ] || $(WGET) $(WGETFLAGS) $(mirror)/$@
	cd $(cache_dir) && $(ECHO) "$($@-md5) *$@" | $(MD5SUM) --check
	$(BSDTAR) --extract $(BSDTARFLAGS) --directory=$(ad_hoc_dir) --file=$(cache_dir)/$@
	$(CHMOD) -R g=u $(ad_hoc_dir)
