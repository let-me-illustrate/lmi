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
assert_not_chrooted

./lmi_setup_02.sh
./lmi_setup_05r.sh
./lmi_setup_07r.sh
./lmi_setup_10r.sh
./lmi_setup_11.sh

# BEGIN ./lmi_setup_13.sh
# For caveats, see:
#    https://lists.nongnu.org/archive/html/lmi/2020-05/msg00040.html
mkdir -p /srv/cache_for_lmi
du   -sb /srv/chroot/"${CHRTNAME}"/srv/cache_for_lmi || echo "Okay."
mkdir -p /srv/chroot/"${CHRTNAME}"/srv/cache_for_lmi
mount --bind /srv/cache_for_lmi /srv/chroot/"${CHRTNAME}"/srv/cache_for_lmi
# END   ./lmi_setup_13.sh

cp -a /tmp/schroot_env /srv/chroot/"${CHRTNAME}"/tmp
cp -a lmi_setup_*.sh   /srv/chroot/"${CHRTNAME}"/tmp
cp -a .zshrc           /srv/chroot/"${CHRTNAME}"/tmp
cp -a .vimrc           /srv/chroot/"${CHRTNAME}"/tmp
cp -a en.utf-8.add     /srv/chroot/"${CHRTNAME}"/tmp
cp -a install_msw.sh   /srv/chroot/"${CHRTNAME}"/tmp

schroot --chroot=${CHRTNAME} --user=root             --directory=/tmp ./lmi_setup_20.sh
schroot --chroot=${CHRTNAME} --user=root             --directory=/tmp ./lmi_setup_21.sh
# On a particular corporate server, root is not a sudoer.
if sudo -l true; then
  sudo                       --user="${NORMAL_USER}"                  ./lmi_setup_30.sh
else
  su                                "${NORMAL_USER}"                  ./lmi_setup_30.sh
fi
schroot --chroot=${CHRTNAME} --user="${NORMAL_USER}" --directory=/tmp ./lmi_setup_40.sh
schroot --chroot=${CHRTNAME} --user="${NORMAL_USER}" --directory=/tmp ./lmi_setup_41.sh
schroot --chroot=${CHRTNAME} --user="${NORMAL_USER}" --directory=/tmp ./lmi_setup_42.sh
schroot --chroot=${CHRTNAME} --user="${NORMAL_USER}" --directory=/tmp ./lmi_setup_43.sh
