#!/bin/sh

# Test files for consistency with various rules: unit test.

# Copyright (C) 2007 Gregory W. Chicares.
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
# email: <chicares@cox.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

# $Id: test_coding_rules_test.sh,v 1.6 2007-12-19 15:26:11 chicares Exp $

echo "Testing 'test_coding_rules'."

rm --force eraseme*

# Files in general.

cat >eraseme000 <<EOF
_Copyright_`date -u +'%Y'`_
EOF

cat >eraseme001 <<EOF
Copyright
  is expected to be on the same line as
`date -u +'%Y'`
EOF

cat >eraseme002 <<EOF
This would seem to be all right:
// Copyright 1900, `date -u +'%Y'`, 2100
but the datestamp is changed to the beginning of the msw epoch.
Don't use the unix epoch, because that causes mayhem on msw.
EOF
touch --date=19800102 eraseme002

# Headers.

cat >eraseme000.hpp <<EOF
_Copyright_`date -u +'%Y'`_
#ifndef eraseme000_hpp
#define eraseme000_hpp
#include "config.hpp"
#endif // eraseme000_hpp
EOF

cat >eraseme001.hpp <<EOF
_Copyright_`date -u +'%Y'`_
#ifndef eraseme001_hpp
#define eraseme001_hpp
#include "config.hpp"
Name in comment is required to match:
#endif // eraseme000_hpp
EOF

cat >eraseme002.hpp <<EOF
_Copyright_`date -u +'%Y'`_
Stray comments on include guards are forbidden as meaningless.
#ifndef eraseme002_hpp // Here's a guard to make the header idempotent.
#define eraseme002_hpp // But such a comment is only distracting.
#include "config.hpp"
#endif // eraseme002_hpp
EOF

# X pixmaps require no copyright, but do require 'const'.
# SOMEDAY !! Require internal name to match file name?

cat >eraseme000.xpm <<EOF
static char const* eraseme000_xpm[]
EOF

cat >eraseme001.xpm <<EOF
static char* eraseme001_xpm[]
EOF

# Compare observed to expected.

./test_coding_rules . a_nonexistent_file eraseme* >eraseme_observed 2>&1

# This line
#   File 'eraseme000.xpm' lacks /^static char const\*/.
# works around a temporary defect.

cat >eraseme_expected <<'EOF'
Exception--file '.': Argument is a directory.
Exception--file 'a_nonexistent_file': File not found.
File 'eraseme000.xpm' lacks /^static char const\*/.
File 'eraseme001' lacks current copyright.
File 'eraseme001.hpp' has noncanonical header guards.
File 'eraseme001.xpm' lacks /^static char const\*/.
File 'eraseme002' lacks current copyright.
File 'eraseme002.hpp' has noncanonical header guards.
EOF

diff --unified=0 eraseme_observed eraseme_expected && rm --force eraseme*

