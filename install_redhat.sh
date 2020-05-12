#!/bin/sh

# Create a chroot for redhat-7.
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
grep "${CHRTNAME}" /proc/mounts | cut -f2 -d" " | xargs --no-run-if-empty umount
rm -rf /srv/chroot/"${CHRTNAME}"
rm /etc/schroot/chroot.d/"${CHRTNAME}".conf || echo "None?"
umount /srv/chroot

# On a server with tiny 4G partitions for /usr, /var, /tmp, /opt,
# etc., no partition had room for a chroot. Using RAM:
#   mount -t tmpfs -o size=10G tmpfs /srv
# was sufficient for a proof of concept, but for real work it's
# necessary to obtain sufficient storage, e.g.:
#   parted --align optimal /dev/sdb -- mklabel msdos mkpart primary ext4 1MiB -1MiB
#   e2label /dev/sdb1 lmi
#   mkdir /lmi
#   chgrp lmi /lmi
#   echo "LABEL=lmi /srv/chroot ext4 defaults 0 0" >> /etc/fstab
#   partprobe
#   mount -a
# Here, explicitly remount /srv/chroot because it was umounted above:
mount LABEL=lmi /srv/chroot
findmnt /srv/chroot

mkdir -p /var/cache/"${CODENAME}"
du   -sb /srv/chroot/"${CHRTNAME}"/var/cache/apt/archives || echo "Okay."
mkdir -p /srv/chroot/"${CHRTNAME}"/var/cache/apt/archives
mount --bind /var/cache/"${CODENAME}" /srv/chroot/"${CHRTNAME}"/var/cache/apt/archives

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
# Try 'update' before 'install' as described here:
#   https://lists.nongnu.org/archive/html/lmi/2020-02/msg00003.html
yum --assumeyes update ca-certificates curl nss-pem
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

# Install a debian chroot inside this redhat chroot.
yum --assumeyes install debootstrap
mkdir -p /srv/chroot/"${CHRTNAME}"
debootstrap "${CODENAME}" /srv/chroot/"${CHRTNAME}" http://deb.debian.org/debian/

echo Installed debian "${CODENAME}".

# Suppress a nuisance: debian-based distributions provide a default
# bash logout file that clears the screen.
sed -e'/^[^#]/s/^/# SUPPRESSED # /' -i /srv/chroot/"${CHRTNAME}"/etc/skel/.bash_logout

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

cat >/etc/schroot/chroot.d/"${CHRTNAME}".conf <<EOF
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
du   -sb /srv/chroot/"${CHRTNAME}"/srv/cache_for_lmi || echo "Okay."
mkdir -p /srv/chroot/"${CHRTNAME}"/srv/cache_for_lmi
mount --bind /srv/cache_for_lmi /srv/chroot/"${CHRTNAME}"/srv/cache_for_lmi

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

# Copy log files that may be useful for tracking down problems with
# certain commands whose output is voluminous and often uninteresting.
# Embed a timestamp in the copies' names (no colons, for portability).
fstamp=$(date -u +"%Y%m%dT%H%MZ" -d "$stamp0")
cp -a /srv/chroot/${CHRTNAME}/home/"${NORMAL_USER}"/log /home/"${NORMAL_USER}"/lmi_rhlog_"${fstamp}"
cp -a /srv/chroot/${CHRTNAME}/tmp/${CHRTNAME}-apt-get-log /home/"${NORMAL_USER}"/apt-get-log-"${fstamp}"

stamp1=$(date -u +'%Y-%m-%dT%H:%M:%SZ')
echo "Finished: $stamp1"

seconds=$(($(date -u '+%s' -d "$stamp1") - $(date -u '+%s' -d "$stamp0")))
elapsed=$(date -u -d @"$seconds" +'%H:%M:%S')
echo "Elapsed: $elapsed"

echo Finished creating debian chroot. >/dev/tty
