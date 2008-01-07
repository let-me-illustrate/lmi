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

# $Id: test_coding_rules_test.sh,v 1.28 2008-01-07 04:04:29 chicares Exp $

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

cat >eraseme_000 <<EOF
$boilerplate
EOF

# Files in general: copyright.

cat >eraseme_copyright_000 <<EOF
$boilerplate
"(C) 1999": upper-case 'C' is correct in ASCII copyright symbol.
EOF

cat >eraseme_copyright_001 <<EOF
$good_rcsid $good_url
Copyright (C)
  is expected to be on the same line as
`date -u +'%Y'`
"(c) 1999": lower-case 'c' is incorrect in ASCII copyright symbol.
EOF

cat >eraseme_copyright_002 <<EOF
$good_rcsid $good_url
This would seem to be all right:
// Copyright (C) 1900, `date -u +'%Y'`, 2100
but the datestamp is changed to the beginning of the msw epoch.
Don't use the unix epoch, because that causes mayhem on msw.
EOF
touch --date=19800102 eraseme_copyright_002

# Files in general: defect markers.

Q='?'

cat >eraseme_marker_000 <<EOF
$boilerplate
 TODO ${Q}${Q} Okay.
 INELEGANT !! Okay.
 number_of_valid_pointers += !!p; // This legitimate usage is allowed.
EOF

cat >eraseme_marker_001 <<EOF
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

cat >eraseme_rcs_id_000 <<EOF
$good_copyright $good_url
A real RCS Id must be accepted:
\$\
Id: test_coding_rules_test.sh,v 1.13 2007/12/29 14:16:18 chicares Exp \$
EOF

cat >eraseme_rcs_id_001 <<EOF
$good_copyright $good_url
\$\
Id\$ # Okay.
EOF

cat >eraseme_rcs_id_002 <<EOF
$good_copyright $good_url
\$\
Id # Lacks terminal dollar sign.
EOF

cat >eraseme_rcs_id_003 <<EOF
$good_copyright $good_url
\$\
Id
\$ # Terminal dollar sign must be on same line.
EOF

cat >eraseme_rcs_id_004 <<EOF
$good_copyright $good_url
# No RCS Id at all.
EOF

cat >eraseme_rcs_id_005 <<EOF
$good_copyright $good_url
\$\
Id gibberish\$ # Lacks colon.
EOF

cat >eraseme_rcs_id_006 <<EOF
$good_copyright $good_url
// \$\
Id: README.auto,v 1.6 2007/02/23 12:43:25 chicares Exp \$
Version: \$\
Id: README.auto,v 1.6 2007/02/23 12:43:25 chicares Exp \$
EOF

# Files in general: taboos.

cat >eraseme_taboo_000 <<EOF
$boilerplate
EOF

cat >eraseme_taboo_001 <<EOF
$boilerplate
675 Mass Ave, Cambridge, MA
59 Temple Place, Suite 330
Then said they unto him, Say now Shibboleth: and he said Sibboleth
EOF

# Files in general: lmi url.

cat >eraseme_url_000 <<EOF
$good_copyright $good_rcsid
<http://savannah.nongnu.org/projects/lmi>
EOF

cat >eraseme_url_001 <<EOF
$good_copyright $good_rcsid
EOF

# Files in general: whitespace.

cat >eraseme_whitespace_000 <<EOF
$boilerplate
Spaces are permitted; they  can   be    consecutive.
EOF

ascii_ff=$'\f'
cat >eraseme_whitespace_001 <<EOF
$boilerplate
$ascii_ff
EOF

ascii_cr=$'\r'
cat >eraseme_whitespace_002 <<EOF
$boilerplate
$ascii_cr
EOF

ascii_ht=$'\t'
cat >eraseme_whitespace_003 <<EOF
$boilerplate
$ascii_ht
EOF

ascii_vt=$'\v'
cat >eraseme_whitespace_004 <<EOF
$boilerplate
$ascii_vt
EOF

