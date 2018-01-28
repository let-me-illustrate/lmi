# Platform specifics: *nix cross for msw with MinGW-w64 toolchain.
#
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 Gregory W. Chicares.
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

# Sanity checks.

ifeq (,$(wildcard /opt/lmi/*))
  $(warning Installation may be invalid: /opt/lmi/ missing or empty. )
endif

################################################################################

system_root := /

PERFORM := wine

################################################################################

# Compiler, linker, and so on.

# Full path to gcc binaries, slash-terminated if nonempty. Setting it
# to an empty string finds gcc on $PATH instead.

gcc_bin_dir :=

host_prefix := i686-w64-mingw32-

AR      := $(gcc_bin_dir)$(host_prefix)ar
CC      := $(gcc_bin_dir)$(host_prefix)gcc
CPP     := $(gcc_bin_dir)$(host_prefix)cpp
CXX     := $(gcc_bin_dir)$(host_prefix)g++
LD      := $(gcc_bin_dir)$(host_prefix)g++
RC      := $(gcc_bin_dir)$(host_prefix)windres

# Identify run-time libraries for redistribution.

# It might seem more robust to write something like
#   compiler_sysroot := $(shell readlink -fn /usr/lib/gcc/i686-w64-mingw32/*-win32)
# but that would actually weaken makefile portability, and there
# is no guarantee that this directory will be named similarly in
# future debian releases, much less on other OSs.
compiler_sysroot := /usr/lib/gcc/i686-w64-mingw32/6.3-win32

# 'libwinpthread*' is no longer needed for debian cross builds--see:
#   https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=748353
# | Provide compilers using Windows and POSIX threads. The default setup
# | uses Windows threads, thus avoiding the dependency on the pthreads DLL
# but including in this list with $(wildcard) does no harm.
compiler_runtime_files := \
  $(wildcard $(compiler_sysroot)/libgcc*.dll) \
  $(wildcard $(compiler_sysroot)/libstdc++*.dll) \
  $(wildcard $(compiler_sysroot)/libwinpthread-1.dll) \

################################################################################

# Standard utilities.

# Required in /bin (if anywhere) by FHS-2.2 .

CP      := cp
DATE    := date
ECHO    := echo
GZIP    := gzip
LS      := ls
MKDIR   := mkdir
MV      := mv
RM      := rm
SED     := sed
TAR     := tar

# FHS-2.2 would presumably put these in /usr/bin . However, debian
# puts 'bzip2' and 'grep' in /bin .

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

# Programs for which FHS doesn't specify a location.

# Instead of requiring installation of the build system's own libxml2:
#   XMLLINT := xmllint
# use the one that lmi builds:
XMLLINT := /opt/lmi/local/bin/xmllint

################################################################################

# Configuration shared by all msw subplatforms.

include $(srcdir)/msw_common.make
$(srcdir)/msw_common.make:: ;

