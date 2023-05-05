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

assert_not_su
assert_chrooted

# Suppress unwanted "wine32 is missing" messages--see:
#   https://lists.nongnu.org/archive/html/lmi/2022-06/msg00016.html
export WINEDEBUG=-all,err+all,fixme+all,fixme-hid,fixme-ntdll,fixme-win

# Install lmi for wine.

cd ~ || { printf 'failed: cd\n'; exit 3; }
cp -a /tmp/install_msw.sh .
chmod +x install_msw.sh
logdir=/srv/cache_for_lmi/logs
mkdir -p "${logdir}"
./install_msw.sh >"${logdir}"/lmi-log 2>&1

# Now everything should work much as it does in native msw. To run an
# msw program, prefix its command line with 'wine'. Test the chroot by
# running the lmi binary built in the preceding step after setting
# $DISPLAY, manually (not under control of this script, which should
# run unattended):

# export DISPLAY=":0.0"
# cd /opt/lmi/bin || { printf 'failed: cd\n'; exit 3; }
# wine ./lmi_wx_shared.exe --ash_nazg --data_path=../data

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0: Installed lmi for '$NORMAL_USER'." | tee /dev/tty || true
