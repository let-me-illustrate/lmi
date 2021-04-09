#!/bin/sh

# Create a chroot for cross-building "Let me illustrate...".
#
# Copyright (C) 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

# CODENAME: debian release wanted in chroot (like `lsb_release --codename`)
# CHRTVER : local serial number (nothing to do with /etc/debian_version)
# CHRTNAME: physical name of chroot (SCHROOT_CHROOT_NAME)

# Variables defined but unused here: this file should only be sourced.
# shellcheck disable=SC2034

set -evx

CODENAME=bullseye
CHRTVER=3
CHRTNAME=lmi_${CODENAME}_${CHRTVER}

umask
umask g=rwx
umask

set +vx

assert_su()
{
if [ "$(id -u)" -ne 0 ]; then
   echo "Must be run as root."
   exit 1
fi
}

assert_not_su()
{
if [ "$(id -u)" -eq 0 ]; then
   echo "Must not be run as root."
   exit 1
fi
}

# Test for the ultimate chroot wherein lmi is built, ignoring any
# intermediate level in a chain of chroots.

assert_chrooted()
{
if [ "$CHRTNAME" != "$SCHROOT_CHROOT_NAME" ]; then
   echo "Must be run in chroot."
   exit 1
fi
}

assert_not_chrooted()
{
if [ "$CHRTNAME" = "$SCHROOT_CHROOT_NAME" ]; then
   echo "Must not be run in chroot."
   exit 1
fi
}
