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

. ./lmi_setup_inc.sh
. /tmp/schroot_env

set -evx

assert_su
assert_not_chrooted

# Add a normal user, and a corresponding group.
#
# This minimal centos chroot lacks openssl, so hardcode a password.

groupadd --gid="${NORMAL_GROUP_GID}" "${NORMAL_GROUP}"
useradd \
  --gid="${NORMAL_GROUP_GID}" \
  --uid="${NORMAL_USER_UID}" \
  --create-home \
  --shell=/bin/zsh \
  --password="\$1\$\$AYD8bMyx6ho3BnmO3jjb60" \
  "${NORMAL_USER}"

# Where debian has a 'sudo' group, redhat has a 'wheel' group.
# The difference seems to be nominal; neither is GID 0.
usermod -aG wheel "${NORMAL_USER}"
# Nevertheless, after exiting the chroot and reentering as 'greg':
#   $groups greg
#   greg : lmi wheel
#   $sudo visudo
#   sudo: no tty present and no askpass program specified
#   $sudo -S visudo
#   greg is not in the sudoers file.  This incident will be reported.
#   $visudo
#   greg is not in the sudoers file.
#   $newgrp wheel
#   $sudo visudo
#   sudo: no tty present and no askpass program specified
# even though the default /etc/sudoers contains:
#   %wheel<Tab>ALL=(ALL)<Tab>ALL
# Enabling the 'wheel' group with no password and disabling
# 'requiretty', as shown below, didn't suffice...so explicitly add
# the normal user:
{
  printf '# Customizations:\n'
  printf '\n'
  printf '# Disable "requiretty" if necessary--see:\n'
  printf '#   https://bugzilla.redhat.com/show_bug.cgi?id=1020147\n'
  printf '# Defaults    !requiretty\n'
  printf '\n'
  printf '%%wheel\tALL=(ALL)\tNOPASSWD: ALL\n'
  printf -- '%s\tALL=(ALL)\tNOPASSWD: ALL\n' "${NORMAL_USER}"
} >/etc/sudoers.d/"${NORMAL_USER}"
chmod 0440 /etc/sudoers.d/"${NORMAL_USER}"
visudo -cs

chsh -s /bin/zsh "${NORMAL_USER}"

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0: Configured user '${NORMAL_USER}'."  | tee /dev/tty
