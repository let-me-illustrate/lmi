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

if [ "$(id -u)" -ne 0 ]; then
   echo "Must be run as root."
   exit 1
fi

. ./lmi_setup_inc.sh

# Progressively uncomment these lines as they're tested.

# ./lmi_setup_10.sh
# ./lmi_setup_11.sh
# ./lmi_setup_20.sh
# ./lmi_setup_21.sh
# ./lmi_setup_30.sh
# ./lmi_setup_40.sh
# ./lmi_setup_41.sh
# ./lmi_setup_42.sh
# ./lmi_setup_43.sh
