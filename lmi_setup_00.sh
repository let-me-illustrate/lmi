#!/bin/sh

# Create a chroot for cross-building "Let me illustrate...".
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

set -evx

stamp0=$(date -u +'%Y-%m-%dT%H:%M:%SZ')
echo "Started: $stamp0"

# A known corporate firewall blocks gnu.org even on a GNU/Linux
# server, yet allows github.com:
if curl https://git.savannah.nongnu.org:443 >/dev/null 2>&1 ; then
  GIT_URL_BASE=https://git.savannah.nongnu.org/cgit/lmi.git/plain
else
  GIT_URL_BASE=https://github.com/let-me-illustrate/lmi/raw/master
fi

# Store dynamic configuration in a temporary file. This method is
# simple and robust, and far better than trying to pass environment
# variables across sudo and schroot barriers.

       NORMAL_USER=$(id -un "$(logname)")

if getent group lmi; then
      NORMAL_GROUP=lmi
  NORMAL_GROUP_GID=$(getent group "$NORMAL_GROUP" | cut -d ':' -f3)
      CHROOT_USERS=$(getent group "$NORMAL_GROUP" | cut -d ':' -f4)
else
      NORMAL_GROUP=$(id -gn "$(logname)")
  NORMAL_GROUP_GID=$(id -g  "$(logname)")
      CHROOT_USERS=$(id -un "$(logname)")
fi

CHROOT_UIDS=
for user in $(echo "${CHROOT_USERS}" | tr ',' ' '); do
  uid=$(id -u "${user}")
  [ -z "${uid}" ] && echo "Oops."
  CHROOT_UIDS="${CHROOT_UIDS},${uid}"
done
# Remove leading delimiter.
CHROOT_UIDS=$(echo "${CHROOT_UIDS}" | sed -e's/^,//')

cat >/tmp/schroot_env <<EOF
set -v
     CHROOT_UIDS=$CHROOT_UIDS
    CHROOT_USERS=$CHROOT_USERS
    GIT_URL_BASE=$GIT_URL_BASE
    NORMAL_GROUP=$NORMAL_GROUP
NORMAL_GROUP_GID=$NORMAL_GROUP_GID
     NORMAL_USER=$NORMAL_USER
set +v
EOF
chmod 0666 /tmp/schroot_env

wget -N -nv "${GIT_URL_BASE}"/gwc/.zshrc
wget -N -nv "${GIT_URL_BASE}"/gwc/.vimrc
wget -N -nv "${GIT_URL_BASE}"/gwc/.vim/spell/en.utf-8.add
chmod 0644 .zshrc .vimrc en.utf-8.add
wget -N -nv "${GIT_URL_BASE}"/install_msw.sh
chmod 0777 install_msw.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_00c.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_01.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_01c.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_01r.sh
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
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_24.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_24c.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_25.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_29.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_30.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_40.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_42.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_43.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_44.sh
wget -N -nv "${GIT_URL_BASE}"/lmi_setup_inc.sh
chmod 0777 lmi_setup_*.sh

. ./lmi_setup_inc.sh

set -evx

assert_su
assert_not_chrooted

case "$(cat /proc/version)" in
    (*Debian*)    flavor_guess=debian ;;
    (*"Red Hat"*) flavor_guess=redhat ;;
    (*)           flavor_guess=debian
        printf '%s\n' "OS not detected--assuming debian."
        ;;
esac

# The 'centos' flavor is extraordinary. It calls for creating a centos
# chroot with a debian chroot inside. Override 'flavor' on the command
# line to use it, e.g.:
#   flavor=centos ./lmi_setup_00.sh
# The "_01" scripts construct a debian-testing chroot. The 'centos'
# case correctly selects a "00" script, which first constructs a
# centos chroot, within which a debian-testing chroot is constructed.

flavor=${flavor:-"$flavor_guess"}
case "${flavor}" in
    (debian) flavor_script=lmi_setup_01.sh  ;;
    (centos) flavor_script=lmi_setup_00c.sh ;;
    (redhat) flavor_script=lmi_setup_01r.sh ;;
    (*) printf '%s\n' "Unanticipated case--exiting."; exit 3 ;;
esac

logdir=/srv/cache_for_lmi/logs
mkdir -p "${logdir}"
./"${flavor_script}" >"${logdir}/${flavor}-log" 2>&1

# Timestamp suffix for log file names (no colons, for portability).
fstamp=$(date -u +"%Y%m%dT%H%MZ" -d "$stamp0")

# Copy log files that may be useful for tracking down problems with
# certain commands whose output is voluminous and often uninteresting.
# Archive them for easy sharing.
#
# It would be easier to specify the files to be archived as
#   ./*"${fstamp}
# but that would cause 'tar' to fail thus:
#   tar: [filename]: file changed as we read it
# if this script's output is redirected to a file that's uniquified
# by the same $fstamp method used here.

(cd "${logdir}"
logfiles=
for z in \
    "${CHRTNAME}"-debootstrap-log \
    "${CHRTNAME}"-apt-get-log \
    lmi-log \
    "${flavor}-log" \
  ; do
  z_stamped="${z}_${fstamp}"
  mv "${z}" "${z_stamped}"
  logfiles="${logfiles} ${z_stamped}"
done
# Word-splitting is definitely wanted for ${logfiles} here:
# shellcheck disable=SC2086
tar -cJvf chroot-logs_"${fstamp}".tar.xz ${logfiles}
)

stamp1=$(date -u +'%Y-%m-%dT%H:%M:%SZ')
echo "Finished: $stamp1"

seconds=$(($(date -u '+%s' -d "$stamp1") - $(date -u '+%s' -d "$stamp0")))
elapsed=$(date -u -d @"$seconds" +'%H:%M:%S')

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0 Installed and tested lmi."    | tee /dev/tty
echo "Elapsed: $elapsed; log suffix: $fstamp" | tee /dev/tty
