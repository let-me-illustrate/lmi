#!/bin/sh

# Create a chroot for cross-building "Let me illustrate...".
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

assert_not_su
assert_chrooted

# Initialize wine. See:
#   https://lists.nongnu.org/archive/html/lmi/2016-10/msg00002.html
WINEDLLOVERRIDES=mscoree=d wine wineboot
#
# Don't worry about this:
#   0009:err:file:init_redirects cannot open L"C:\\windows" (c000000f)
# or a dozen or so lines of ':err:ole:' diagnostics mentioning
# "IPSFactory" and "MarshalInterface", or this:
#   0014:err:ole:get_local_server_stream Failed: 80004002
#   Could not load wine-gecko. HTML rendering will be disabled.

# Configure wine:

# First, to make it usable in general ("Default Settings"):
#   on "Applications" tab, set "Windows Version" to "XP"
#   on "Graphics" tab, set DPI to 192
winecfg

# HKCU\Control Panel\International:
#   set sShortDate and sLongDate to "yyyy-MM-dd"
wine regedit

# Troubleshoot wine:
#
# Later, to fix any application that gets the wrong X tab order:
# winecfg
#   "Applications": "add" (probably from drive Z:); then
#   "Graphics": "Emulate virtual desktop"
#     specify appropriate size: e.g., 1900x1120 for a 1920x1200 monitor
#     (the other options don't seem to matter)
#   then restart the application

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0: Installed 'wine'."  | tee /dev/tty
