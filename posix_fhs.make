# Platform specifics: GNU/Linux and systems that equivalently comply
# with POSIX and the Filesystem Hierarchy Standard.
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

SHREXT := .so

gcc_proclitic :=

EXTRA_LIBS := -ldw -lunwind -ldl

# Programs for which FHS doesn't specify a location.

# The 'xmllint' lmi builds matches the libxml2 version lmi uses:
#   XMLLINT := $(localbindir)/xmllint
# while the build system's own 'xmllint', if installed, may differ:
    XMLLINT := xmllint
