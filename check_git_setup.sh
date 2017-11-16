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

# Navigate to the directory where this script resides, and make sure
# it's a git "toplevel" directory.

cd $(dirname $(readlink -f $0))
toplevel=$(git rev-parse --show-toplevel)
printf "'%s' is current directory\n" $PWD
printf "'%s' is git toplevel directory\n" $toplevel
[ "$PWD" = "$toplevel" ] || { printf "fail: PWD is not toplevel\n"; exit 1; }

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
#
# The mv command is a nicety; it's okay if it fails because no
# .git/hooks subdirectory exists.

case "$(readlink -f .git/hooks)" in
  ("$PWD/.git/hooks")
    mv .git/hooks .git/hooks-orig && printf "moved" || printf "didn't move"
    printf " old hooks/ directory to hooks-orig/\n"
    printf "creating symlink\n"
    ln --symbolic --force --no-dereference ../hooks .git
    ;;
esac

printf "  'readlink -f .git/hooks':\n"
printf "    expected '%s'\n" "$PWD"/hooks
printf "    observed '%s'\n" "$(readlink -f .git/hooks)"

# Verify that .git/hooks is a symlink to the repository's hooks/
# directory, and that it contains an executable pre-commit hook.
# (There may be other hooks, but that one should always exist.)

case "$(readlink -f .git/hooks)" in
  ("$PWD/hooks")
    [ -x ".git/hooks/pre-commit" ] || { printf "fail: missing hook\n"; exit 2; }
    printf "git hooks directory is properly symlinked\n"
    exit 0
    ;;
  ("$PWD/.git/hooks")
    printf "attempted hooks/ change failed\n"
    exit 3
    ;;
  (*)
    printf "unanticipated error\n"
    exit 4
    ;;
esac

