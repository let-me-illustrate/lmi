#!/bin/sh

# Use 'shellcheck' to test a shell script.

# Copyright (C) 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

# Do nothing for cygwin, which has no 'shellcheck' package.

lmi_build_type=$(/usr/share/misc/config.guess)
case "$lmi_build_type" in
  (*-*-cygwin*) exit 0 ;;
esac

# Skip:
#  - empty argument: no script to check
#  - git's default hooks: the maintainers don't use 'shellcheck'
#  - tabs/ scripts: they're extraordinary

case "$1" in
    ("")                      exit 0 ;;
    (*.git/hooks-orig*)       exit 0 ;;
    (*tabs/*/startup_script*) exit 0 ;;
esac

# First line, truncated at its first blank:
shebang="$(sed -e'1!d' -e's/ .*$//' "$1")"

# Treat scripts beginning with '#!/bin/zsh' as though the shebang
# were '#!/bin/sh', writing shellcheck directives wherever
# zsh-specific features are used. See:
#   https://lists.nongnu.org/archive/html/lmi/2021-08/msg00011.html

case $shebang  in
    ("#!/bin/sh")
        shellcheck --external-sources "$1"
    ;;
    ("#!/bin/zsh")
        shellcheck --external-sources --shell="sh" "$1"
    ;;
    ("#!/usr/bin/make") ;;
    ("#!/bin/sed") ;;
    (*)
        printf '%s\n' "file $1 is executable, but should it be?"
        printf '%s\n' "if it's a script, then its shebang"
        printf '%s\n' "  $shebang"
        printf '%s\n' "is unknown; otherwise, consider running"
        printf '%s\n' "  chmod -x $1"
        printf '%s\n' "to remove the executable bit"
    ;;
esac
