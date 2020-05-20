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

set -vx

assert_su
assert_not_chrooted

# A _normal_ file /dev/null seems to be created automatically:
#   -rw-r--r-- 1 root root    0 Oct  1 15:44 /dev/null
# so it needs to be removed to create the pseudo-device.

[ -c /dev/null ] || ( rm /dev/null; mknod /dev/null c 1 3)
chmod 666 /dev/null
[ -c /dev/ptmx ] || mknod /dev/ptmx c 5 2
chmod 666 /dev/ptmx
[ -d /dev/pts  ] || mkdir /dev/pts

getent group "${NORMAL_GROUP}" || groupadd --gid="${NORMAL_GROUP_GID}" "${NORMAL_GROUP}"
getent passwd "${NORMAL_USER}" || useradd \
  --gid="${NORMAL_GROUP_GID}" \
  --uid="${NORMAL_USER_UID}" \
  --create-home \
  --shell=/bin/zsh \
  --password="$(openssl passwd -1 expired)" \
  "${NORMAL_USER}"

usermod -aG sudo "${NORMAL_USER}" || echo "Oops."

mountpoint /dev/pts || mount -t devpts -o rw,nosuid,noexec,relatime,mode=600 devpts /dev/pts
mountpoint /proc    || mount -t proc -o rw,nosuid,nodev,noexec,relatime proc /proc

findmnt /var/cache/yum
findmnt /proc
findmnt /dev/pts

sed -i /etc/yum.conf -e's/keepcache=0/keepcache=1/'

yum --assumeyes install ncurses-term less sudo vim zsh
chsh -s /bin/zsh root
chsh -s /bin/zsh "${NORMAL_USER}"
