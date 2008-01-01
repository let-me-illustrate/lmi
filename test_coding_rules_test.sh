#!/bin/sh

# Test files for consistency with various rules: unit test.

# Copyright (C) 2007, 2008 Gregory W. Chicares.
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

# $Id: test_coding_rules_test.sh,v 1.22 2008-01-01 18:29:56 chicares Exp $

echo "Testing 'test_coding_rules'."

rm --force eraseme*

# Boilerplate required in most files.

good_copyright="_Copyright (C)_`date -u +'%Y'`_"

# Dollar signs must be escaped with '\' in here-documents; writing
# them on different lines prevents RCS substitution on cvs checkin.

good_rcsid="_\$\
Id:_\$_"

good_url="http://savannah.nongnu.org/projects/lmi"

boilerplate="$good_copyright $good_rcsid $good_url"

# Files in general.

cat >eraseme000 <<EOF
$boilerplate
EOF

# Files in general: copyright.

cat >eraseme_copyright000 <<EOF
$boilerplate
"(C) 1999": upper-case 'C' is correct in ASCII copyright symbol.
EOF

cat >eraseme_copyright001 <<EOF
$good_rcsid $good_url
Copyright (C)
  is expected to be on the same line as
`date -u +'%Y'`
"(c) 1999": lower-case 'c' is incorrect in ASCII copyright symbol.
EOF

cat >eraseme_copyright002 <<EOF
$good_rcsid $good_url
This would seem to be all right:
// Copyright (C) 1900, `date -u +'%Y'`, 2100
but the datestamp is changed to the beginning of the msw epoch.
Don't use the unix epoch, because that causes mayhem on msw.
EOF
touch --date=19800102 eraseme_copyright002

# Files in general: defect markers.

Q='?'

cat >eraseme_marker000 <<EOF
$boilerplate
 TODO ${Q}${Q} Okay.
 INELEGANT !! Okay.
 number_of_valid_pointers += !!p; // This legitimate usage is allowed.
EOF

cat >eraseme_marker001 <<EOF
$boilerplate
 TODO${Q}${Q} Bad spacing.
 TODO ${Q}${Q}Bad spacing.
 TODO ${Q}${Q}${Q} Tripled.
 tODO ${Q}${Q} Wrong case.
 ODO ${Q}${Q} Truncated.
 INELEGANT!! Bad spacing.
 INELEGANT !!Bad spacing.
 INELEGANT !!! Tripled.
 ELEGANT !! No such marker.
EOF

# Files in general: RCS Ids.

cat >eraseme_rcs_id000 <<EOF
$good_copyright $good_url
A real RCS Id must be accepted:
\$\
Id: test_coding_rules_test.sh,v 1.13 2007/12/29 14:16:18 chicares Exp \$
EOF

cat >eraseme_rcs_id001 <<EOF
$good_copyright $good_url
\$\
Id\$ # Okay.
EOF

cat >eraseme_rcs_id002 <<EOF
$good_copyright $good_url
\$\
Id # Lacks terminal dollar sign.
EOF

cat >eraseme_rcs_id003 <<EOF
$good_copyright $good_url
\$\
Id
\$ # Terminal dollar sign must be on same line.
EOF

cat >eraseme_rcs_id004 <<EOF
$good_copyright $good_url
# No RCS Id at all.
EOF

cat >eraseme_rcs_id005 <<EOF
$good_copyright $good_url
\$\
Id gibberish\$ # Lacks colon.
EOF

cat >eraseme_rcs_id006 <<EOF
$good_copyright $good_url
// \$\
Id: README.auto,v 1.6 2007/02/23 12:43:25 chicares Exp \$
Version: \$\
Id: README.auto,v 1.6 2007/02/23 12:43:25 chicares Exp \$
EOF

# Files in general: taboos.

cat >eraseme_taboo000 <<EOF
$boilerplate
EOF

cat >eraseme_taboo001 <<EOF
$boilerplate
675 Mass Ave, Cambridge, MA
59 Temple Place, Suite 330
Then said they unto him, Say now Shibboleth: and he said Sibboleth
EOF

# Files in general: lmi url.

cat >eraseme_url000 <<EOF
$good_copyright $good_rcsid
<http://savannah.nongnu.org/projects/lmi>
EOF

