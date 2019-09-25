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

# Install lmi for wine.

cd ~ || { printf 'failed: cd\n'; exit 3; }
wget -N 'http://git.savannah.gnu.org/cgit/lmi.git/plain/install_msw.sh'
chmod +x install_msw.sh
./install_msw.sh >log 2>&1

# Now everything should work much as it does in native msw. To run an
# msw program, prefix its command line with 'wine'. Test the chroot by
# running the lmi binary built in the preceding step:

cd /opt/lmi/bin || { printf 'failed: cd\n'; exit 3; }
wine ./lmi_wx_shared.exe --ash_nazg --data_path=../data
