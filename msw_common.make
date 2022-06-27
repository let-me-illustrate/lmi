# Platform specifics: msw, shared by all subplatforms.
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

$(srcdir)/msw_common.make:: ;

platform_gui_ldflags := -mwindows

SHREXT := .dll

# Libraries and associated options.

# There is no universal standard way to install free software on this
# platform, so copy libraries and their headers to /usr/local as FHS
# prescribes.

# Prefer to use a shared-library version of libxml2: it links faster
# and rarely needs to be updated.

platform_defines := \
  -DXMLWRAPP_USE_DLL \
  -DXSLTWRAPP_USE_DLL \
  -DSTRICT \

# Identify msw run-time libraries for redistribution. See:
#   https://lists.nongnu.org/archive/html/lmi/2017-05/msg00046.html
# Perhaps gcc's '-print-sysroot' would be more suitable, but that
# option returns an empty string with debian cross compilers.
#
# It might seem more robust to write something like
#   compiler_sysroot := $(shell readlink -fn /usr/lib/gcc/$(LMI_TRIPLET)/*-win32)
# but that would actually weaken makefile portability, and there
# is no guarantee that this directory will be named similarly in
# future debian releases, much less on other OSs.
#
# Invoke the C++ compiler in a direct fashion because $(CXX) has not
# yet been defined. 'GNUmakefile' needs to install the runtime files
# to make sure they're always available, but it has no reason to know
# how $(CXX) is defined. Making any target that uses $(CXX), e.g.:
#   make unit_tests unit_test_targets=sandbox_test
# thus ensures that the runtime files are available on $WINEPATH.

ifeq (mingw32,$(findstring mingw32,$(LMI_TRIPLET)))
compiler_sysroot := $(dir $(shell $(gcc_proclitic)g++ -print-libgcc-file-name))

compiler_runtime_files := \
  $(wildcard $(compiler_sysroot)/libgcc*.dll) \
  $(wildcard $(compiler_sysroot)/libstdc++*.dll) \

endif
