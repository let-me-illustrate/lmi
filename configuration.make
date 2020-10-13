# Configuration makefile.
#
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

# Generic settings.

# Flags for all other xml libraries are provided by *-config scripts,
# but '-lexslt' is a special case--see:
#   https://mail.gnome.org/archives/xslt/2001-October/msg00133.html
#   https://lists.nongnu.org/archive/html/lmi/2020-10/msg00066.html

platform_xml_libraries := \
  $(shell xmlwrapp-config --libs) \
  -lexslt \
  $(shell xslt-config --libs) \
  $(shell xml2-config --libs) \

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
