#!/bin/sh

# Create a chroot for cross-building "Let me illustrate...".
#
# Copyright (C) 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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
assert_chrooted

# Create directories required by lmi. Only root can create all these
# directories, but they must be owned by a normal user.

mkdir --parents /opt/lmi
chmod g=u+s     /opt/lmi
chown "${NORMAL_USER}":"${NORMAL_GROUP}" /opt/lmi
mkdir --parents /etc/opt/lmi
chmod g=u+s     /etc/opt/lmi
chown "${NORMAL_USER}":"${NORMAL_GROUP}" /etc/opt/lmi
mkdir --parents /var/opt/lmi
chmod g=u+s     /var/opt/lmi
chown "${NORMAL_USER}":"${NORMAL_GROUP}" /var/opt/lmi
mkdir --parents /srv/cache_for_lmi
chmod g=u+s     /srv/cache_for_lmi
chown "${NORMAL_USER}":"${NORMAL_GROUP}" /srv/cache_for_lmi

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0: Created lmi directories."  | tee /dev/tty
