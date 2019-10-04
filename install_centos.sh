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

. ./lmi_setup_inc.sh

set -evx

stamp0=$(date -u +'%Y-%m-%dT%H:%M:%SZ')
echo "Started: $stamp0"

cat >/etc/schroot/chroot.d/centos7lmi.conf <<EOF
[centos7lmi]
description=centos-7.7
directory=/srv/chroot/centos7lmi
users=greg
groups=greg
root-groups=root
type=plain
EOF

apt-get update
apt-get --assume-yes install schroot rinse
rinse --arch amd64 --distribution centos-7 \
  --directory /srv/chroot/centos7lmi \
  --mirror http://mirror.net.cen.ct.gov/centos/7.7.1908/os/x86_64/Packages \

# There should be no bind mounts yet:
findmnt --kernel -n --list | grep '\[' |sort

# Now centos has its own cache directory, with subdirectories:
ls /srv/chroot/centos7lmi/var/cache/yum
ls /srv/chroot/centos7lmi/var/cache/yum/x86_64/7/
# There's very little there--perhaps 49272 bytes:
du -sb /srv/chroot/centos7lmi/var/cache/yum

mkdir -p /var/cache/centos_lmi
du -sb /var/cache/centos_lmi
# 'rbind' seems necessary because centos uses subdirs
mount --rbind /var/cache/centos_lmi /srv/chroot/centos7lmi/var/cache/yum || echo "Oops"
findmnt --kernel -n --list | grep '\[' |sort

cat >/srv/chroot/centos7lmi/tmp/setup0.sh <<EOF
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

du -sb /var/cache/yum

yum --assumeyes install ncurses-term zsh
chsh -s /bin/zsh root
chsh -s /bin/zsh greg

du -sb /var/cache/yum

yum --assumeyes install centos-release-scl
yum-config-manager --enable rhel-server-rhscl-7-rpms

du -sb /var/cache/yum

yum --assumeyes install devtoolset-8 rh-git218
# In order to use the tools on the three preceding lines, do:
#   scl enable devtoolset-8 rh-git218 $SHELL
# and then they'll be available in that environment.

du -sb /var/cache/yum

# Install "EPEL". See:
#   https://lists.nongnu.org/archive/html/lmi/2019-09/msg00037.html
yum --assumeyes install ca-certificates curl nss-pem
rpm -ivh https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm

du -sb /var/cache/yum

yum --assumeyes install schroot
# To show available debootstrap scripts:
#   ls /usr/share/debootstrap/scripts

du -sb /var/cache/yum

# Install a debian chroot inside this centos chroot.
yum --assumeyes install debootstrap.noarch
mkdir -p /srv/chroot/"${CHRTNAME}"
debootstrap "${CODENAME}" /srv/chroot/"${CHRTNAME}" http://deb.debian.org/debian/

du -sb /var/cache/yum

echo Installed debian "${CODENAME}".
EOF

chmod +x /srv/chroot/centos7lmi/tmp/setup0.sh
schroot --chroot=centos7lmi --user=root --directory=/tmp ./setup0.sh

du -sb /var/cache/centos_lmi
du -sb /srv/chroot/centos7lmi/var/cache/yum

cp -a ~/.zshrc /srv/chroot/centos7lmi/root/.zshrc
cp -a ~/.zshrc /srv/chroot/centos7lmi/home/greg/.zshrc

cat >/srv/chroot/centos7lmi/etc/schroot/chroot.d/"${CHRTNAME}".conf <<EOF
[${CHRTNAME}]
aliases=lmi
description=debian ${CODENAME} cross build ${CHRTVER}
directory=/srv/chroot/${CHRTNAME}
users=greg
groups=greg
root-groups=root
type=plain
EOF

# This seems to work perfectly:
mkdir /srv/chroot/centos7lmi/srv/chroot/"${CHRTNAME}"/cache_for_lmi
mount --bind /srv/cache_for_lmi /srv/chroot/centos7lmi/srv/chroot/"${CHRTNAME}"/cache_for_lmi || echo "Oops"
findmnt --kernel -n --list | grep '\[' |sort

# At this point, the debian chroot's /var/cache/apt/archives/
# directory exists, but is empty:
du -sb /srv/chroot/centos7lmi/srv/chroot/"${CHRTNAME}"/var/cache/apt/archives
# The host's apt archives contain:
du -sb /var/cache/"${CODENAME}"
findmnt --kernel -n --list | grep '\[' |sort

mount --bind /var/cache/"${CODENAME}" /srv/chroot/centos7lmi/srv/chroot/"${CHRTNAME}"/var/cache/apt/archives || echo "Oops"
findmnt --kernel -n --list | grep '\[' |sort

cat >/srv/chroot/centos7lmi/tmp/setup1.sh <<EOF
#!/bin/sh
set -vx

echo "Adapted from 'lmi_setup_00.sh'."

wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/lmi_setup_10.sh'
wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/lmi_setup_11.sh'
wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/lmi_setup_20.sh'
wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/lmi_setup_21.sh'
wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/lmi_setup_30.sh'
wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/lmi_setup_40.sh'
wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/lmi_setup_41.sh'
wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/lmi_setup_42.sh'
wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/lmi_setup_43.sh'
wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/lmi_setup_inc.sh'
chmod +x lmi_setup_*.sh

. ./lmi_setup_inc.sh

set -vx

