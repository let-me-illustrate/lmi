#!/bin/sh

# Ensure sound git configuration.

# Copyright (C) 2017 Gregory W. Chicares.
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

# For msw (cygwin) only, make sure 'core.filemode' is "false". See:
#   https://lists.nongnu.org/archive/html/lmi/2017-11/msg00018.html

case $(uname -s) in
  (CYGWIN*)
    printf "cygwin detected\n"
    git config core.filemode false
    ;;
  (*)
    printf "cygwin not detected--assuming OS is POSIX\n"
    ;;
esac

printf "core.filemode is '%s'\n" $(git config --get-all core.filemode)

# Make sure the hooks in the repository's hooks/ directory are used.
# Do this in the directory where this script resides, which is
# deliberately the "toplevel" directory.

cd $(dirname $(readlink --canonicalize $0))

case "$(readlink -f .git/hooks)" in
  ("$(pwd)/.git/hooks")
    printf "moving old hooks directory to hooks-orig/ and creating symlink\n"
    mv .git/hooks .git/hooks-orig && ln --symbolic --force --no-dereference ../hooks .git
    ;;
esac

printf "  'readlink -f .git/hooks':\n"
printf "    expected '%s'\n" "$(pwd)"/hooks
printf "    observed '%s'\n" "$(readlink -f .git/hooks)"

case "$(readlink -f .git/hooks)" in
  ("$(pwd)/hooks")
    printf "git hooks directory is properly symlinked\n"
    exit 0
    ;;
  ("$(pwd)/.git/hooks")
    printf "attempted hooks/ change failed\n"
    exit 1
    ;;
  (*)
    printf "unanticipated error\n"
    exit 2
    ;;
esac

