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
# http://savannah.nongnu.org/projects/lmi
# email: <gchicares@sbcglobal.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

. ./lmi_setup_inc.sh
. /tmp/schroot_env

set -evx

assert_su
# doesn't matter whether chrooted or not

# Configure zsh, for root and normal users.

cp -a .zshrc ~
cp -a ~/.zshrc /home/"${NORMAL_USER}"/.zshrc
chown "${NORMAL_USER}":"${NORMAL_GROUP}" /home/"${NORMAL_USER}"/.zshrc

# Configure vim. Rather than trying to split its contents between
# '~/.vimrc' and '/etc/vim/vimrc.local', use '~/.vimrc' for all
# customizations and copy that file for the normal user too.

cp -a .vimrc ~
cp -a ~/.vimrc /home/"${NORMAL_USER}"/.vimrc
chown "${NORMAL_USER}":"${NORMAL_GROUP}" /home/"${NORMAL_USER}"/.vimrc

# Without this, 'zg' gives an error message; with it, vim creates a
# spellfile the first time 'zg' is used, if none already exists.
mkdir ~/.vim
mkdir /home/"${NORMAL_USER}"/.vim
chown "${NORMAL_USER}":"${NORMAL_GROUP}" /home/"${NORMAL_USER}"/.vim
# It's a much better idea to install a mature spellfile:
mkdir ~/.vim/spell
cp -a en.utf-8.add ~/.vim/spell/en.utf-8.add
mkdir /home/"${NORMAL_USER}"/.vim/spell
chown "${NORMAL_USER}":"${NORMAL_GROUP}" /home/"${NORMAL_USER}"/.vim/spell
cp -a ~/.vim/spell/en.utf-8.add /home/"${NORMAL_USER}"/.vim/spell/en.utf-8.add
chown "${NORMAL_USER}":"${NORMAL_GROUP}" /home/"${NORMAL_USER}"/.vim/spell/en.utf-8.add
# and then (imperatively) run this command:
vim -es -c ':mkspell! ~/.vim/spell/en.utf-8.add' -c ':q'
# which will be repeated later for the normal user.

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0: Configured zsh and vim."  | tee /dev/tty
