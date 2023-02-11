#!/bin/sh

# Recast definitions from 'set_toolchain.sh' for use in makefiles.

# Copyright (C) 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

# See 'GNUmakefile' for suggested use.

# Directory where this script resides.

srcdir=$(dirname "$(readlink --canonicalize "$0")")

# shellcheck disable=SC1090
. "$srcdir"/set_toolchain.sh

printf '%s\n' "export LMI_COMPILER    := $LMI_COMPILER"
printf '%s\n' "export LMI_TRIPLET     := $LMI_TRIPLET"
printf '%s\n' "export PATH            := $PATH"
printf '%s\n' "export LD_LIBRARY_PATH := $LD_LIBRARY_PATH"
printf '%s\n' "export WINEPATH        := $WINEPATH"
printf '%s\n' "export EXEEXT          := $EXEEXT"
printf '%s\n' "export PERFORM         := $PERFORM"
