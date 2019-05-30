#!/bin/sh

# Use 'shellcheck' to test a shell script.

# Copyright (C) 2019 Gregory W. Chicares.
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

# Skip:
#  - empty argument: no script to check
#  - git's default hooks: the maintainers don't use shellcheck
#  - tabs/ scripts: they're extraordinary

case "$1" in
    ("")                      exit 0 ;;
    (*.git/hooks-orig*)       exit 0 ;;
    (*tabs/*/startup_script*) exit 0 ;;
esac

# First line, truncated at its first blank:
shebang="$(sed -e'1!d' -e's/ .*$//' "$1")"
case $shebang  in
    ("#!/bin/sh")
        shellcheck --external-sources "$1"
    ;;
    ("#!/bin/zsh")
        sed -e'1s/zsh/sh/' "$1" | shellcheck --external-sources -
    ;;
    ("#!/usr/bin/make") ;;
    ("#!/bin/sed") ;;
    (*) printf '%s\n' "unknown shebang $shebang in file $1" ;;
esac
