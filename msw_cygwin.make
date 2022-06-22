# Platform specifics: msw (cygwin) with MinGW-w64 toolchain.
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

# Sanity checks.

cygdrive_diagnostic := \
  Current working directory '$(CURDIR)' contains '/cygdrive/'. \
  This is likely to cause problems with native tools such as the \
  MinGW-w64 compiler. Build from an identity-mount directory instead

ifeq (/cygdrive/,$(findstring /cygdrive/,$(CURDIR)))
  $(error $(cygdrive_diagnostic))
endif

ifeq (,$(wildcard /opt/lmi/*))
  $(warning Installation may be invalid: /opt/lmi/ missing or empty. )
endif

################################################################################

# Use cygwin as a quasi-cross-compiler for --host=*-w64-mingw32.

# For autotoolized libraries, pass flags such as these to 'configure':
# cross_compile_flags := --build=x86_64-pc-cygwin --host=$(LMI_TRIPLET)

################################################################################

# Compiler, linker, and so on.

# Full path to gcc binaries, slash-terminated if nonempty. Setting it
# to an empty string finds gcc on $PATH instead.

mingw_dir   := /opt/lmi/${LMI_COMPILER}_${LMI_TRIPLET}/gcc_msw
gcc_bin_dir := $(mingw_dir)/bin/

# Oddly, MinGW-w64 provides prefixed versions of compilers, e.g.:
#   i686-w64-mingw32-gcc.exe
#   x86_64-w64-mingw32-g++.exe
# but not of the other tools. Yet its distributed archives each
# contain a distinct subdirectory indicating word size, e.g.:
#   mingw32/bin/g++.exe in 'i686-*' archives
#   mingw64/bin/g++.exe in 'x86_64-*' archives
# which can potentially coexist.

#host_hyphen := $(LMI_TRIPLET)-
host_hyphen :=

AR      := $(gcc_bin_dir)$(host_hyphen)ar
CC      := $(gcc_bin_dir)$(host_hyphen)gcc
CPP     := $(gcc_bin_dir)$(host_hyphen)cpp
CXX     := $(gcc_bin_dir)$(host_hyphen)g++
LD      := $(gcc_bin_dir)$(host_hyphen)g++
RC      := $(gcc_bin_dir)$(host_hyphen)windres

# Programs for which FHS doesn't specify a location.

# The 'xmllint' lmi builds matches the libxml2 version lmi uses:
    XMLLINT := $(localbindir)/xmllint
# while the build system's own 'xmllint', if installed, may differ:
#   XMLLINT := xmllint

# Identify run-time libraries for redistribution. See:
#   https://cygwin.com/ml/cygwin/2010-09/msg00553.html
# Of course manipulating an lmi user's $PATH is out of the question.

compiler_sysroot := $(mingw_dir)/$(LMI_TRIPLET)/lib

compiler_runtime_files := \
  $(wildcard $(compiler_sysroot)/libgcc*.dll) \
  $(wildcard $(compiler_sysroot)/libstdc++*.dll) \

################################################################################

# Configuration shared by all msw subplatforms.

include $(srcdir)/msw_common.make
$(srcdir)/msw_common.make:: ;
