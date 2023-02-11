#!/bin/sh

# Upload a file.

# Copyright (C) 2022, 2023 Gregory W. Chicares.
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

set -x

if [ "greg" != "$(whoami)" ]; then
  exit 0
fi

if [ "$#" -ne 1 ]; then
  printf '%s arguments given, but exactly one expected.\n' "$#"
  exit 2
fi

filepath="$(readlink --canonicalize "$1")"

if [ ! -f "$filepath" ]; then
  printf 'No such file "%s"--exiting.\n' "$1"
  exit 3
fi

gpg --local-user gchicares@sbcglobal.net --detach-sign "$filepath"
chmod 644 "$filepath" "$filepath.sig"
cd "$(dirname "$filepath")" || { printf 'fail: cd\n'; exit 4; }
rsync -vv \
  "$(basename "$filepath")" \
  "$(basename "$filepath.sig")" \
  chicares@dl.sv.nongnu.org:/releases/lmi/
