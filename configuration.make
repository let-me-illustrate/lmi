# Configuration makefile.
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

# $Id: configuration.make,v 1.1 2005-01-28 01:34:43 chicares Exp $

################################################################################

# Useful paths relative to $(src_dir). TODO ?? Not used yet--expunge?

# It might seem more natural to specify the parent of $(src_dir) as
#   $(dir $(src_dir))
# but that would retain the trailing slash, which can cause problems
# on the msw platform, even with a decent shell.

lmi_root        := $(src_dir:%/$(notdir $(src_dir))=%)

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

