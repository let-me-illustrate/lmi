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

set -vx

assert_not_su
assert_chrooted

# Kludge:
HOME=/home/"${NORMAL_USER}"

# Symlink directories used by lmi, so that both native and wine
# builds use the same directories and can share the same
# architecture-independent 'configurable_settings.xml'--much like the
# "identity mount" technique used with cygwin. See:
#   https://lists.nongnu.org/archive/html/lmi/2017-05/msg00018.html

mkdir -p ~/.wine/drive_c/users/"${NORMAL_USER}"/opt/
cd ~/.wine/drive_c/users/"${NORMAL_USER}"/opt/ || { printf 'failed: cd\n'; exit 3; }
ln --symbolic --relative --force --no-dereference /opt/lmi/ ./lmi

mkdir -p ~/.wine/drive_c/users/"${NORMAL_USER}"/etc/opt/
cd ~/.wine/drive_c/users/"${NORMAL_USER}"/etc/opt/ || { printf 'failed: cd\n'; exit 3; }
ln --symbolic --relative --force --no-dereference /etc/opt/lmi/ ./lmi

mkdir -p ~/.wine/drive_c/users/"${NORMAL_USER}"/var/opt/
cd ~/.wine/drive_c/users/"${NORMAL_USER}"/var/opt/ || { printf 'failed: cd\n'; exit 3; }
ln --symbolic --relative --force --no-dereference /var/opt/lmi/ ./lmi

cd ~ || { printf 'failed: cd\n'; exit 3; }
# Rebuild vim spellfile (as was done above for root)
vim -es -c ':mkspell! ~/.vim/spell/en.utf-8.add' -c ':q'

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
# Use your own name and email address.
git config --global user.email gchicares@sbcglobal.net
git config --global user.name "Gregory W. Chicares"
