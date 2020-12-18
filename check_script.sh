#!/bin/sh

# Use 'shellcheck' to test a shell script.

# Copyright (C) 2019, 2020 Gregory W. Chicares.
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

# For scripts beginning with '#!/bin/zsh', a pipeline changes that
# hashbang to '#!/bin/sh' and feeds the result into shellcheck.
# In that case, any errors are reported as occurring in file '-',
# so the command below writes the name of the zsh script after any
# error messages.
#
# A fancier alternative might postprocess shellcheck output thus:
#  | sed -e"/^In - line [0-9]*:$/s/-/$1"
# but that would be harder to maintain, and more fragile because
# the format of shellcheck's output might change in future.

case $shebang  in
    ("#!/bin/sh")
        shellcheck --external-sources "$1"
    ;;
    ("#!/bin/zsh")
        sed -e'1s/zsh/sh/' "$1" | shellcheck --external-sources - \
          || { printf '%s\n' "...in file $1"; exit 0; }
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
