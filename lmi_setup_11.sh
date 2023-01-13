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

. ./lmi_setup_inc.sh
# shellcheck disable=SC1091
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
# was considered, but seemed too extraordinary--see:
#   https://lists.nongnu.org/archive/html/lmi/2020-05/msg00028.html
# Instead, chroot package downloads are kept in their own directory
# (distinct from the host's own cache), and commingled there (because
# they're uniquely named, there's no need to segregate packages by
# OS release, or to separate '.deb' from '.rpm' files).

CACHEDIR=/var/cache/lmi_schroots
mkdir -p "${CACHEDIR}"

# At this point,
#   /srv/
# probably exists already; and
#   /srv/chroot/
# might not exist (in which case 'debootstrap' will create it); but
#   /srv/chroot/"${CHRTNAME}"
# should not exist--debootstrapping into a nonempty directory can
# fail in mysterious ways.
if [ -e /srv/chroot/"${CHRTNAME}" ] ; then echo "Oops."; exit 9; fi

# Make the root directory of the about-to-be-created chroot, and files
# and directories created under it, accessible to the "lmi" group--see:
#   https://lists.nongnu.org/archive/html/lmi/2020-02/msg00007.html
# et seqq.
mkdir -p   /srv/chroot/"${CHRTNAME}"
chgrp lmi  /srv/chroot/"${CHRTNAME}"
chmod 2770 /srv/chroot/"${CHRTNAME}"
umask 0007

# Bootstrap a minimal debian system. Options:
#   --include=zsh, because of "shell=/bin/zsh" below
#   --variant=minbase, as explained here:
#     https://lists.nongnu.org/archive/html/lmi/2020-05/msg00026.html
debootstrap --arch=amd64 --cache-dir="${CACHEDIR}" \
 --variant=minbase --include=zsh \
 "${CODENAME}" /srv/chroot/"${CHRTNAME}" >"${CHRTNAME}"-debootstrap-log 2>&1

# This command should produce no output:
grep --invert-match '^I:' "${CHRTNAME}"-debootstrap-log || true

# Installing 'schroot' creates this 'chroot.d' directory.
cat >/etc/schroot/chroot.d/"${CHRTNAME}".conf <<EOF
[${CHRTNAME}]
# aliases=lmi
description=debian ${CODENAME} cross build ${CHRTVER}
directory=/srv/chroot/${CHRTNAME}
users=${CHROOT_USERS}
groups=${NORMAL_GROUP}
root-groups=root
shell=/bin/zsh
type=directory
profile=lmi_profile
# setup.config=lmi_profile/deliberately_unused
EOF

mkdir -p /etc/schroot/lmi_profile
rm    -f /etc/schroot/lmi_profile/copyfiles
touch    /etc/schroot/lmi_profile/copyfiles
rm    -f /etc/schroot/lmi_profile/nssdatabases
touch    /etc/schroot/lmi_profile/nssdatabases

cat >/etc/schroot/lmi_profile/fstab <<EOF
# For caveats, see:
#    https://lists.nongnu.org/archive/html/lmi/2020-05/msg00040.html
#
# [block comment copied from 'schroot' distribution]
# fstab: static file system information for chroots.
# Note that the mount point will be prefixed by the chroot path
# (CHROOT_PATH)
#
# <file system>        <mount point>           <type>  <options>  <dump>  <pass>
/dev/pts                /dev/pts                none    rw,bind    0       0
/proc                   /proc                   none    rw,bind    0       0
/srv/cache_for_lmi      /srv/cache_for_lmi      none    rw,bind    0       0
/var/cache/lmi_schroots /var/cache/apt/archives none    rw,bind    0       0
# If a debian-within-centos-within-debian intermediate chroot is used,
# then it is convenient to bind a redhat cache mountpoint as well:
/var/cache/lmi_schroots /var/cache/yum          none    rw,bind    0       0
# ...and also a pass-through mount for an intermediate chroot:
/var/cache/lmi_schroots /var/cache/lmi_schroots none    rw,bind    0       0
# ...and setting up those extra mounts in simpler cases costs nothing.
#
# schroot creates a mountpoint if it does not already exist:
# /tmp                  /nonexistent/mountpoint none    rw,bind    0       0
# but of course schroot won't create a nonexistent filesystem:
# /dev/nonexistent      /var/cache/nonexistent  none    rw,bind    0       0
EOF

# Summarize what's already here--nothing if /var/cache/lmi_schroots
# has never been populated, or a full system if these scripts have
# been run previously:
du   -sb /srv/chroot/"${CHRTNAME}"/var/cache/apt/archives

# These two files are necessary, but as of 2023-01 'schroot' provides
# no '/etc/hosts' at all; copy '/etc/nsswitch.conf' as well because it
# has been a problem in the past--see:
#   https://lists.nongnu.org/archive/html/lmi/2022-08/msg00014.html
cp -a /etc/hosts /etc/nsswitch.conf /srv/chroot/"${CHRTNAME}"/etc

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0: Ran 'debootstrap'; configured networking."  | tee /dev/tty
