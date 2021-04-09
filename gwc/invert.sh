#!/bin/zsh

# Obfuscate-deobfuscate MST<->XST files. Cf. 'mst_to_xst.sh'.

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
# https://savannah.nongnu.org/projects/lmi
# email: <gchicares@sbcglobal.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

# MST files are so easy to edit that end users might be tempted to
# customize them, which might impair regulatory compliance. That
# temptation is removed by distributing only obfuscated copies.
# This script, for maintainer use only, deobfuscates and reobfuscates
# them in place.
#
# Mapping 255..0 to 0..255 is XORing with 0xFF.

# This script requires zsh because brace expansion is more convenient
# than 'seq', and especially because zsh offers glob qualifiers.
# shellcheck disable=SC1036,SC2039

X=$(printf '\%03o' {255..0})
Y=$(printf '\%03o' {0..255})

MST=(*.mst(N))
XST=(*.xst(N))

# shellcheck disable=SC2128
for z in $MST; do tr "$X" "$Y" <"$z" >"${z%%.mst}.xst"; done
# shellcheck disable=SC2128
for z in $XST; do tr "$X" "$Y" <"$z" >"${z%%.xst}.mst"; done
