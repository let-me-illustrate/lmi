#!/bin/sh

# Destroy all existing centos chroots, brutally.
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

# This brutal approach assumes that only one centos chroot exists.
# That's a workable assumption if a unique centos chroot is used
# only to emulate an inconvenient corporate redhat server. See
# 'lmi_setup_02.sh' for a more thoughtful approach.
#
# The grep command conveniently finds not only mounts created to
# support the centos chroot itself, but also mounts created to
# support a debian chroot within the centos chroot.

if [ "greg" != "$(logname)" ]; then
   echo "This script would eradicate all your centos chroots--beware."
   exit 1
fi

grep centos /proc/mounts | cut -f2 -d" " | xargs --no-run-if-empty umount
rm -rf /srv/chroot/centos7lmi
rm /etc/schroot/chroot.d/centos7lmi.conf

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0: Removed old centos chroot."  | tee /dev/tty
