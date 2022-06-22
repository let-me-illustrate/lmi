# Platform specifics: *nix cross for msw with MinGW-w64 toolchain.
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

ifeq (,$(wildcard /opt/lmi/*))
  $(warning Installation may be invalid: /opt/lmi/ missing or empty. )
endif

################################################################################

# Compiler, linker, and so on.

# Full path to gcc binaries, slash-terminated if nonempty. Setting it
# to an empty string finds gcc on $PATH instead.

gcc_bin_dir :=

host_hyphen := $(LMI_TRIPLET)-

AR      := $(gcc_bin_dir)$(host_hyphen)ar
CC      := $(gcc_bin_dir)$(host_hyphen)gcc
CPP     := $(gcc_bin_dir)$(host_hyphen)cpp
CXX     := $(gcc_bin_dir)$(host_hyphen)g++
LD      := $(gcc_bin_dir)$(host_hyphen)g++
RC      := $(gcc_bin_dir)$(host_hyphen)windres

# Programs for which FHS doesn't specify a location.

# The 'xmllint' lmi builds matches the libxml2 version lmi uses:
#   XMLLINT := $(PERFORM) $(localbindir)/xmllint$(EXEEXT)
# while the build system's own 'xmllint', if installed, may differ:
    XMLLINT := xmllint
# Don't use the one lmi builds until 'wine' is fixed--see:
#   https://lists.nongnu.org/archive/html/lmi/2019-01/msg00034.html

# Identify run-time libraries for redistribution. See:
#   https://lists.nongnu.org/archive/html/lmi/2017-05/msg00046.html
# Perhaps gcc's '-print-sysroot' would be more suitable, but that
# option returns an empty string with debian cross compilers.
#
# It might seem more robust to write something like
#   compiler_sysroot := $(shell readlink -fn /usr/lib/gcc/$(LMI_TRIPLET)/*-win32)
# but that would actually weaken makefile portability, and there
# is no guarantee that this directory will be named similarly in
# future debian releases, much less on other OSs.

compiler_sysroot := $(dir $(shell $(CXX) -print-libgcc-file-name))

compiler_runtime_files := \
  $(wildcard $(compiler_sysroot)/libgcc*.dll) \
  $(wildcard $(compiler_sysroot)/libstdc++*.dll) \

################################################################################

# Configuration shared by all msw subplatforms.

include $(srcdir)/msw_common.make
$(srcdir)/msw_common.make:: ;
