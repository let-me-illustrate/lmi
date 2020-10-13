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
# https://savannah.nongnu.org/projects/lmi
# email: <gchicares@sbcglobal.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

. ./lmi_setup_inc.sh
# shellcheck disable=SC1091
. /tmp/schroot_env

set -evx

assert_su
assert_chrooted

groupadd --gid="${NORMAL_GROUP_GID}" "${NORMAL_GROUP}"

# Add an 'lmi' group, which may be useful in a multi-user chroot.
getent group 1001 || groupadd --gid=1001 lmi || echo "Oops."

i=1
while true
do
{
  uid=$( echo "${CHROOT_UIDS}"  | cut -d ',' -f"${i}")
  user=$(echo "${CHROOT_USERS}" | cut -d ',' -f"${i}")
  if [ -z "${user}" ] && [ -z "${uid}" ] ; then break; fi
  if [ -z "${user}" ] || [ -z "${uid}" ] ; then echo "Oops."; exit 9; fi
  # Now ${user} and ${uid} have corresponding values.
  i=$((i + 1))

  # Add a normal user, setting its shell and groups.
  #
  # No attempt is made to set a real password, because that can't be
  # done securely in a script. A better password can be set later,
  # interactively, if desired. Forcing the dummy password to expire
  # immediately, thus (e.g.):
  #   chage -d 0 "${user}"
  # may seem like a good idea, but invoking schroot with that userid
  # doesn't prompt for a password change.
  #
  # Hardcode the salt so that repeated openssl invocations yield
  # identical results, to avoid gratuitous regressions when comparing
  # successive logs.

  useradd \
    --gid="${NORMAL_GROUP_GID}" \
    --uid="${uid}" \
    --create-home \
    --shell=/bin/zsh \
    --password="$(openssl passwd -1 --salt '' expired)" \
    "${user}"

  usermod -aG lmi  "${user}" || echo "Oops."
  usermod -aG sudo "${user}" || echo "Oops."

  chsh -s /bin/zsh "${user}"
} done

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0: Configured users."  | tee /dev/tty
