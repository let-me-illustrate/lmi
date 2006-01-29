# Configuration makefile.
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

# $Id: configuration.make,v 1.3 2006-01-29 13:52:00 chicares Exp $

################################################################################

# Include platform-specific makefile.

uname := $(shell uname -s 2>/dev/null)

platform-makefile := posix_fhs.make

ifeq (MINGW,$(findstring MINGW,$(uname)))
  platform-makefile := msw_msys.make
else
  ifeq (CYGWIN,$(findstring CYGWIN,$(uname)))
    platform-makefile := msw_cygwin.make
  else
    ifeq (,$(uname))
      uname := msw_generic
      platform-makefile := msw_generic.make
    endif
  endif
endif

include $(src_dir)/$(platform-makefile)
$(src_dir)/$(platform-makefile):: ;

################################################################################

# Path to preprocessor used to generate dependencies.

# If the GNU preprocessor and C compiler aren't what you get when you type
#   cpp --version
#   gcc --version
# then give their full path here.

GENERIC_GNU_PREPROCESSOR := $(CPP)
GENERIC_GCC              := $(CXX)