cat >eraseme_url001 <<EOF
$good_copyright $good_rcsid
EOF

# C++ source files.

cat >eraseme000.cpp <<EOF
$boilerplate
EOF

cat >eraseme001.cpp <<EOF
$boilerplate
This header must be included only by other headers:
#include "config.hpp"
EOF

cat >eraseme002.cpp <<EOF
$boilerplate
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
except for 'default', whose indentation is unchecked because it should
be aligned with its corresponding case labels:
     default   :
       default :
EOF

# Headers.

cat >eraseme000.hpp <<EOF
$boilerplate
#ifndef eraseme000_hpp
#define eraseme000_hpp
#include "config.hpp"
#endif // eraseme000_hpp
EOF

cat >eraseme001.hpp <<EOF
$boilerplate
#ifndef eraseme001_hpp
#define eraseme001_hpp
#include "config.hpp"
Name in comment is required to match:
#endif // eraseme000_hpp
EOF

cat >eraseme002.hpp <<EOF
$boilerplate
Stray comments on include guards are forbidden as meaningless.
#ifndef eraseme002_hpp // Here's a guard to make the header idempotent.
#define eraseme002_hpp // But such a comment is only distracting.
#include "config.hpp"
#endif // eraseme002_hpp
EOF

cat >eraseme003.hpp <<EOF
$boilerplate
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
$boilerplate
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

cat >eraseme_expected <<EOF
Exception--file '.': Argument is a directory.
Exception--file 'a_nonexistent_file': File not found.
File 'eraseme001.cpp' must not include 'config.hpp'.
File 'eraseme001.hpp' has noncanonical header guards.
File 'eraseme001.xpm' lacks /^static char const\*/.
File 'eraseme002.cpp' has misindented label ' wrong   :'.
File 'eraseme002.cpp' has misindented label '   No2   :'.
File 'eraseme002.cpp' has misindented label '       x_:'.
File 'eraseme002.hpp' has noncanonical header guards.
File 'eraseme003.hpp' must include 'config.hpp' first.
File 'eraseme004.hpp' must include 'config.hpp'.
File 'eraseme004.hpp' lacks line '#include "config.hpp"'.
File 'eraseme004.hpp' must include 'config.hpp' first.
File 'eraseme_copyright001' lacks current copyright.
File 'eraseme_copyright001' breaks taboo '\(c\) *[0-9]'.
File 'eraseme_copyright002' lacks current copyright.
File 'eraseme_marker001' has irregular defect marker 'TODO${Q}${Q} '.
File 'eraseme_marker001' has irregular defect marker 'TODO ${Q}${Q}B'.
File 'eraseme_marker001' has irregular defect marker 'TODO ${Q}${Q}${Q} '.
File 'eraseme_marker001' has irregular defect marker 'tODO ${Q}${Q} '.
File 'eraseme_marker001' has irregular defect marker 'ODO ${Q}${Q} '.
File 'eraseme_marker001' has irregular defect marker 'INELEGANT!! '.
File 'eraseme_marker001' has irregular defect marker 'INELEGANT !!B'.
File 'eraseme_marker001' has irregular defect marker 'INELEGANT !!!'.
File 'eraseme_marker001' has irregular defect marker 'ELEGANT !! '.
File 'eraseme_rcs_id002' lacks a well-formed RCS Id.
File 'eraseme_rcs_id002' contains a malformed RCS Id.
File 'eraseme_rcs_id003' lacks a well-formed RCS Id.
File 'eraseme_rcs_id003' contains a malformed RCS Id.
File 'eraseme_rcs_id004' lacks a well-formed RCS Id.
File 'eraseme_rcs_id005' lacks a well-formed RCS Id.
File 'eraseme_rcs_id006' contains more than one RCS Id.
File 'eraseme_taboo001' breaks taboo 'Cambridge'.
File 'eraseme_taboo001' breaks taboo 'Temple'.
File 'eraseme_taboo001' breaks taboo 'Shibboleth'.
File 'eraseme_taboo001' breaks taboo 'sibboleth'.
File 'eraseme_url001' lacks lmi URL.
EOF

diff --unified=0 eraseme_expected eraseme_observed && rm --force eraseme*

