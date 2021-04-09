#!/bin/sh

# Hard-link host's static-built 'perf' for use in a chroot.

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

# Instead of hard-linking 'perf' and all its dependencies, build it
# statically as suggested here:
#   https://lists.nongnu.org/archive/html/lmi/2020-10/msg00020.html
# Unfortunately, while the resulting static 'perf' seems to work just
# fine for 'perf record', for 'perf report' it doesn't decode symbols
# or present the expected interactive interface--see example below.

set -vx

# apt-get install linux-source
# pushd /usr/src
#   redirect because output is about 64K lines
#   in an interactive session, press <Tab> where indicated:
# tar xvf /usr/src/linux-source-*<Tab> 2>&1 |less
#   here, the tab completion is inlined:
# tar xvf /usr/src/linux-source-4.19.tar.xz  2>&1 |less
# pushd   /usr/src/linux-source-4.19/tools/perf
#   this fails...
# make clean; make EXTRA_LDFLAGS=-static 2>&1 |less
#   ...so do this instead--see:
#   https://lists.nongnu.org/archive/html/lmi/2020-12/msg00003.html
# make clean; make EXTRA_LDFLAGS=-static EXTRA_CFLAGS=-Wno-discarded-qualifiers 2>&1 |less
#   still, it's not purely static:
# ldd /usr/src/linux-source-4.19/tools/perf/perf
#   linux-vdso.so.1 (0x00007ffff5d2c000)
#   libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007fd5341b6000)
#   librt.so.1 => /lib/x86_64-linux-gnu/librt.so.1 (0x00007fd5341ac000)
#   libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007fd534029000)
#   libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007fd534024000)
#   libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007fd533e63000)
#   /lib64/ld-linux-x86-64.so.2 (0x00007fd534727000)
# ...but perhaps that doesn't actually matter.

# For this static build (cf. 'allow_perf.sh'):
# chgrp perf_users /usr/src/linux-source-4.19/tools/perf/perf
# chmod o-rwx /usr/src/linux-source-4.19/tools/perf/perf

d=/srv/cache_for_lmi/perf_static
mkdir -p $d
ln /usr/src/linux-source-4.19/tools/perf/perf $d/perf

# later, in a chroot that mounts /srv/cache_for_lmi/
# cd /opt/lmi/bin
#   library path required only for lmi's libraries, not perf's
# LD_LIBRARY_PATH=.:/opt/lmi/local/gcc_x86_64-pc-linux-gnu/lib/ /srv/cache_for_lmi/perf_static/perf record --freq=max --call-graph lbr ./lmi_cli_shared --accept --data_path=/opt/lmi/data --selftest
# Failed to open [ext4], continuing without symbols
# Failed to open [kvm], continuing without symbols
# Failed to open [fscrypto], continuing without symbols
# Failed to read max cpus, using default of 4096
# [ perf record: Captured and wrote 126.807 MB perf.data (367155 samples) ]
#
# specifying $LD_LIBRARY_PATH here neither hurts nor helps:
# LD_LIBRARY_PATH=.:/srv/cache_for_lmi/perf_static /srv/cache_for_lmi/perf_static/perf report
#
#    34.35%     0.00%  lmi_cli_shared  liblmi.so              [.] 0x00007f58de18d8e9
#            |
#            ---0x7f58de18d8e9
#               |
#               |--28.65%--0x7f58de222bce
#
# No runtime error is reported, but the customary text-mode interface
# is not offered. This static 'perf' generates a valid 'perf.data'
# output file, which can be viewed with the 'perf_static.sh' variant
# of perf in the chroot (which then does present the customary text-
# mode interface and does decode symbols).
#
# The usual advice is to make sure the program being profiled was
# built with debugging enabled and with '-fno-omit-frame-pointer', but
# that doesn't help here: the program was built with those options,
# and the 'perf_ln.sh' technique works but this 'perf_static.sh'
# technique leads to the problems above.
