# Installer for MinGW.
#
# Copyright (C) 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

# Set default version.

version   := MinGW-20090203

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
#
# If $(system_root) is empty, then '$(system_root)/foo' means simply
# '/foo', which is reasonable enough for msw; for a posix-emulation
# system like Cygwin, of course, that's not in the msw root directory.

prefix    := $(system_root)/$(version)

cache_dir := .

# In the past, it seemed necessary to specify a mirror, e.g.:
#  mirror := http://easynews.dl.sourceforge.net/sourceforge/mingw
# but as of about 2006-12 sf.net seems to select one automatically
# when this is passed to wget:
mirror    := http://downloads.sourceforge.net/mingw

# File lists ###################################################################

MinGW-20090203 = \
  binutils-2.19.1-mingw32-bin.tar.gz \
  gcc-core-3.4.5-20060117-3.tar.gz \
  gcc-g++-3.4.5-20060117-3.tar.gz \
  mingwrt-3.15.2-mingw32-dev.tar.gz \
  w32api-3.13-mingw32-dev.tar.gz \

# $(MinGW-20080502): For the nonce, the (humongous) java tarball is
# required due to a g++ packaging defect--see:
#   http://article.gmane.org/gmane.comp.gnu.mingw.user/26485
#     [2008-05-09T02:53:49Z from Brian Dessent]

MinGW-20080502 = \
  binutils-2.17.50-20060824-1.tar.gz \
  gcc-part-core-4.3.0-20080502-2-mingw32-alpha-bin.tar.gz \
  gcc-part-c++-4.3.0-20080502-2-mingw32-alpha-bin.tar.gz \
  gcc-part-java-4.3.0-20080502-2-mingw32-alpha-bin.tar.gz \
  mingw-runtime-3.14.tar.gz \
  w32api-3.11.tar.gz \

# $(MinGW-20061119): 'Candidate' versions from
#   Chris Sutcliffe's 2006-11-19T02:27Z email to MinGW-dvlpr,
# but see
#   Keith MARSHALL's 2006-12-02T20:30Z email to Mingw-users.

MinGW-20061119 = \
  binutils-2.16.91-20060119-1.tar.gz \
  gcc-core-3.4.5-20060117-1.tar.gz \
  gcc-g++-3.4.5-20060117-1.tar.gz \
  mingw-runtime-3.11.tar.gz \
  w32api-3.8.tar.gz \

MinGW-20060119 = \
  binutils-2.16.91-20060119-1.tar.gz \
  gcc-core-3.4.5-20060117-1.tar.gz \
  gcc-g++-3.4.5-20060117-1.tar.gz \
  mingw-runtime-3.9.tar.gz \
  w32api-3.6.tar.gz \

# This, I think, is equivalent to the C++ portion of 'MinGW-5.0.0.exe'.

MinGW-20050827 = \
  binutils-2.16.91-20050827-1.tar.gz \
  gcc-core-3.4.4-20050522-1.tar.gz \
  gcc-g++-3.4.4-20050522-1.tar.gz \
  mingw-runtime-3.8.tar.gz \
  w32api-3.3.tar.gz \

# This almost perfectly reproduces 'mingw-1.0.1-20010726.tar.gz',
# which is no longer available at sf.net for reasons unknown. Only
# one file differs: 'include/excpt.h', which in the vanished tarball
# tries harder to implement "structured exception handling". That
# tarball contained fourteen files in an 'info-html' subdirectory,
# and a 'make' binary; those fifteen superfluous files aren't created
# by this target. Aside from those differences, this target exactly
# reproduces the vanished tarball's contents.
#
# One file, 'include/float.h', contains a nasty defect: its values
# for DBL_EPSILON and LDBL_EPSILON are wrong. The correct values are:
#   DBL_EPSILON  2.2204460492503131e-16
#   LDBL_EPSILON 1.08420217248550443401e-19L

MinGW-20010726 = \
  binutils-2.11.90-20010705.tar.gz \
  gcc-2.95.3-20010723.tar.gz \
  mingw-runtime-1.0.1-20010726.tar.gz \
  w32api-1.0.1-20010726.tar.gz \

# Archive md5sums ##############################################################

