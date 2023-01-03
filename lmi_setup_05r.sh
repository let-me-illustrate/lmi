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

# The intention here was to un-mount and re-mount, in the hope of
# making the server work more reliably across reboots (perhaps because
# the earliest versions of this script used a temporary filesystem).
# However, now it fails (perhaps because of a lingering session of an
# earlier chroot that used a differently-mounted /srv) with a message
# like:
#   umount: /srv: target is busy.
# so maybe it's not appropriate default behavior.
#umount /srv

# On a server with tiny 4G partitions for /usr, /var, /tmp, /opt,
# etc., no partition had room for a chroot. Using RAM:
#   mount -t tmpfs -o size=10G tmpfs /srv
# was sufficient for a proof of concept, but for real work that must
# survive a reboot, it's necessary to obtain sufficient storage, e.g.:
#   parted --align optimal /dev/sdb -- mklabel msdos mkpart primary ext4 1MiB -1MiB
#   e2label /dev/sdb1 lmi
#   mkdir /lmi
#   chgrp lmi /lmi
#   echo "LABEL=lmi /srv ext4 defaults 0 0" >> /etc/fstab
#   partprobe
#   mount -a
# To change from ext4 to xfs, unmount and...
#   mkfs.xfs -f -L lmi /dev/sdb1
# after which
#   blkid /dev/sdb1
# should show
#   /dev/sdb2: LABEL="lmi" ... TYPE="xfs"
# Then do:
#   echo "LABEL=lmi /srv xfs defaults 0 0" >> /etc/fstab
# removing any former /srv line.
#
# Here, explicitly remount /srv if it was umounted above:
# mount LABEL=lmi /srv
findmnt /srv

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0: Reconfigured redhat chroot."  | tee /dev/tty
