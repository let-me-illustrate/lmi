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

wget -N -nv "${GIT_URL_BASE}"/lmi_setup_00c.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_02.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_02c.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_05c.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_05r.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_07r.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_10.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_10c.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_10r.sh
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

./lmi_setup_10.sh
./lmi_setup_11.sh

# BEGIN ./lmi_setup_13.sh
# For caveats, see:
#    https://lists.nongnu.org/archive/html/lmi/2020-05/msg00040.html
mkdir -p /srv/cache_for_lmi
du   -sb /srv/chroot/"${CHRTNAME}"/srv/cache_for_lmi || echo "Okay."
mkdir -p /srv/chroot/"${CHRTNAME}"/srv/cache_for_lmi
mount --bind /srv/cache_for_lmi /srv/chroot/"${CHRTNAME}"/srv/cache_for_lmi
# END   ./lmi_setup_13.sh

cp -a lmi_setup_*.sh /tmp/schroot_env /srv/chroot/${CHRTNAME}/tmp
schroot --chroot=${CHRTNAME} --user=root             --directory=/tmp ./lmi_setup_20.sh
schroot --chroot=${CHRTNAME} --user=root             --directory=/tmp ./lmi_setup_21.sh
# On a particular corporate server, root is not a sudoer.
if sudo -l true; then
  sudo                       --user="${NORMAL_USER}"                  ./lmi_setup_30.sh
else
  su                                "${NORMAL_USER}"                  ./lmi_setup_30.sh
fi
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
