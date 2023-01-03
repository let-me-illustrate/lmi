# Platform specifics: msw (WSL) with MinGW-w64 toolchain.
#
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

$(srcdir)/msw_wsl.make:: ;

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

gcc_proclitic := $(gcc_bin_dir)$(host_hyphen)

# Programs for which FHS doesn't specify a location.

# The 'xmllint' lmi builds matches the libxml2 version lmi uses:
    XMLLINT := $(localbindir)/xmllint$(EXEEXT)
# while the build system's own 'xmllint', if installed, may differ:
#   XMLLINT := xmllint

################################################################################

# Configuration shared by all msw subplatforms.

include $(srcdir)/msw_common.make
$(srcdir)/msw_common.make:: ;
