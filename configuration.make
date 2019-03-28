# Configuration makefile.
#
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019 Gregory W. Chicares.
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

################################################################################

# Include platform-specific makefile.

uname := $(shell uname 2>/dev/null)

platform-makefile := posix_fhs.make

ifeq (i686-w64-mingw32,$(findstring i686-w64-mingw32,$(LMI_HOST)))
  platform-makefile := msw_generic.make
else ifeq (x86_64-w64-mingw32,$(findstring x86_64-w64-mingw32,$(LMI_HOST)))
  platform-makefile := msw_generic.make
endif

ifeq (MINGW,$(findstring MINGW,$(uname)))
  platform-makefile := msw_msys.make
else ifeq (CYGWIN,$(findstring CYGWIN,$(uname)))
  platform-makefile := msw_cygwin.make
else ifeq (,$(uname))
  uname := msw_generic
  platform-makefile := msw_generic.make
endif

include $(srcdir)/$(platform-makefile)
$(srcdir)/$(platform-makefile):: ;

################################################################################

# GNU tools for special purposes.

# Always use the GNU C++ compiler and preprocessor, version 3.x or
# later, for testing physical closure and generating autodependencies.
# This obviates figuring out how other toolchains support these needs.
#
# Override these definitions to specify GNU tools when using a
# toolchain other than gcc-3.x or later.

GNU_CPP := $(CPP)
GNU_CXX := $(CXX)
