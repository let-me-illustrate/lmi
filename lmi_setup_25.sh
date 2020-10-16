#!/bin/sh

# Create a chroot for cross-building "Let me illustrate...".
#
# Copyright (C) 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

. ./lmi_setup_inc.sh
# shellcheck disable=SC1091
. /tmp/schroot_env

set -evx

# doesn't matter whether superuser or not
# doesn't matter whether chrooted or not

# Configure zsh.

cp -a .zshrc ~

# Configure vim. Rather than trying to split its contents between
# '~/.vimrc' and '/etc/vim/vimrc.local', use '~/.vimrc' for all
# customizations, and copy that file for each user.

cp -a .vimrc ~

# Without this, 'zg' gives an error message; with it, vim creates a
# spellfile the first time 'zg' is used, if none already exists.
# But it's a much better idea to install a mature spellfile and
# (imperatively) run 'mkspell'. However, 'vim-gtk (2:8.2.0716-3)'
# balks at doing that automatically here:
#  - as given: "No protocol specified"
#  - with '-X' or '-v': silently returns 1
# so it might need to be done manually later.
mkdir -p ~/.vim/spell
cp -a en.utf-8.add ~/.vim/spell/en.utf-8.add
vim -es -c ':mkspell! ~/.vim/spell/en.utf-8.add' -c ':q' ||
  echo "Run ':mkspell! ~/.vim/spell/en.utf-8.add' manually."

# Configure git. See:
#   https://lists.nongnu.org/archive/html/lmi/2016-03/msg00006.html
git config --global color.ui auto
git config --global commit.cleanup scissors
git config --global core.pager "less -+F -+X"
git config --global diff.colormoved plain
git config --global log.date iso8601-strict-local
git config --global log.follow true
git config --global pull.ff only
git config --global push.default simple
git config --global push.recurseSubmodules check
if [ "greg" = "$(whoami)" ]; then
git config --global user.email gchicares@sbcglobal.net
git config --global user.name "Gregory W. Chicares"
fi

# Avoid "fatal: Invalid cleanup mode scissors" with ancient git.
# This is just a temporary workaround that can be deleted once RHEL
# catches up to git-2.x, so nothing more elaborate is warranted.
case "$(git --version)" in
  ("git version 1*")
    git config --global --unset commit.cleanup
    ;;
esac

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0: Configured {zsh,vim,git} for user '$(whoami)'."  | tee /dev/tty
