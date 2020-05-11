#!/bin/sh

# Create a chroot for centos-7.
#
# Copyright (C) 2019, 2020 Gregory W. Chicares.
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

umask g=rwx

# A known corporate firewall blocks gnu.org even on a GNU/Linux
# server, yet allows github.com:
if curl https://git.savannah.nongnu.org:443 >/dev/null 2>&1 ; then
  GIT_URL_BASE=https://git.savannah.nongnu.org/cgit/lmi.git/plain
else
  GIT_URL_BASE=https://github.com/vadz/lmi/raw/master
fi

wget -N -nv "${GIT_URL_BASE}"/lmi_setup_10.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_11.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_20.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_21.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_30.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_40.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_41.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_42.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_43.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_inc.sh
chmod 0777 lmi_setup_*.sh

. ./lmi_setup_inc.sh

set -evx

assert_su
assert_not_chrooted

# First, destroy any chroot left by a prior run.
grep centos /proc/mounts | cut -f2 -d" " | xargs umount
rm -rf /srv/chroot/centos7lmi
rm /etc/schroot/chroot.d/centos7lmi.conf

# Store dynamic configuration in a temporary file. This method is
# simple and robust, and far better than trying to pass environment
# variables across sudo and schroot barriers.

       NORMAL_USER=$(id -un "$(logname)")
   NORMAL_USER_UID=$(id -u  "$(logname)")

if getent group lmi; then
      NORMAL_GROUP=lmi
  NORMAL_GROUP_GID=$(getent group "$NORMAL_GROUP" | cut -d: -f3)
      CHROOT_USERS=$(getent group "$NORMAL_GROUP" | cut -d: -f4)
else
      NORMAL_GROUP=$(id -gn "$(logname)")
  NORMAL_GROUP_GID=$(id -g  "$(logname)")
      CHROOT_USERS=$(id -un "$(logname)")
fi

cat >/tmp/schroot_env <<EOF
set -v
    CHROOT_USERS=$CHROOT_USERS
    GIT_URL_BASE=$GIT_URL_BASE
    NORMAL_GROUP=$NORMAL_GROUP
NORMAL_GROUP_GID=$NORMAL_GROUP_GID
     NORMAL_USER=$NORMAL_USER
 NORMAL_USER_UID=$NORMAL_USER_UID
set +v
EOF
chmod 0666 /tmp/schroot_env

set -evx

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

apt-get update
apt-get --assume-yes install schroot rinse
rinse --arch amd64 --distribution centos-7 \
  --directory /srv/chroot/centos7lmi \

mkdir -p /var/cache/centos_lmi
# There are probably a few directories here, with no regular files.
du   -sb /srv/chroot/centos7lmi/var/cache/yum || echo "Oops: rinse didn't create cache"
mkdir -p /srv/chroot/centos7lmi/var/cache/yum
# 'rbind' seems necessary because centos uses subdirs
mount --rbind /var/cache/centos_lmi /srv/chroot/centos7lmi/var/cache/yum

mkdir -p /var/cache/"${CODENAME}"
du   -sb /srv/chroot/centos7lmi/srv/chroot/"${CHRTNAME}"/var/cache/apt/archives || echo "Okay."
mkdir -p /srv/chroot/centos7lmi/srv/chroot/"${CHRTNAME}"/var/cache/apt/archives
mount --bind /var/cache/"${CODENAME}" /srv/chroot/centos7lmi/srv/chroot/"${CHRTNAME}"/var/cache/apt/archives

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
EOF

chmod +x /srv/chroot/centos7lmi/tmp/setup0.sh
schroot --chroot=centos7lmi --user=root --directory=/tmp ./setup0.sh

cp -a /tmp/schroot_env /srv/chroot/centos7lmi/tmp

cp -a ~/.vimrc /srv/chroot/centos7lmi/root/.vimrc
cp -a ~/.vimrc /srv/chroot/centos7lmi/home/"${NORMAL_USER}"/.vimrc

# Experimentally, instead of this:
# cp -a ~/.zshrc /srv/chroot/centos7lmi/root/.zshrc
# cp -a ~/.zshrc /srv/chroot/centos7lmi/home/"${NORMAL_USER}"/.zshrc
# do this:
wget -N -nv "${GIT_URL_BASE}"/gwc/.zshrc
cp -a ~/.zshrc /srv/chroot/centos7lmi/root/.zshrc
cp -a ~/.zshrc /srv/chroot/centos7lmi/home/"${NORMAL_USER}"/.zshrc
# If that works well, then treat vim configuration the same way,
# here and elsewhere.

