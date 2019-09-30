#!/bin/sh

# Create a chroot for cross-building "Let me illustrate...".
#
# Copyright (C) 2016, 2017, 2018, 2019 Gregory W. Chicares.
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

set -vx

assert_su
assert_not_chrooted

# Unpack the OS tarball into the particular chroot being created.
mkdir -p /srv/chroot/"${CHRTNAME}"
debootstrap --arch=amd64 --unpack-tarball=/var/cache/"${CODENAME}"_bootstrap.tar \
 "${CODENAME}" /srv/chroot/"${CHRTNAME}" >"${CHRTNAME}"-debootstrap-log 2>&1

# This command should produce no output:
grep --invert-match '^I:' "${CHRTNAME}"-debootstrap-log

cat >/etc/schroot/chroot.d/"${CHRTNAME}".conf <<EOF
[${CHRTNAME}]
aliases=lmi
description=debian ${CODENAME} cross build ${CHRTVER}
directory=/srv/chroot/${CHRTNAME}
users=greg
groups=greg
root-groups=root
type=plain
EOF

# Bind-mount apt archives for the chroot's debian release, to save a
# great deal of bandwidth if multiple chroots are created with the
# same release. Do this:
#   - after invoking 'debootstrap', so that /var exists; and
#   - before invoking 'apt-get' in the chroot, to save bandwidth; and
#   - while not chrooted, so that the host filesystem is accessible.
# The alternative of rbind-mounting parent directory var/cache/apt is
# not used because it's more complicated and has no benefit.
mkdir -p /var/cache/"${CODENAME}"
mount --bind /var/cache/"${CODENAME}" /srv/chroot/"${CHRTNAME}"/var/cache/apt/archives
