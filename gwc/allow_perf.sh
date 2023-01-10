#!/bin/sh

# Allow 'perf' to be run by members of a certain group.

# Copyright (C) 2020 Gregory W. Chicares.
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

# The copyright notice above is pro forma only. This is all based on:
#   https://www.kernel.org/doc/html/latest/admin-guide/perf-security.html

# This script may become useful someday, but don't run it for now.
# 'shellcheck' complained of a simple 'exit' here, so...
printf 'DO NOT RUN THIS SCRIPT--PRESS ^C NOW\n'; sleep 60;

# See:
#   https://lists.nongnu.org/archive/html/lmi/2020-09/msg00073.html

cd /usr/bin || { printf 'failed: cd\n'; exit 3; }
groupadd perf_users
chgrp perf_users perf
chmod o-rwx perf
ls -l perf
# if "cap_perfmon" is supported:
# setcap "cap_perfmon,cap_sys_ptrace,cap_syslog=ep" perf
# setcap -v "cap_perfmon,cap_sys_ptrace,cap_syslog=ep" perf
# otherwise:
setcap "38,cap_ipc_lock,cap_sys_ptrace,cap_syslog=ep" perf
setcap -v "38,cap_ipc_lock,cap_sys_ptrace,cap_syslog=ep" perf
getcap perf

echo "usermod -aG perf_users SomeUserName"

# To use the new capabilities without logging out and back in:
#   newgrp perf_users
# but see:
#   https://lists.nongnu.org/archive/html/lmi/2020-09/msg00073.html

# Disliked alternative, which has the virtue of actually working--see:
#   https://lists.nongnu.org/archive/html/lmi/2020-10/msg00017.html
echo 1 >/proc/sys/kernel/perf_event_paranoid
