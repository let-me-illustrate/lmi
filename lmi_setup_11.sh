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

# Cache apt archives for the chroot's debian release, to save a great
# deal of bandwidth if multiple chroots are created with the same
# release. Do this:
#   - before invoking 'debootstrap' (or 'apt-get' in the chroot),
#     so that all packages are cached; and
#   - while not chrooted, so that the host filesystem is accessible.
# The alternative of bind-mounting parent directory var/cache/apt
# (using a single directory to store '.deb' files for all releases)
# might be investigated--see:
#   https://lists.nongnu.org/archive/html/lmi/2020-05/msg00028.html
CACHEDIR=/var/cache/"${CODENAME}"
mkdir -p "${CACHEDIR}"

# Bootstrap a minimal debian system. Options:
#   --include=zsh, because of "shell=/bin/zsh" below
#   --variant=minbase, as explained here:
#     https://lists.nongnu.org/archive/html/lmi/2020-05/msg00026.html
mkdir -p /srv/chroot/"${CHRTNAME}"
debootstrap --arch=amd64 --cache-dir="${CACHEDIR}" \
 --variant=minbase --include=zsh \
 "${CODENAME}" /srv/chroot/"${CHRTNAME}" >"${CHRTNAME}"-debootstrap-log 2>&1

# This command should produce no output:
grep --invert-match '^I:' "${CHRTNAME}"-debootstrap-log || true

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

# Experimentally show whether anything's already here:
du   -sb /srv/chroot/"${CHRTNAME}"/var/cache/apt/archives
# Bind-mount apt archives for the chroot's debian release. Do this:
#   - after invoking 'debootstrap', so the chroot's /var exists; and
#   - before invoking 'apt-get' in the chroot, to save bandwidth; and
#   - while not chrooted, so that the host filesystem is accessible.
mount --bind "${CACHEDIR}" /srv/chroot/"${CHRTNAME}"/var/cache/apt/archives

# Should the next two commands both find the mount just established?
findmnt "${CACHEDIR}"
findmnt /srv/chroot/"${CHRTNAME}"/var/cache/apt/archives

# Are the next two commands useful?
findmnt /proc
findmnt /dev/pts

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0: Ran 'debootstrap'."  | tee /dev/tty
