# Configuration makefile.
#
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

################################################################################

# Include platform-specific makefile.

lmi_build_type := $(shell /usr/share/misc/config.guess)

ifeq (msys,$(findstring msys,$(lmi_build_type)))
  platform_makefile := msw_msys.make
else ifeq (cygwin,$(findstring cygwin,$(lmi_build_type)))
  platform_makefile := msw_cygwin.make
else
# 'config.guess' might indicate something like "x86_64-pc-wsl"
# someday, but until then:
  kernel_release := $(shell uname -r)
  ifeq (Microsoft,$(findstring Microsoft,$(kernel_release)))
    platform_makefile := msw_wsl.make
  else ifeq (mingw32,$(findstring mingw32,$(LMI_TRIPLET)))
    platform_makefile := msw_generic.make
  else
    platform_makefile := posix_fhs.make
  endif
endif

include $(srcdir)/$(platform_makefile)
$(srcdir)/$(platform_makefile):: ;

################################################################################

# Standard utilities.

# Required in /bin (if anywhere) by FHS-2.2 .

CHMOD   := chmod
CP      := cp
DATE    := date
ECHO    := echo
GZIP    := gzip
INSTALL := install
LS      := ls
MKDIR   := mkdir
MV      := mv
RM      := rm
SED     := sed
TAR     := tar

# FHS-2.2 would presumably put these in /usr/bin . However, debian
# puts 'bzip2' and 'grep' in /bin .

BSDTAR  := bsdtar
BZIP2   := bzip2
DIFF    := diff
GREP    := grep
MD5SUM  := md5sum
PATCH   := patch
SORT    := sort
TOUCH   := touch
TR      := tr
WC      := wc
WGET    := wget
