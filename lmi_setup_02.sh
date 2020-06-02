#!/bin/sh

# Destroy any existing chroot named in './lmi_setup_inc.sh'.
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

set -vx

assert_su
assert_not_chrooted

# umount expected mounts, then list any that seem to have been missed.
#
# It might seem snazzier to extract the relevant field of
#   grep "${CHRTNAME}" /proc/mounts
# and pipe it into
#   xargs umount
# but that would do something astonishing if two chroots (one nested
# and the other not) have mounted /proc thus:
#   proc /srv/chroot/"${CHRTNAME}"/proc
#   proc /srv/chroot/centos7lmi/srv/chroot/"${CHRTNAME}"/proc
# and only the non-nested one is intended to be destroyed.
#
# The 'findmnt' invocation is elaborated thus:
#   -r
# [see "column -t" below];
#   o SOURCE,TARGET
# because the natural order is {real-name, pseudonym}:
#   eric blair alias george orwell
# cf. /proc/self/mountstats:
#   device proc mounted on /srv/chroot/"${CHRTNAME}"/proc
# and thus, for commands, {existing, to-be-created}:
#   cp existing_file copied_file
#   ln -s original_name link_name
#   mount /dev/sda1 /
# but by default 'findmnt' reverses that order (its other default
# fields are generally less interesting for chroot bind mounts);
#   | grep "${CHRTNAME}"
# because 'findmnt -T' doesn't find substrings;
#   | sed -e's,^[/A-Za-z0-9_-]*[[]\([^]]*\)[]],\1,'
# because in output like
#   /dev/sdb5[/srv/cache_for_lmi]
#   /dev/mapper/VolGroup00-1v_root[/srv/cache_for_lmi]
# the part in brackets is the more interesting, while the "/dev..."
# part is distracting (and the '--nofsroot' option would discard the
# more interesting part)--yet bear in mind that the part in brackets
# may be incomplete if a leading component like '/srv' is mounted on
# a distinct device (and '--canonicalize' wouldn't fix that); and
#   | column -t
# along with '-r' because '-l' does a poor job of columnization.

umount /srv/chroot/"${CHRTNAME}"/var/cache/apt/archives
umount /srv/chroot/"${CHRTNAME}"/dev/pts
umount /srv/chroot/"${CHRTNAME}"/proc

findmnt -ro SOURCE,TARGET \
  | grep "${CHRTNAME}" \
  | sed -e's,^[/A-Za-z0-9_-]*[[]\([^]]*\)[]],\1,' \
  | column -t

# Use '--one-file-system' because it was designed for this use case:
#   https://lists.gnu.org/archive/html/bug-coreutils/2006-10/msg00332.html
# | This option is useful when removing a build "chroot" hierarchy
#
# Use 'schroot --location' rather than /srv/chroot/"$CHRTNAME" because
# chroots need not be located in /srv .

rm --one-file-system --recursive --force \
  "$(schroot --chroot="${CHRTNAME}" --location)"

# schroot allows configuration files in /etc/schroot/chroot.d/ only.

rm /etc/schroot/chroot.d/"${CHRTNAME}".conf

# These commands fail harmlessly if the chroot doesn't already exist.

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0: Removed old chroot."  | tee /dev/tty