binutils-2.11.90-20010705.tar.gz-md5    := 0173ff92655b77fafe63fca4ef68e005
binutils-2.16.91-20050827-1.tar.gz-md5  := 9d2de9e3cd5fede3d12150b8a7d0bbf7
binutils-2.16.91-20060119-1.tar.gz-md5  := a54f33ca9d6cf56dc99c0c5367f58ea3
binutils-2.17.50-20060824-1.tar.gz-md5  := 401468f2873a75923d3753144e171401
binutils-2.19.1-mingw32-bin.tar.gz-md5  := 6bba3bd1bf510d152a42d0beeeefa14d
gcc-2.95.3-20010723.tar.gz-md5          := caf08d8b16e79265c110c01a57c892f7
gcc-core-3.4.4-20050522-1.tar.gz-md5    := 46f17998ab61be9dbede233b44c7b0e6
gcc-core-3.4.5-20060117-1.tar.gz-md5    := 1a4afae471ea93b975e3f8b3ac529eac
gcc-core-3.4.5-20060117-3.tar.gz-md5    := 33c9fdf7468a3c410f830471d0e3ffb8
gcc-g++-3.4.4-20050522-1.tar.gz-md5     := db44ac5b06d7f262c59422ae21511659
gcc-g++-3.4.5-20060117-1.tar.gz-md5     := d11a9d63a0f862650f755fdb4e947dc4
gcc-g++-3.4.5-20060117-3.tar.gz-md5     := b3e7875d160fd8df2b13c5448ca78c4e
gcc-part-core-4.3.0-20080502-2-mingw32-alpha-bin.tar.gz-md5 := 1f157b66d097da0d07940dd6cec54a91
gcc-part-c++-4.3.0-20080502-2-mingw32-alpha-bin.tar.gz-md5  := 869751a0ace9bf47b8a0627fdd91e0a9
gcc-part-java-4.3.0-20080502-2-mingw32-alpha-bin.tar.gz-md5 := 7d8d0a13e66887b13dc0e463ad946f89
mingw-runtime-1.0.1-20010726.tar.gz-md5 := 0641d545a9a23fbf3c028631c2303217
mingw-runtime-3.8.tar.gz-md5            := 5852e9b2c369aff1d4ba47d3dd20728f
mingw-runtime-3.9.tar.gz-md5            := 0cb66b1071da224ea2174f960c593e2e
mingw-runtime-3.11.tar.gz-md5           := e1c21f8c4ece49d8bd9fef9e1b0e44a7
mingw-runtime-3.14.tar.gz-md5           := 44a42fc00ccaa50f1dd17f465078cc61
mingwrt-3.15-mingw32-dev.tar.gz-md5     := 8a34bb94197d93647375ef70438bcb37
mingwrt-3.15.2-mingw32-dev.tar.gz-md5   := f24d63744af66b54547223bd5476b8f0
w32api-1.0.1-20010726.tar.gz-md5        := d8d6d91d41f638e742be9d21ca046c42
w32api-3.3.tar.gz-md5                   := 2da21c26013711ae90d3b2416c20856e
w32api-3.6.tar.gz-md5                   := 2f86ec42cafd774ec82162fbc6e6808d
w32api-3.8.tar.gz-md5                   := b53fdf670f33d2e901749f4792e659f2
w32api-3.11.tar.gz-md5                  := 3cb523f58ea5d12f81397cf85f3b7011
w32api-3.12-mingw32-dev.tar.gz-md5      := 2542596278bc805d92ee27c56ce9e16f
w32api-3.13-mingw32-dev.tar.gz-md5      := a50fff6bc1e1542451722e2650cb53b4

# Utilities ####################################################################

ECHO   := echo
GREP   := grep
MD5SUM := md5sum
MKDIR  := mkdir
MV     := mv
RM     := rm
TAR    := tar
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

# Some files are duplicated in MinGW archives, so the order of
# extraction is important. It follows these instructions:
#   http://groups.yahoo.com/group/mingw32/message/1145
# It is apparently fortuitous that the order is alphabetical.
#
# Some gcc archives distributed by MinGW contain a version of
# 'libiberty.a' that's incompatible with the version provided with
# binutils, so gcc's 'libiberty.a' is explicitly excluded: it seems
# reasonable to expect binutils to provide a 'libiberty.a' that works
# with the 'libbfd.a' it also provides. See:
#   http://article.gmane.org/gmane.comp.gnu.mingw.user/6932/
#     [2003-04-29T21:32:35Z from Danny Smith]
# to learn why this may matter a great deal.
#
# These archives
#   binutils-2.16.91-20050827-1.tar.gz
#   gcc-core-3.4.4-20050522-1.tar.gz
# contain 'info/dir' files that differ. They aren't necessary because
# documentation is available online, and it is not our goal to correct
# MinGW packaging anomalies, so that directory is simply excluded.
#
# Other conflicts are managed by specifying '--keep-old-files' and
# permitting 'tar' to fail with an error message without stopping this
# makefile.

TARFLAGS := --keep-old-files
%.tar.bz2: TARFLAGS += --bzip2
%.tar.gz:  TARFLAGS += --gzip
gcc%:      TARFLAGS += --exclude 'libiberty.a' --exclude 'info/dir'

# New spelling '--no-verbose' has replaced original '--non-verbose':
#   http://sourceware.org/ml/cygwin-apps/2005-10/msg00140.html
# However, don't use
#   WGETFLAGS := '--no-verbose --timestamping'
# because, as this is written in 2007-08, sourceforge's mirror system
# is behaving in anomalous ways that '--no-verbose' would mask.

WGETFLAGS := '--timestamping'

.PHONY: %.tar.bz2 %.tar.gz
%.tar.bz2 %.tar.gz:
	cd $(cache_dir) && [ -e $@ ] || $(WGET) $(WGETFLAGS) $(mirror)/$@
	cd $(cache_dir) && $(ECHO) "$($@-md5) *$@" | $(MD5SUM) --check
	-$(TAR) --extract $(TARFLAGS) --directory=scratch --file=$(cache_dir)/$@

# Test #########################################################################

test_prefix = /eraseme

test_file_list = \
  gcc-response-file-2.tar.gz \
  zlib-1.2.3-mingwPORT.tar.bz2 \

# Archive md5sums for $(test_file_list).
gcc-response-file-2.tar.gz-md5         := aa9825791000af0e4d4d0345bd4325ce
zlib-1.2.3-mingwPORT.tar.bz2-md5       := e131ea48214af34bd6adee6b7bdadfd5

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

