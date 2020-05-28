#!/bin/sh

# Create a chroot for cross-building "Let me illustrate..." on centos-7.
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

./lmi_setup_02c.sh

cat >/etc/schroot/chroot.d/centos7lmi.conf <<EOF
[centos7lmi]
description=centos-7.7
directory=/srv/chroot/centos7lmi
users=${CHROOT_USERS}
groups=${NORMAL_GROUP}
root-groups=root
shell=/bin/zsh
type=plain
EOF

./lmi_setup_10c.sh

rinse --arch amd64 --distribution centos-7 \
  --directory /srv/chroot/centos7lmi \

mkdir -p /var/cache/centos_lmi
# There are probably a few directories here, with no regular files.
du   -sb /srv/chroot/centos7lmi/var/cache/yum || echo "Oops: rinse didn't create cache"
mkdir -p /srv/chroot/centos7lmi/var/cache/yum
mount --bind /var/cache/centos_lmi /srv/chroot/centos7lmi/var/cache/yum

echo Installed centos chroot.

cp -a /tmp/schroot_env /srv/chroot/centos7lmi/tmp
cp -a lmi_setup_*.sh   /srv/chroot/centos7lmi/tmp

# BEGIN ./lmi_setup_13.sh
CACHEDIR=/var/cache/"${CODENAME}"
mkdir -p "${CACHEDIR}"
mkdir -p /srv/chroot/centos7lmi/"${CACHEDIR}"
mount --bind "${CACHEDIR}" /srv/chroot/centos7lmi/"${CACHEDIR}"

mkdir -p /srv/cache_for_lmi
du   -sb /srv/chroot/centos7lmi/srv/cache_for_lmi || echo "Okay."
mkdir -p /srv/chroot/centos7lmi/srv/cache_for_lmi
mount --bind /srv/cache_for_lmi /srv/chroot/centos7lmi/srv/cache_for_lmi
# END   ./lmi_setup_13.sh

schroot --chroot=centos7lmi --user=root --directory=/tmp ./lmi_setup_01c.sh
