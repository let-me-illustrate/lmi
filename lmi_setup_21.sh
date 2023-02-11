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
assert_chrooted

# Repair /usr/share/libtool/.../ltmain.sh as indicated here:
#   https://lists.gnu.org/archive/html/libtool-patches/2011-06/msg00001.html
# Do this as root because root owns the file to be patched.

cat >~/ltmain.sh.patch <<EOF
--- /usr/share/libtool/build-aux/ltmain.sh.orig 2016-08-20 12:34:31.000000000 +0000
+++ /usr/share/libtool/build-aux/ltmain.sh 2017-08-10 13:10:28.466155965 +0000
@@ -5555,7 +5555,7 @@
 /* declarations of non-ANSI functions */
 #if defined __MINGW32__
 # ifdef __STRICT_ANSI__
-int _putenv (const char *);
+_CRTIMP int _putenv (const char *);
 # endif
 #elif defined __CYGWIN__
 # ifdef __STRICT_ANSI__
EOF

patch --dry-run --strip=0 --directory=/ <~/ltmain.sh.patch \
 && patch --strip=0 --directory=/ <~/ltmain.sh.patch

# Enable stable and security upgrades--see:
#    https://www.debian.org/releases/stretch/amd64/apds03.html.en#idm4504
# for a 'stretch' example, and these messages:
#   https://lists.nongnu.org/archive/html/lmi/2019-09/msg00046.html
#   https://lists.nongnu.org/archive/html/lmi/2019-09/msg00047.html
# for the '-security' line and for the reason why https is not used.

cat >/etc/apt/sources.list <<EOF
deb http://deb.debian.org/debian/ ${CODENAME} main
deb http://deb.debian.org/debian/ ${CODENAME}-updates main
deb http://security.debian.org/ ${CODENAME}-security main
EOF

# Apply any available upgrades:

apt-get --assume-yes update
apt-get --assume-yes dist-upgrade

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0: Updated debian chroot." | tee /dev/tty || true
