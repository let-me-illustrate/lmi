# Compiler-specific makefile: ascertain gcc version.
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

# Don't remake this makefile.

$(srcdir)/compiler_gcc_version.make:: ;

# Aliases for tools used in targets elsewhere.

AR      := $(gcc_proclitic)ar
CC      := $(gcc_proclitic)gcc
CPP     := $(gcc_proclitic)cpp
CXX     := $(gcc_proclitic)g++
LD      := $(gcc_proclitic)g++
# For GNU/Linux, $(RC) is never invoked.
RC      := $(gcc_proclitic)windres

# GNU tools (or workalikes) for special purposes.
#
# For testing physical closure and generating autodependencies, use
# either GNU tools or closely compatible equivalents such as clang.
# This obviates figuring out how other toolchains support these needs.
#
# Override these definitions to specify GNU tools when using an
# incompatible toolchain.

GNU_CPP := $(CPP)
GNU_CXX := $(CXX)

# Compiler version.

# $(subst): workaround for debian, whose MinGW-w64 identifies its
# version 7.x.0 as "7.x-win32".

ifeq (gcc,$(LMI_COMPILER))
  gcc_version   := $(subst -win32,.0,$(shell $(CXX)     -dumpversion))
endif

# These are defined even for toolchains other than gcc.

gnu_cpp_version := $(subst -win32,.0,$(shell $(GNU_CPP) -dumpversion))
gnu_cxx_version := $(subst -win32,.0,$(shell $(GNU_CXX) -dumpversion))

ifeq      (10,$(gnu_cpp_version))
else ifeq (10.0,$(gnu_cpp_version))
else ifeq (11,$(gnu_cpp_version))
else ifeq (11.0,$(gnu_cpp_version))
else ifeq (12,$(gnu_cpp_version))
else ifeq (12.0,$(gnu_cpp_version))
else
  $(warning Untested $(GNU_CPP) version '$(gnu_cpp_version)')
endif

ifeq      (10,$(gnu_cxx_version))
else ifeq (10.0,$(gnu_cxx_version))
else ifeq (11,$(gnu_cxx_version))
else ifeq (11.0,$(gnu_cxx_version))
else ifeq (12,$(gnu_cxx_version))
else ifeq (12.0,$(gnu_cxx_version))
else
  $(warning Untested $(GNU_CXX) version '$(gnu_cxx_version)')
endif
