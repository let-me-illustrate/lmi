#!/bin/sh

# Create a chroot for cross-building "Let me illustrate...".
#
# Copyright (C) 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

# A _normal_ file /dev/null seems to be created automatically:
#   -rw-r--r-- 1 root root    0 Oct  1 15:44 /dev/null
# so it needs to be removed to create the pseudo-device.

[ -c /dev/null ] || ( rm --force /dev/null; mknod /dev/null c 1 3)
chmod 666 /dev/null

# /dev/tty likewise:

[ -c /dev/tty  ] || ( rm --force /dev/tty ; mknod /dev/tty  c 5 0)
chmod 666 /dev/tty
chown root:tty /dev/tty

[ -c /dev/ptmx ] || mknod /dev/ptmx c 5 2
chmod 666 /dev/ptmx
[ -d /dev/pts  ] || mkdir /dev/pts

findmnt /var/cache/yum
findmnt /proc
findmnt /dev/pts

sed -i /etc/yum.conf -e's/keepcache=0/keepcache=1/'

yum --assumeyes install git less ncurses-term sudo vim zsh

chsh -s /bin/zsh root

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0: Reconfigured centos chroot."  | tee /dev/tty