# C++ source files.

cat >eraseme_cpp_000.cpp <<EOF
$boilerplate
EOF

cat >eraseme_cpp_001.cpp <<EOF
$boilerplate
This header must be included only by other headers:
#include "config.hpp"
EOF

cat >eraseme_cpp_002.cpp <<EOF
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

cat >eraseme_hpp_000.hpp <<EOF
$boilerplate
#ifndef eraseme_hpp_000_hpp
#define eraseme_hpp_000_hpp
#include "config.hpp"
#endif // eraseme_hpp_000_hpp
EOF

cat >eraseme_hpp_001.hpp <<EOF
$boilerplate
#ifndef eraseme_hpp_001_hpp
#define eraseme_hpp_001_hpp
#include "config.hpp"
Name in comment is required to match:
#endif // eraseme_hpp_000_hpp
EOF

cat >eraseme_hpp_002.hpp <<EOF
$boilerplate
Stray comments on include guards are forbidden as meaningless.
#ifndef eraseme_hpp_002_hpp // Here's a guard to make the header idempotent.
#define eraseme_hpp_002_hpp // But such a comment is only distracting.
#include "config.hpp"
#endif // eraseme_hpp_002_hpp
EOF

cat >eraseme_hpp_003.hpp <<EOF
$boilerplate
Include guards must occur in correct order.
#define eraseme_hpp_003_hpp
#ifndef eraseme_hpp_003_hpp
#include "config.hpp"
#endif // eraseme_hpp_003_hpp
EOF

cat >eraseme_hpp_004.hpp <<EOF
$boilerplate
#ifndef eraseme_hpp_004_hpp
#define eraseme_hpp_004_hpp
#include "config.hpp"
#endif // eraseme_hpp_004_hpp
Nothing but newline is permitted after the closing include guard.
EOF

cat >eraseme_hpp_005.hpp <<EOF
$boilerplate
#ifndef eraseme_hpp_005_hpp
#define eraseme_hpp_005_hpp
This compulsory include directive must occur, in canonical form (with
only one space, preceding '"'), before any other include directive.
#   include "config.hpp"
#include "config.hpp" // Stray comments forbidden.
#include "config.hpp"
#endif // eraseme_hpp_005_hpp
EOF

cat >eraseme_hpp_006.hpp <<EOF
$boilerplate
#ifndef eraseme_hpp_006_hpp
#define eraseme_hpp_006_hpp
Missing compulsory include directive.
#endif // eraseme_hpp_006_hpp
EOF

# Makefiles.

cat >eraseme_make_000.make <<EOF
${ascii_ht}
$boilerplate
A line's initial tab can be followed by any other character.
${ascii_ht}This is okay.
${ascii_ht}
EOF

cat >eraseme_make_001.make <<EOF
$boilerplate
Consecutive tabs are forbidden.
${ascii_ht}${ascii_ht}
EOF

cat >eraseme_make_002.make <<EOF
$boilerplate
Tab can occur only at the beginning of a line.
 ${ascii_ht}
EOF

# X pixmaps.

