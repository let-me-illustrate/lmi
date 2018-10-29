#!/bin/zsh

# Copy MST files, obfuscating them for distribution.

# Copyright (C) 2018 Gregory W. Chicares.
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

# MST files are so easy to edit that end users might be tempted to
# customize them, which might impair regulatory compliance. That
# temptation is removed by distributing only obfuscated copies.
#
# Mapping 255..0 to 0..255 is XORing with 0xFF.
#
# Stripping the copyright header before obfuscation is a possible
# future enhancement that might improve runtime performance.

# This script requires zsh because brace expansion is more convenient
# than 'seq'. It is intended to be called from a makefile that sets
# both $datadir and $srcdir.

[ -d "$datadir" ] || { printf 'fail: invalid datadir\n'; exit 2; }
[ -d "$srcdir"  ] || { printf 'fail: invalid srcdir\n';  exit 3; }

X=$(printf '\%03o' {255..0})
Y=$(printf '\%03o' {0..255})

cd "$srcdir" || { printf 'fail: cd\n'; exit 4; }
for z in *.mst; do tr "$X" "$Y" <"$z" >"$datadir/${z%%.mst}.xst"; done
