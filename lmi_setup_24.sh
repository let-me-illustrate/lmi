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
assert_chrooted

# Add a normal user, and a corresponding group.
#
# No attempt is made to set a real password, because that can't be
# done securely in a script. A better password can be set later,
# interactively, if desired. Forcing the dummy password to expire
# immediately, thus (e.g.):
#   chage -d 0 "${NORMAL_USER}"
# may seem like a good idea, but invoking schroot with that userid
# doesn't prompt for a password change.
#
# Hardcode the salt so that repeated openssl invocations yield
# identical results, to avoid gratuitous regressions when comparing
# successive logs.

groupadd --gid="${NORMAL_GROUP_GID}" "${NORMAL_GROUP}"
useradd \
  --gid="${NORMAL_GROUP_GID}" \
  --uid="${NORMAL_USER_UID}" \
  --create-home \
  --shell=/bin/zsh \
  --password="$(openssl passwd -1 --salt '' expired)" \
  "${NORMAL_USER}"

usermod -aG sudo "${NORMAL_USER}" || echo "Oops."

# Add an 'lmi' group, which may be useful in a multi-user chroot.
getent group 1001 || groupadd --gid=1001 lmi || echo "Oops."
usermod -aG lmi "${NORMAL_USER}" || echo "Oops."

chsh -s /bin/zsh "${NORMAL_USER}"

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0: Configured user '${NORMAL_USER}'."  | tee /dev/tty
