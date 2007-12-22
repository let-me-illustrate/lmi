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

# $Id: test_coding_rules_test.sh,v 1.9 2007-12-22 05:23:22 chicares Exp $

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

# C++ source files.

cat >eraseme000.cpp <<EOF
_Copyright_`date -u +'%Y'`_
EOF

cat >eraseme001.cpp <<EOF
_Copyright_`date -u +'%Y'`_
This header must be included only by other headers:
#include "config.hpp"
EOF

cat >eraseme002.cpp <<EOF
_Copyright_`date -u +'%Y'`_
Labels must be indented either two spaces:
  l0:
  LABEL_1 :
or six--an extra four to accommodate indented namespaces:
      label:
      L    :
but no other indentation is allowed:
 wrong   :
   No2   :
       x_:
except for 'default', whose indentation is uncheck because it should
be aligned with its corresponding case labels:
     default   :
       default :
EOF

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

cat >eraseme003.hpp <<EOF
_Copyright_`date -u +'%Y'`_
#ifndef eraseme003_hpp
#define eraseme003_hpp
This compulsory include directive must occur, in canonical form (with
only one space, preceding '"'), before any other include directive.
#   include "config.hpp"
#include "config.hpp" // Stray comments forbidden.
#include "config.hpp"
#endif // eraseme003_hpp
EOF

cat >eraseme004.hpp <<EOF
_Copyright_`date -u +'%Y'`_
#ifndef eraseme004_hpp
#define eraseme004_hpp
Missing compulsory include directive.
#endif // eraseme004_hpp
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

cat >eraseme_expected <<'EOF'
Exception--file '.': Argument is a directory.
Exception--file 'a_nonexistent_file': File not found.
File 'eraseme001' lacks current copyright.
File 'eraseme001.cpp' must not include 'config.hpp'.
File 'eraseme001.hpp' has noncanonical header guards.
File 'eraseme001.xpm' lacks /^static char const\*/.
File 'eraseme002' lacks current copyright.
File 'eraseme002.cpp' has misindented label ' wrong   :'.
File 'eraseme002.cpp' has misindented label '   No2   :'.
File 'eraseme002.cpp' has misindented label '       x_:'.
File 'eraseme002.hpp' has noncanonical header guards.
File 'eraseme003.hpp' must include 'config.hpp' first.
File 'eraseme004.hpp' must include 'config.hpp'.
File 'eraseme004.hpp' lacks line '#include "config.hpp"'.
File 'eraseme004.hpp' must include 'config.hpp' first.
EOF

diff --unified=0 eraseme_expected eraseme_observed && rm --force eraseme*

