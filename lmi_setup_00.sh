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

set -vx

. ./lmi_setup_inc.sh

assert_su
assert_not_chrooted

wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/lmi_setup_10.sh'
wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/lmi_setup_11.sh'
wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/lmi_setup_20.sh'
wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/lmi_setup_21.sh'
wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/lmi_setup_30.sh'
wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/lmi_setup_40.sh'
wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/lmi_setup_41.sh'
wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/lmi_setup_42.sh'
wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/lmi_setup_43.sh'

./lmi_setup_10.sh
./lmi_setup_11.sh
cp -a lmi_setup_*.sh /srv/chroot/${CHRTNAME}/tmp
schroot --chroot=${CHRTNAME} --user=root --directory=/tmp ./lmi_setup_20.sh
schroot --chroot=${CHRTNAME} --user=root --directory=/tmp ./lmi_setup_21.sh
sudo -u greg ./lmi_setup_30.sh
schroot --chroot=${CHRTNAME} --user=greg --directory=/tmp ./lmi_setup_40.sh
schroot --chroot=${CHRTNAME} --user=greg --directory=/tmp ./lmi_setup_41.sh
schroot --chroot=${CHRTNAME} --user=greg --directory=/tmp ./lmi_setup_42.sh
schroot --chroot=${CHRTNAME} --user=greg --directory=/tmp ./lmi_setup_43.sh