# Just get rid of this stanza?
# mkdir -p /var/cache/"${CODENAME}"
# mountpoint /var/cache/apt/archives || mount --bind /var/cache/"${CODENAME}" /srv/chroot/"${CHRTNAME}"/var/cache/apt/archives || echo "Oops"
# mountpoint: /var/cache/apt/archives: No such file or directory
# This stanza (now suppressed) looks like a possible cause of the
# circular bind mounts demonstrated below (search for "eraseme-findmnt"):
# + mount --bind /var/cache/bullseye /srv/chroot/lmi_bullseye_1/var/cache/apt/archives

# At this point, the debian chroot's /var/cache/apt/archives/
# directory exists, but is empty:
du -sb /srv/chroot/"${CHRTNAME}"/var/cache/apt/archives

# ./lmi_setup_10.sh
# ./lmi_setup_11.sh
cp -a lmi_setup_*.sh /srv/chroot/${CHRTNAME}/tmp
schroot --chroot=${CHRTNAME} --user=root --directory=/tmp ./lmi_setup_20.sh
schroot --chroot=${CHRTNAME} --user=root --directory=/tmp ./lmi_setup_21.sh
# sudo -u greg ./lmi_setup_30.sh
schroot --chroot=${CHRTNAME} --user=greg --directory=/tmp ./lmi_setup_40.sh
schroot --chroot=${CHRTNAME} --user=greg --directory=/tmp ./lmi_setup_41.sh
schroot --chroot=${CHRTNAME} --user=greg --directory=/tmp ./lmi_setup_42.sh
schroot --chroot=${CHRTNAME} --user=greg --directory=/tmp ./lmi_setup_43.sh
EOF

chmod +x /srv/chroot/centos7lmi/tmp/setup1.sh
schroot --chroot=centos7lmi --user=root --directory=/tmp ./setup1.sh

du -sb /srv/chroot/centos7lmi/srv/chroot/"${CHRTNAME}"/var/cache/apt/archives
du -sb /var/cache/"${CODENAME}"
findmnt --kernel -n --list | grep '\[' |sort

stamp1=$(date -u +'%Y-%m-%dT%H:%M:%SZ')
echo "Finished: $stamp1"

seconds=$(($(date '+%s' -d "$stamp1") - $(date '+%s' -d "$stamp0")))
elapsed=$(date -u -d @"$seconds" +'%H:%M:%S')
echo "Elapsed: $elapsed"

cat >/tmp/eraseme-findmnt <<EOF
/root[0]#findmnt --kernel -n --list | grep '\[' |sort
/srv/chroot/centos7lmi/srv/chroot/lmi_bullseye_1/var/cache/apt/archives /dev/sdb5[/srv/chroot/centos7lmi/var/cache/bullseye] ext4       rw,relatime,errors=remount-ro
/srv/chroot/centos7lmi/srv/chroot/lmi_bullseye_1/var/cache/apt/archives /dev/sdb5[/var/cache/bullseye]                       ext4       rw,relatime,errors=remount-ro
/var/cache/bullseye                                                     /dev/sdb5[/srv/chroot/centos7lmi/var/cache/bullseye] ext4       rw,relatime,errors=remount-ro
/root[0]#
/root[0]#du -sb /srv/chroot/centos7lmi/srv/chroot/lmi_bullseye_1/var/cache/apt/archives
593658728       /srv/chroot/centos7lmi/srv/chroot/lmi_bullseye_1/var/cache/apt/archives
/root[0]#du -sb /srv/chroot/centos7lmi/var/cache/bullseye
593658728       /srv/chroot/centos7lmi/var/cache/bullseye
/root[0]#du -sb /var/cache/bullseye
593658728       /var/cache/bullseye
/root[0]#
/root[0]#umount /var/cache/bullseye
/root[0]#du -sb /var/cache/bullseye
682609842       /var/cache/bullseye
/root[0]#
/root[0]#findmnt --kernel -n --list | grep '\[' |sort
/srv/chroot/centos7lmi/srv/chroot/lmi_bullseye_1/var/cache/apt/archives /dev/sdb5[/var/cache/bullseye] ext4       rw,relatime,errors=remount-ro
/root[0]#du -sb /srv/chroot/centos7lmi/srv/chroot/lmi_bullseye_1/var/cache/apt/archives
682609842       /srv/chroot/centos7lmi/srv/chroot/lmi_bullseye_1/var/cache/apt/archives
/root[0]#du -sb /srv/chroot/centos7lmi/var/cache/bullseye
593658728       /srv/chroot/centos7lmi/var/cache/bullseye
/root[0]#
/root[0]#du -sb /srv/chroot/centos7lmi/srv/chroot/lmi_bullseye_1/var/cache/apt/archives
682609842       /srv/chroot/centos7lmi/srv/chroot/lmi_bullseye_1/var/cache/apt/archives
/root[0]#du -sb /srv/chroot/centos7lmi/var/cache/bullseye
593658728       /srv/chroot/centos7lmi/var/cache/bullseye
/root[0]#umount /srv/chroot/centos7lmi/srv/chroot/lmi_bullseye_1/var/cache/apt/archives
/root[0]#du -sb /srv/chroot/centos7lmi/srv/chroot/lmi_bullseye_1/var/cache/apt/archives
49157474        /srv/chroot/centos7lmi/srv/chroot/lmi_bullseye_1/var/cache/apt/archives
/root[0]#du -sb /srv/chroot/centos7lmi/var/cache/bullseye
593658728       /srv/chroot/centos7lmi/var/cache/bullseye
/root[0]#du -sb /var/cache/bullseye
682609842       /var/cache/bullseye
EOF
