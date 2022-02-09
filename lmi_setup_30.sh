#!/bin/sh

# Create a chroot for cross-building "Let me illustrate...".
#
# Copyright (C) 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

assert_not_su
assert_not_chrooted

# To copy the cache in /srv from a different drive:
#
# src=/mnt/sda1/srv/cache_for_lmi
#
# dst=/srv/cache_for_lmi
# sudo mkdir ${dst} &&
#   sudo chown greg:greg ${dst} &&
#   cp --dereference --preserve --recursive ${src}/* ${dst}
#
# dst=/srv/chroot/"${CHRTNAME}"/srv/cache_for_lmi
# sudo mkdir ${dst} &&
#   sudo chown greg:greg ${dst} &&
#   cp --dereference --preserve --recursive ${src}/* ${dst}

# If cached lmi downloads are available elsewhere, copy them now.
# Copying cache_for_lmi/downloads/ is an optional step that merely
# conserves bandwidth. Directory /srv/cache_for_lmi/ in a native msw
# installation also contains cygwin files, which are not wanted in a
# chroot. For convenience, add a bare repository of proprietary files
# to this directory (after making sure it's up to date), e.g.:
#   rm -rf /srv/cache_for_lmi/blessed/proprietary
#   cp --dereference --preserve --recursive \
#     /srv/chroot/some-prior-chroot/opt/lmi/blessed/ /srv/cache_for_lmi
# to update the host; then those files will be accessible in chroots
# created by these scripts (which mount that directory).

# Also copy any desired msw software into the chroot now, e.g.:
#   cp -a /srv/chroot/some-prior-chroot/opt/xyzzy /srv/chroot/${CHRTNAME}/opt/xyzzy
# unless it requires running an "install" program, which must be
# postponed until wine has been installed (in a later script). For
# example, to copy all software installed in subdirectories of /opt/
# other than /opt/lmi/ :
#
# setopt extended_glob &&
#   sudo cp -a /mnt/sda1/srv/chroot/lmi-buster2/opt/^lmi /srv/chroot/"${CHRTNAME}"/opt

# Configure ssh, iff this chroot needs write access to savannah.
# The easiest way is to copy existing credentials, e.g.:
cp -a ~/.ssh/ /srv/chroot/${CHRTNAME}/home/"${NORMAL_USER}" || true
# Make sure the .ssh/config file contains:
#   Protocol 2
#   HashKnownHosts no
# See the discussion of hashing (inter alia) here:
#   https://lists.nongnu.org/archive/html/lmi/2018-01/msg00003.html

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0: Copied optional files."  | tee /dev/tty
