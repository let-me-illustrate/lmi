#!/bin/sh

# Create a chroot for cross-building "Let me illustrate...".
#
# Copyright (C) 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

assert_su
assert_not_chrooted

./lmi_setup_05c.sh
./lmi_setup_07r.sh
./lmi_setup_10r.sh
./lmi_setup_11.sh

./lmi_setup_24c.sh
./lmi_setup_25.sh
for user in $(echo "${CHROOT_USERS}" | tr ',' '\n'); do
{
su "${user}" ./lmi_setup_25.sh
} done

cp -a /tmp/schroot_env /srv/chroot/"${CHRTNAME}"/tmp
cp -a lmi_setup_*.sh   /srv/chroot/"${CHRTNAME}"/tmp
cp -a .zshrc           /srv/chroot/"${CHRTNAME}"/tmp
cp -a .vimrc           /srv/chroot/"${CHRTNAME}"/tmp
cp -a en.utf-8.add     /srv/chroot/"${CHRTNAME}"/tmp
cp -a install_msw.sh   /srv/chroot/"${CHRTNAME}"/tmp

schroot --chroot="${CHRTNAME}" --user=root             --directory=/tmp ./lmi_setup_20.sh
schroot --chroot="${CHRTNAME}" --user=root             --directory=/tmp ./lmi_setup_21.sh
schroot --chroot="${CHRTNAME}" --user=root             --directory=/tmp ./lmi_setup_24.sh
schroot --chroot="${CHRTNAME}" --user=root             --directory=/tmp ./lmi_setup_25.sh
for user in $(echo "${CHROOT_USERS}" | tr ',' '\n'); do
{
schroot --chroot="${CHRTNAME}" --user="${user}"        --directory=/tmp ./lmi_setup_25.sh
} done
schroot --chroot="${CHRTNAME}" --user=root             --directory=/tmp ./lmi_setup_29.sh
# On a particular corporate server, root is not a sudoer.
if sudo -l true; then
  sudo                       --user="${NORMAL_USER}"                  ./lmi_setup_30.sh
else
  su                                "${NORMAL_USER}"                  ./lmi_setup_30.sh
fi
for user in $(echo "${CHROOT_USERS}" | tr ',' '\n'); do
{
schroot --chroot="${CHRTNAME}" --user="${user}"        --directory=/tmp ./lmi_setup_40.sh
} done
schroot --chroot="${CHRTNAME}" --user="${NORMAL_USER}" --directory=/tmp ./lmi_setup_42.sh
schroot --chroot="${CHRTNAME}" --user="${NORMAL_USER}" --directory=/tmp ./lmi_setup_43.sh
schroot --chroot="${CHRTNAME}" --user=nemo             --directory=/tmp ./lmi_setup_44.sh

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0 [centos host]" | tee /dev/tty || true
