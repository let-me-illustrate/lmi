#!/bin/sh

# Create a chroot for centos-7.
#
# Copyright (C) 2019 Gregory W. Chicares.
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

set -evx

stamp0=$(date -u +'%Y-%m-%dT%H:%M:%SZ')
echo "Started: $stamp0"

cat >/etc/schroot/chroot.d/centos7.conf <<EOF
[centos7]
description=centos-7.7
directory=/srv/chroot/centos7
users=greg
groups=greg
root-groups=root
type=plain
EOF

apt-get update
apt-get --assume-yes install schroot rinse
rinse --arch amd64 --distribution centos-7 \
  --directory /srv/chroot/centos7 \
  --mirror http://mirror.net.cen.ct.gov/centos/7.7.1908/os/x86_64/Packages \

cat >/srv/chroot/centos7/tmp/setup0.sh <<EOF
#!/bin/sh
set -evx

# A _normal_ file /dev/null seems to be created automatically:
#   -rw-r--r-- 1 root root    0 Oct  1 15:44 /dev/null
# so it needs to be removed to create the pseudo-device.

[ -c /dev/null ] || ( rm /dev/null; mknod /dev/null c 1 3)
chmod 666 /dev/null
[ -c /dev/ptmx ] || mknod /dev/ptmx c 5 2
chmod 666 /dev/ptmx
[ -d /dev/pts  ] || mkdir /dev/pts

getent group greg || groupadd --gid=1000 greg
getent passwd greg || useradd --gid=1000 --groups=greg --uid=1000 \
  --create-home --shell=/bin/zsh \
  --password="$(openssl passwd -1 expired)" greg

mountpoint /dev/pts || mount -t devpts -o rw,nosuid,noexec,relatime,mode=600 devpts /dev/pts
mountpoint /proc    || mount -t proc -o rw,nosuid,nodev,noexec,relatime proc /proc

yum --assumeyes install ncurses-term zsh
chsh -s /bin/zsh greg

yum --assumeyes install centos-release-scl
yum-config-manager --enable rhel-server-rhscl-7-rpms
yum --assumeyes install devtoolset-8 rh-git218

# In order to use the tools on the three preceding lines, do:
#   scl enable devtoolset-8 rh-git218 $SHELL
# and then they'll be available in that environment.

# Install a debian chroot inside this centos chroot. See:
#   https://lists.nongnu.org/archive/html/lmi/2019-09/msg00037.html
#
yum --assumeyes install ca-certificates curl nss-pem
rpm -ivh https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
yum install -y debootstrap.noarch
#
mkdir -p /srv/chroot/debian-stable
debootstrap stable /src/chroot/debian-stable http://deb.debian.org/debian/
#
echo Installed debian-stable.
EOF

chmod +x /srv/chroot/centos7/tmp/setup0.sh
schroot --chroot=centos7 --user=root --directory=/tmp ./setup0.sh

cp -a ~/.zshrc /srv/chroot/centos7/home/greg/.zshrc

stamp1=$(date -u +'%Y-%m-%dT%H:%M:%SZ')
echo "Finished: $stamp1"

seconds=$(($(date '+%s' -d "$stamp1") - $(date '+%s' -d "$stamp0")))
elapsed=$(date -u -d @"$seconds" +'%H:%M:%S')
echo "Elapsed: $elapsed"
