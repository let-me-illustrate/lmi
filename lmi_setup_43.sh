#!/bin/sh

# Create a chroot for cross-building "Let me illustrate...".
#
# Copyright (C) 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

# Symlink the repository's hooks/ directory:
cd /opt/lmi/src/lmi || { printf 'failed: cd\n'; exit 3; }
mv .git/hooks .git/hooks-orig
ln --symbolic --force --no-dereference ../hooks .git

# Iff this chroot needs write access to savannah, then reconfigure
# the URL, using your savannah ID instead of mine:
if [ "greg" = "$(whoami)" ]; then
  git remote set-url --push origin chicares@git.sv.gnu.org:/srv/git/lmi.git
fi

# Clone the bare proprietary repository to create a working copy.
#
# Apparently '--config core.SharedRepository=group' would have little
# or no benefit here.
cd /opt/lmi || { printf 'failed: cd\n'; exit 3; }
git clone -b master file:///srv/cache_for_lmi/blessed/proprietary
#
# Fix ownership and permissions of working copy.
chgrp -R "$NORMAL_GROUP" proprietary
# This is better than 'chmod -R g+s' (it affects only directories):
find proprietary -type d -exec chmod g+s {} +
# Specifying 's' here would cause many 'S' occurrences in 'ls' output;
# specifying 'g+w' here would cause pack files to be group writable:
#   chmod -R g+swX proprietary
# Instead, use 'g=u', which doesn't override the earlier 'g+s'--see:
#   https://lists.nongnu.org/archive/html/lmi/2020-03/msg00019.html
chmod -R g=u proprietary
#
# ...and verify it:
cd proprietary || { printf 'failed: cd\n'; exit 3; }
git rev-parse HEAD
# ...then symlink its hooks/ directory:
mv .git/hooks .git/hooks-orig
ln --symbolic --force --no-dereference ../hooks .git

# Create a "stash" directory as a manual alternative to 'git-stash':
mkdir --parents /opt/lmi/stash
chmod g=u+s     /opt/lmi/stash

# Create and populate the proprietary source directory used by 'vpath':
mkdir --parents /opt/lmi/src/products/src
chmod g=u+s     /opt/lmi/src/products/src
cp -a /opt/lmi/proprietary/src /opt/lmi/src/products
# ...and the directories for system testing:
cp -a /opt/lmi/proprietary/test /opt/lmi
mkdir --parents /opt/lmi/touchstone
chmod g=u+s     /opt/lmi/touchstone
cp -a /opt/lmi/proprietary/test/* /opt/lmi/touchstone/

# Remove object files previously built without proprietary source:
# shellcheck disable=SC1091
. /opt/lmi/src/lmi/set_toolchain.sh
rm /opt/lmi/"${LMI_COMPILER}_${LMI_TRIPLET}"/build/ship/my*

# Regenerate the binary database (expect the 'rm' command here to fail
# the first time, because there are no old files to remove):
cd /opt/lmi/data || { printf 'failed: cd\n'; exit 3; }
rm --force proprietary.dat proprietary.ndx
wine /opt/lmi/bin/rate_table_tool.exe --accept --file=proprietary --merge=/opt/lmi/proprietary/tables

coefficiency="--jobs=$(nproc)"

# Run a system test.
cd /opt/lmi/src/lmi || { printf 'failed: cd\n'; exit 3; }
make "$coefficiency" system_test 2>&1 |sed -e'/^Cannot open.*test/d'
# That test fails the first time because no results are saved in
# touchstone/ yet. Remove timestamped summaries...
rm --force /opt/lmi/test/analysis-* /opt/lmi/test/diffs-* /opt/lmi/test/md5sums-*
# ...copy the results just generated...
cp -a /opt/lmi/test/* /opt/lmi/touchstone
# ...discarding unwanted files...
rm --force /opt/lmi/touchstone/regressions.tsv /opt/lmi/touchstone/trace.txt
# ...and rerun the test, which should now succeed:
make "$coefficiency" system_test

# Create a local mirror of the gnu.org repository:
cd /opt/lmi || { printf 'failed: cd\n'; exit 3; }
mkdir --parents free/src
chmod g=u+s     free/src
cd free/src || { printf 'failed: cd\n'; exit 3; }

# Use git's own protocol wherever possible. In case that's blocked
# by a corporate firewall, fall back on https. In case a firewall
# inexplicably blocks the gnu.org domain, try Vadim's github clone
# as a last resort. Any messages like
#   fatal: read error: Connection timed out
# can be ignored as long as one of the subsequent git-clone commands
# succeeds.

git clone git://git.savannah.nongnu.org/lmi.git \
  || git clone https://git.savannah.nongnu.org/r/lmi.git \
  || git clone https://github.com/let-me-illustrate/lmi.git

if [ "greg" = "$(whoami)" ]; then
git remote add xanadu     https://github.com/vadz/lmi.git   || echo "Oops."
git remote add shangri-la https://github.com/thesiv/lmi.git || echo "Oops."
fi

cd lmi || { printf 'failed: cd\n'; exit 3; }
find . -path ./.git -prune -o -type f -print0 \
  | xargs --null --max-procs="$(nproc)" --replace='{}' touch '--reference=/opt/lmi/src/lmi/{}' '{}'

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0: Ran system test for '$(whoami)'."  | tee /dev/tty
