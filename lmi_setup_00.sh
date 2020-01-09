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

assert_su
assert_not_chrooted

stamp0=$(date -u +'%Y-%m-%dT%H:%M:%SZ')
echo "Started: $stamp0"

wget -N "${GIT_URL_BASE}"/lmi_setup_10.sh
wget -N "${GIT_URL_BASE}"/lmi_setup_11.sh
wget -N "${GIT_URL_BASE}"/lmi_setup_20.sh
wget -N "${GIT_URL_BASE}"/lmi_setup_21.sh
wget -N "${GIT_URL_BASE}"/lmi_setup_30.sh
wget -N "${GIT_URL_BASE}"/lmi_setup_40.sh
wget -N "${GIT_URL_BASE}"/lmi_setup_41.sh
wget -N "${GIT_URL_BASE}"/lmi_setup_42.sh
wget -N "${GIT_URL_BASE}"/lmi_setup_43.sh
wget -N "${GIT_URL_BASE}"/lmi_setup_inc.sh
chmod +x lmi_setup_*.sh

./lmi_setup_10.sh
./lmi_setup_11.sh
cp -a lmi_setup_*.sh /srv/chroot/${CHRTNAME}/tmp
schroot --chroot=${CHRTNAME} --user=root --directory=/tmp ./lmi_setup_20.sh
schroot --chroot=${CHRTNAME} --user=root --directory=/tmp ./lmi_setup_21.sh
sudo -u "${NORMAL_USER}" ./lmi_setup_30.sh
schroot --chroot=${CHRTNAME} --user="${NORMAL_USER}" --directory=/tmp ./lmi_setup_40.sh
schroot --chroot=${CHRTNAME} --user="${NORMAL_USER}" --directory=/tmp ./lmi_setup_41.sh
schroot --chroot=${CHRTNAME} --user="${NORMAL_USER}" --directory=/tmp ./lmi_setup_42.sh
schroot --chroot=${CHRTNAME} --user="${NORMAL_USER}" --directory=/tmp ./lmi_setup_43.sh

stamp1=$(date -u +'%Y-%m-%dT%H:%M:%SZ')
echo "Finished: $stamp1"

seconds=$(($(date '+%s' -d "$stamp1") - $(date '+%s' -d "$stamp0")))
elapsed=$(date -u -d @"$seconds" +'%H:%M:%S')
echo "Elapsed: $elapsed"
