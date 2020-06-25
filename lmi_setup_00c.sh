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
./lmi_setup_10c.sh

# Installing 'schroot' creates this 'chroot.d' directory.
cat >/etc/schroot/chroot.d/centos7lmi.conf <<EOF
[centos7lmi]
# aliases=deliberately_unused
description=centos-7.7
directory=/srv/chroot/centos7lmi
users=${CHROOT_USERS}
groups=${NORMAL_GROUP}
root-groups=root
shell=/bin/zsh
type=directory
profile=lmi_profile
# setup.config=lmi_profile/deliberately_unused
EOF

mkdir -p /etc/schroot/lmi_profile
rm    -f /etc/schroot/lmi_profile/copyfiles
touch    /etc/schroot/lmi_profile/copyfiles
rm    -f /etc/schroot/lmi_profile/fstab
touch    /etc/schroot/lmi_profile/fstab
rm    -f /etc/schroot/lmi_profile/nssdatabases
touch    /etc/schroot/lmi_profile/nssdatabases

# Use the same cache directory for all chroot package downloads.
#
# Packages ('.deb' as well as '.rpm') are uniquely named, so no
# collision can occur.
#
# Actually, 'rinse' creates a subdirectory like 'centos-7.amd64/', and
# updating the centos chroot using 'yum' creates subdirectories like
# 'x86_64/7/...', so the files remain segregated. Some duplication of
# '.rpm' files may occur, but that's harmless.

CACHEDIR=/var/cache/lmi_schroots
mkdir -p "${CACHEDIR}"

if [ -e /srv/chroot/centos7lmi ] ; then echo "Oops."; exit 9; fi
rinse --arch amd64 --distribution centos-7 \
  --cache-dir "${CACHEDIR}" \
  --directory /srv/chroot/centos7lmi \

# There are probably a few directories here, with no regular files.
du   -sb /srv/chroot/centos7lmi/var/cache/yum || echo "Oops: rinse didn't create cache"
mkdir -p /srv/chroot/centos7lmi/var/cache/yum
mount --bind "${CACHEDIR}" /srv/chroot/centos7lmi/var/cache/yum

echo Installed centos chroot.

cp -a /tmp/schroot_env /srv/chroot/centos7lmi/tmp
cp -a lmi_setup_*.sh   /srv/chroot/centos7lmi/tmp
cp -a .zshrc           /srv/chroot/centos7lmi/tmp
cp -a .vimrc           /srv/chroot/centos7lmi/tmp
cp -a en.utf-8.add     /srv/chroot/centos7lmi/tmp
cp -a install_msw.sh   /srv/chroot/centos7lmi/tmp

./lmi_setup_13c.sh

schroot --chroot=centos7lmi --user=root --directory=/tmp ./lmi_setup_01c.sh

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0 Installed in centos chroot."  | tee /dev/tty