cat >eraseme_xpm_000.xpm <<EOF
No boilerplate is required.
static char const* eraseme_xpm_000_xpm[] = {
EOF

cat >eraseme_xpm_001.xpm <<EOF
'const' is required.
static char* eraseme_xpm_001_xpm[] = {
EOF

cat >eraseme_xpm_002.xpm <<EOF
Variable name must be file name with '.xpm' changed to '_xpm'.
static char const* wrong_file_002_xpm[] = {
EOF

cat >eraseme_xpm_003-dot.dash-dot.xpm <<EOF
Hyphens in file name must be changed to underscores in variable name.
static char const* eraseme_xpm_003_dot_dash_dot_xpm[] = {
EOF

# 'touch' should suffice, but empty files trigger an exception.
echo " " > an_expungible_file.bak

# Compare observed to expected. Note that directory '.' is ignored.

./test_coding_rules \
  . \
  a_nonexistent_file \
  an_expungible_file.bak \
  eraseme* \
  >eraseme_observed 2>&1

cat >eraseme_expected <<EOF
Exception--file 'a_nonexistent_file': File not found.
File 'an_expungible_file.bak' ignored as being expungible.
File 'eraseme_copyright_001' lacks current copyright.
File 'eraseme_copyright_001' breaks taboo '\(c\) *[0-9]'.
File 'eraseme_copyright_002' lacks current copyright.
File 'eraseme_cpp_001.cpp' must not include 'config.hpp'.
File 'eraseme_cpp_002.cpp' has misindented label ' wrong   :'.
File 'eraseme_cpp_002.cpp' has misindented label '   No2   :'.
File 'eraseme_cpp_002.cpp' has misindented label '       x_:'.
File 'eraseme_hpp_001.hpp' lacks canonical header guards.
File 'eraseme_hpp_002.hpp' lacks canonical header guards.
File 'eraseme_hpp_003.hpp' lacks canonical header guards.
File 'eraseme_hpp_004.hpp' lacks canonical header guards.
File 'eraseme_hpp_005.hpp' must include 'config.hpp' first.
File 'eraseme_hpp_006.hpp' must include 'config.hpp'.
File 'eraseme_hpp_006.hpp' lacks line '#include "config.hpp"'.
File 'eraseme_hpp_006.hpp' must include 'config.hpp' first.
Exception--file 'eraseme_make_001.make': File contains postinitial '\t'.
Exception--file 'eraseme_make_002.make': File contains postinitial '\t'.
File 'eraseme_marker_001' has irregular defect marker 'TODO${Q}${Q} '.
File 'eraseme_marker_001' has irregular defect marker 'TODO ${Q}${Q}B'.
File 'eraseme_marker_001' has irregular defect marker 'TODO ${Q}${Q}${Q} '.
File 'eraseme_marker_001' has irregular defect marker 'tODO ${Q}${Q} '.
File 'eraseme_marker_001' has irregular defect marker 'ODO ${Q}${Q} '.
File 'eraseme_marker_001' has irregular defect marker 'INELEGANT!! '.
File 'eraseme_marker_001' has irregular defect marker 'INELEGANT !!B'.
File 'eraseme_marker_001' has irregular defect marker 'INELEGANT !!!'.
File 'eraseme_marker_001' has irregular defect marker 'ELEGANT !! '.
File 'eraseme_rcs_id_002' lacks a well-formed RCS Id.
File 'eraseme_rcs_id_002' contains a malformed RCS Id.
File 'eraseme_rcs_id_003' lacks a well-formed RCS Id.
File 'eraseme_rcs_id_003' contains a malformed RCS Id.
File 'eraseme_rcs_id_004' lacks a well-formed RCS Id.
File 'eraseme_rcs_id_005' lacks a well-formed RCS Id.
File 'eraseme_rcs_id_006' contains more than one RCS Id.
File 'eraseme_taboo_001' breaks taboo 'Cambridge'.
File 'eraseme_taboo_001' breaks taboo 'Temple'.
File 'eraseme_taboo_001' breaks taboo 'Shibboleth'.
File 'eraseme_taboo_001' breaks taboo 'sibboleth'.
File 'eraseme_url_001' lacks lmi URL.
Exception--file 'eraseme_whitespace_001': File contains '\f'.
Exception--file 'eraseme_whitespace_002': File contains '\r' or '\v'.
Exception--file 'eraseme_whitespace_003': File contains '\t'.
Exception--file 'eraseme_whitespace_004': File contains '\r' or '\v'.
File 'eraseme_xpm_001.xpm' lacks proper variable assignment.
File 'eraseme_xpm_002.xpm' lacks proper variable assignment.
EOF

diff --unified=0 eraseme_expected eraseme_observed && rm --force eraseme* *.bak