cat >/srv/chroot/centos7lmi/etc/schroot/chroot.d/"${CHRTNAME}".conf <<EOF
[${CHRTNAME}]
aliases=lmi
description=debian ${CODENAME} cross build ${CHRTVER}
directory=/srv/chroot/${CHRTNAME}
users=${CHROOT_USERS}
groups=${NORMAL_GROUP}
root-groups=root
shell=/bin/zsh
type=plain
EOF

mkdir -p /srv/cache_for_lmi
du   -sb /srv/chroot/centos7lmi/srv/cache_for_lmi || echo "Okay."
mkdir -p /srv/chroot/centos7lmi/srv/cache_for_lmi
mount --bind /srv/cache_for_lmi /srv/chroot/centos7lmi/srv/cache_for_lmi
# Might as well do likewise now for ${CHRTNAME} as well.
du   -sb /srv/chroot/centos7lmi/srv/chroot/"${CHRTNAME}"/srv/cache_for_lmi || echo "Okay."
mkdir -p /srv/chroot/centos7lmi/srv/chroot/"${CHRTNAME}"/srv/cache_for_lmi
mount --bind /srv/cache_for_lmi /srv/chroot/centos7lmi/srv/chroot/"${CHRTNAME}"/srv/cache_for_lmi

cat >/srv/chroot/centos7lmi/tmp/setup1.sh <<EOF
#!/bin/sh
set -vx

echo "Adapted from 'lmi_setup_00.sh'."

wget -N -nv "${GIT_URL_BASE}"/lmi_setup_10.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_11.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_20.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_21.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_30.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_40.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_41.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_42.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_43.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_inc.sh
chmod 0777 lmi_setup_*.sh

. ./lmi_setup_inc.sh

set -vx

# ./lmi_setup_10.sh
# ./lmi_setup_11.sh
cp -a lmi_setup_*.sh /tmp/schroot_env /srv/chroot/${CHRTNAME}/tmp
schroot --chroot=${CHRTNAME} --user=root             --directory=/tmp ./lmi_setup_20.sh
schroot --chroot=${CHRTNAME} --user=root             --directory=/tmp ./lmi_setup_21.sh
sudo                         --user="${NORMAL_USER}"                  ./lmi_setup_30.sh
schroot --chroot=${CHRTNAME} --user="${NORMAL_USER}" --directory=/tmp ./lmi_setup_40.sh
schroot --chroot=${CHRTNAME} --user="${NORMAL_USER}" --directory=/tmp ./lmi_setup_41.sh
schroot --chroot=${CHRTNAME} --user="${NORMAL_USER}" --directory=/tmp ./lmi_setup_42.sh
schroot --chroot=${CHRTNAME} --user="${NORMAL_USER}" --directory=/tmp ./lmi_setup_43.sh
EOF

chmod +x /srv/chroot/centos7lmi/tmp/setup1.sh
schroot --chroot=centos7lmi --user=root --directory=/tmp ./setup1.sh

# Copy log files that may be useful for tracking down problems with
# certain commands whose output is voluminous and often uninteresting.
# Embed a timestamp in the copies' names (no colons, for portability).
fstamp=$(date -u +"%Y%m%dT%H%MZ" -d "$stamp0")
cp -a /srv/chroot/centos7lmi/srv/chroot/${CHRTNAME}/home/"${NORMAL_USER}"/log /home/"${NORMAL_USER}"/lmi_rhlog_"${fstamp}"
cp -a /srv/chroot/centos7lmi/srv/chroot/${CHRTNAME}/tmp/${CHRTNAME}-apt-get-log /home/"${NORMAL_USER}"/apt-get-log-"${fstamp}"

stamp1=$(date -u +'%Y-%m-%dT%H:%M:%SZ')
echo "Finished: $stamp1"

seconds=$(($(date -u '+%s' -d "$stamp1") - $(date -u '+%s' -d "$stamp0")))
elapsed=$(date -u -d @"$seconds" +'%H:%M:%S')
echo "Elapsed: $elapsed"
