#!/bin/sh

# Hard-link host's 'perf' and its dependencies for use in a chroot.

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

# See
#   https://lists.nongnu.org/archive/html/lmi/2020-10/msg00016.html
# for an initial attempt that used 'cp' instead of 'ln' (and included
# a good deal of relevant discussion), and
#   https://lists.nongnu.org/archive/html/lmi/2020-10/msg00019.html
# for an improvement using 'ln' instead of 'cp'.

set -vx
d=/srv/cache_for_lmi/perf_ln
mkdir -p $d
for z in \
  /usr/bin/perf_4.19                                   \
  /usr/lib/x86_64-linux-gnu/libopencsd_c_api.so.0      \
  /usr/lib/x86_64-linux-gnu/libopencsd_c_api.so.0.10.1 \
  /usr/lib/x86_64-linux-gnu/libopencsd.so.0            \
  /usr/lib/x86_64-linux-gnu/libopencsd.so.0.10.1       \
  /usr/lib/x86_64-linux-gnu/libperl.so.5.28            \
  /usr/lib/x86_64-linux-gnu/libperl.so.5.28.1          \
  /usr/lib/x86_64-linux-gnu/libpython3.7m.so.1.0       \
  /usr/lib/x86_64-linux-gnu/libbabeltrace-ctf.so.1     \
  /usr/lib/x86_64-linux-gnu/libbabeltrace-ctf.so.1.0.0 \
  /usr/lib/x86_64-linux-gnu/libbabeltrace.so.1         \
  /usr/lib/x86_64-linux-gnu/libbabeltrace.so.1.0.0     \
  ;
do ln $z $d/"$(basename "$z")"; done

# later, in a chroot that mounts /srv/cache_for_lmi/
# cd /opt/lmi/bin
#   library path required for perf's and lmi's binaries
# LD_LIBRARY_PATH=.:/opt/lmi/local/gcc_x86_64-pc-linux-gnu/lib/:/srv/cache_for_lmi/perf_ln /srv/cache_for_lmi/perf_ln/perf_4.19 record --freq=max --call-graph lbr ./lmi_cli_shared --accept --data_path=/opt/lmi/data --selftest
# Failed to open [ext4], continuing without symbols
# Failed to open [fscrypto], continuing without symbols
# Failed to open [kvm], continuing without symbols
# Failed to read max cpus, using default of 4096
# [ perf record: Captured and wrote 123.760 MB perf.data (358633 samples) ]
#
# LD_LIBRARY_PATH=.:/srv/cache_for_lmi/perf_ln /srv/cache_for_lmi/perf_ln/perf_4.19 report
