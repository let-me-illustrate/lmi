#!/bin/zsh

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

# Symlink the repository's hooks/ directory:
cd /opt/lmi/src/lmi
mv .git/hooks .git/hooks-orig
ln --symbolic --force --no-dereference ../hooks .git

# Iff this chroot needs write access to savannah, then reconfigure
# the URL, using your savannah ID instead of mine:
git remote set-url --push origin chicares@git.sv.gnu.org:/srv/git/lmi.git

# Duplicate proprietary repository (if available).
# First, copy "blessed" repository (here, 'cp' is sufficient: this
# bare repository has no references that need to be resolved):
cd /opt/lmi
cp --dereference --preserve --recursive /cache_for_lmi/blessed .
# Then create a working copy by cloning the bare repository...
git clone -b master file:///opt/lmi/blessed/proprietary
# ...and verify it:
cd proprietary
git rev-parse HEAD
# ...then symlink its hooks/ directory:
mv .git/hooks .git/hooks-orig
ln --symbolic --force --no-dereference ../hooks .git

# Create and populate the proprietary source directory used by 'vpath':
mkdir --parents /opt/lmi/src/products/src
cp -a /opt/lmi/proprietary/src /opt/lmi/src/products
# ...and the directories for system testing:
cp -a /opt/lmi/proprietary/test /opt/lmi
mkdir --parents /opt/lmi/touchstone
cp -a /opt/lmi/proprietary/test/* /opt/lmi/touchstone/

# Remove object files previously built without proprietary source:
rm /opt/lmi/src/build/lmi/Linux/gcc/ship/my*

# Regenerate the binary database (expect the 'rm' command here to fail
# the first time, because there are no old files to remove):
cd /opt/lmi/data
rm proprietary.dat proprietary.ndx
wine /opt/lmi/bin/rate_table_tool --accept --file=proprietary --merge=/opt/lmi/proprietary/tables

# Run a system test.
cd /opt/lmi/src/lmi
make $coefficiency system_test 2>&1 |less -S
# That test fails the first time because no results are saved in
# touchstone/ yet. Copy the results just generated...
cp -a /opt/lmi/test/* /opt/lmi/touchstone
# ...removing summaries...
rm /opt/lmi/touchstone/analysis* /opt/lmi/touchstone/diffs* /opt/lmi/touchstone/md5sum*
# ...and rerun the test, which should now succeed:
make $coefficiency system_test 2>&1 |less -S

# Create a local mirror of the gnu.org repository:
cd /opt/lmi
mkdir --parents free/src
cd free/src
git clone git://git.savannah.nongnu.org/lmi.git
