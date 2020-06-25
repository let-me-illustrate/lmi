#!/bin/sh

# Destroy any existing centos chroot for lmi.
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

umount /srv/chroot/centos7lmi/srv/chroot/"${CHRTNAME}"/srv/cache_for_lmi      || true
umount /srv/chroot/centos7lmi/srv/chroot/"${CHRTNAME}"/var/cache/apt/archives || true
umount /srv/chroot/centos7lmi/srv/chroot/"${CHRTNAME}"/dev/pts                || true
umount /srv/chroot/centos7lmi/srv/chroot/"${CHRTNAME}"/proc                   || true
umount /srv/chroot/centos7lmi/srv/cache_for_lmi      || true
umount /srv/chroot/centos7lmi/var/cache/lmi_schroots || true
umount /srv/chroot/centos7lmi/var/cache/yum          || true
umount /srv/chroot/centos7lmi/dev/pts                || true
umount /srv/chroot/centos7lmi/proc                   || true

findmnt -ro SOURCE,TARGET \
  | grep centos7lmi \
  | sed -e's,^[/A-Za-z0-9_-]*[[]\([^]]*\)[]],\1,' \
  | column -t

# Abort if any centos lmi mountpoint remains.
#
# This shouldn't occur, and 'rm --one-file-system' below should be
# safe anyway, yet an actual catastrophe did occur nonetheless.
findmnt | grep 'centos.*lmi' && exit 9

loc0=/srv/chroot/centos7lmi
loc1="$(schroot --chroot=centos7lmi --location)"
if [ -n "${loc1}" ] && [ "${loc0}" != "${loc1}" ]; then
  echo "chroot found in unexpected location--remove it manually"
  exit 9
fi

rm --one-file-system --recursive --force "${loc0}"
if [ -e "${loc0}" ] || [ -e "${loc1}" ] ; then echo "Oops."; exit 9; fi

rm --force /etc/schroot/chroot.d/centos7lmi.conf

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0: Removed old centos chroot."  | tee /dev/tty
