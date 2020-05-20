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

# Suppress a nuisance: rh-based distributions provide a default
# zsh logout file that clears the screen.
sed -e'/^[^#]/s/^/# SUPPRESSED # /' -i /etc/zlogout

# Make a more modern 'git' available via 'scl'. This is not needed
# if all real work is done in a debian chroot.
#yum --assumeyes install centos-release-scl
#yum-config-manager --enable rhel-server-rhscl-7-rpms
#yum --assumeyes install devtoolset-8 rh-git218
# In order to use the tools on the three preceding lines, do:
#   scl enable devtoolset-8 rh-git218 $SHELL
# and then they'll be available in that environment.

# Fix weird errors like "Problem with the SSL CA cert (path? access rights?)".
yum --assumeyes install ca-certificates curl nss-pem

# Install "EPEL" by using 'rpm' directly [historical]. See:
#   https://lists.nongnu.org/archive/html/lmi/2019-09/msg00037.html
#rpm -ivh https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
# Instead, use 'yum' to install "EPEL".
#yum --assumeyes install https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
yum --assumeyes install epel-release

yum --assumeyes install schroot
# To show available debootstrap scripts:
#   ls /usr/share/debootstrap/scripts

# Install a debian chroot inside this centos chroot.
yum --assumeyes install debootstrap
mkdir -p /srv/chroot/"${CHRTNAME}"
debootstrap "${CODENAME}" /srv/chroot/"${CHRTNAME}" http://deb.debian.org/debian/

echo Installed debian "${CODENAME}".
