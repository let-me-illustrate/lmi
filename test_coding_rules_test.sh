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

# $Id: test_coding_rules_test.sh,v 1.4 2007-12-16 01:24:29 chicares Exp $

echo "Testing 'test_coding_rules'."

rm --force eraseme*

cat >eraseme000 <<EOF
_Copyright_`date -u +'%Y'`_
EOF

cat >eraseme001 <<EOF
Copyright
  is expected to be on the same line as
`date -u +'%Y'`
EOF

cat >eraseme002 <<EOF
// Copyright 1900, `date -u +'%Y'`, 2100
EOF
touch --date=19700101 eraseme002

./test_coding_rules . a_nonexistent_file eraseme* >eraseme_observed 2>&1

cat >eraseme_expected <<'EOF'
Exception--file '.': Argument is a directory.
Exception--file 'a_nonexistent_file': File not found.
File 'eraseme001' lacks current copyright.
File 'eraseme002' lacks current copyright.
EOF

diff --unified=0 eraseme_observed eraseme_expected && rm --force eraseme*

