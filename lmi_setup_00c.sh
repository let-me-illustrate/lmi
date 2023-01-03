#!/bin/sh

# Create a chroot for cross-building "Let me illustrate..." on centos-7.
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
rm    -f /etc/schroot/lmi_profile/nssdatabases
touch    /etc/schroot/lmi_profile/nssdatabases

cat >/etc/schroot/lmi_profile/fstab <<EOF
# For caveats, see:
#    https://lists.nongnu.org/archive/html/lmi/2020-05/msg00040.html
#
# [block comment copied from 'schroot' distribution]
# fstab: static file system information for chroots.
# Note that the mount point will be prefixed by the chroot path
# (CHROOT_PATH)
#
# <file system>        <mount point>           <type>  <options>  <dump>  <pass>
/dev/pts                /dev/pts                none    rw,bind    0       0
/proc                   /proc                   none    rw,bind    0       0
/srv/cache_for_lmi      /srv/cache_for_lmi      none    rw,bind    0       0
/var/cache/lmi_schroots /var/cache/apt/archives none    rw,bind    0       0
# If a debian-within-centos-within-debian intermediate chroot is used,
# then it is convenient to bind a redhat cache mountpoint as well:
/var/cache/lmi_schroots /var/cache/yum          none    rw,bind    0       0
# ...and also a pass-through mount for an intermediate chroot:
/var/cache/lmi_schroots /var/cache/lmi_schroots none    rw,bind    0       0
# ...and setting up those extra mounts in simpler cases costs nothing.
#
# schroot creates a mountpoint if it does not already exist:
# /tmp                  /nonexistent/mountpoint none    rw,bind    0       0
# but of course schroot won't create a nonexistent filesystem:
# /dev/nonexistent      /var/cache/nonexistent  none    rw,bind    0       0
EOF

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

# There are probably a few directories here, with few regular files,
# because this script is intended to be run on a yum-free debian host.
du   -sb /srv/chroot/centos7lmi/var/cache/yum || echo "Oops: rinse didn't create cache"
find /srv/chroot/centos7lmi/var/cache/yum -type f -print0 | xargs -0 ls -l

echo Installed centos chroot.

cp -a /tmp/schroot_env /srv/chroot/centos7lmi/tmp
cp -a lmi_setup_*.sh   /srv/chroot/centos7lmi/tmp
cp -a .zshrc           /srv/chroot/centos7lmi/tmp
cp -a .vimrc           /srv/chroot/centos7lmi/tmp
cp -a en.utf-8.add     /srv/chroot/centos7lmi/tmp
cp -a install_msw.sh   /srv/chroot/centos7lmi/tmp

schroot --chroot=centos7lmi --user=root --directory=/tmp ./lmi_setup_01c.sh

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0 Installed in centos chroot."  | tee /dev/tty
