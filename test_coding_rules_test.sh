#!/bin/sh

# Test files for consistency with various rules: unit test.

# Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

# This script is intended to be invoked by lmi makefiles; it would
# rarely make sense to run it independently.

# '\' really is an intentional escape in many here-documents.
# shellcheck disable=SC1117

echo "Testing 'test_coding_rules'."

# Directory where this script resides.

srcdir=$(dirname "$(readlink --canonicalize "$0")")

# Cannot recursively check script on path determined at runtime, so
# a directive like 'source="$srcdir"' doesn't work.
# shellcheck disable=SC1090
. "$srcdir"/set_toolchain.sh

rm --force eraseme*

# Boilerplate required in most files.

good_copyright="...Copyright (C)...$(date -u +'%Y')..."

# Dollar signs must be escaped with '\' in here-documents; writing
# them on different lines prevents RCS substitution here.

good_url="...https://savannah.nongnu.org/projects/lmi..."

boilerplate=$(printf '%s\n%s' "$good_copyright" "$good_url")

# Files in general.

cat >eraseme_000 <<EOF
$boilerplate
EOF

touch eraseme_0_bytes.touchstone

printf '\n'   >eraseme_1_byte_good.touchstone
printf ' '    >eraseme_1_byte_bad.touchstone
printf 'z\n'  >eraseme_2_bytes_good.touchstone
printf '\\\n' >eraseme_2_bytes_bad.touchstone

# Files in general: copyright.

cat >eraseme_copyright_000 <<EOF
$boilerplate
"(C) 1999": upper-case 'C' is correct in ASCII copyright symbol.
EOF

cat >eraseme_copyright_001 <<EOF
$good_url
Copyright (C)
  is expected to be on the same line as
$(date -u +'%Y')
"(c) 1999": lower-case 'c' is incorrect in ASCII copyright symbol.
EOF

# html files: copyright.

cat >eraseme_copyright_002.html <<EOF
$boilerplate
Copyright &copy; $(date -u +'%Y')
EOF

cat >eraseme_copyright_003.html <<EOF
$good_url
Missing "(C)" copyright.
Copyright &copy; $(date -u +'%Y')
EOF

cat >eraseme_copyright_004.html <<EOF
$boilerplate
Missing circle-C copyright.
EOF

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
$good_copyright
<https://savannah.nongnu.org/projects/lmi>
EOF

cat >eraseme_url_001 <<EOF
$good_copyright
EOF

# Files in general: whitespace.

cat >eraseme_whitespace_000 <<EOF
$boilerplate
Spaces are permitted; they  can   be    consecutive.
EOF

ascii_ff=$(printf '\f')
cat >eraseme_whitespace_001 <<EOF
$boilerplate
$ascii_ff
EOF

ascii_cr=$(printf '\r')
cat >eraseme_whitespace_002 <<EOF
$boilerplate
$ascii_cr
EOF

ascii_ht=$(printf '\t')
cat >eraseme_whitespace_003 <<EOF
$boilerplate
$ascii_ht
EOF

ascii_vt=$(printf '\v')
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

cat >eraseme_cpp_003.cpp <<EOF
$boilerplate
'*' and '&' are part of the type, not the name: write
  foo& bar(); // bar() returns a 'reference to foo'
  int* x;     // x is a 'pointer to int'
not
  foo &bar(); // bar() is a 'reference function returning foo'?
  int *x;     // x is a 'pointer variable of type int'?
An apparent violation like
  some_type &uninitialized_reference; // Diagnosable error.
wouldn't be rejected because the program allows '&html_entity;',
but that's okay: the compiler would catch it.
EOF

cat >eraseme_cpp_004.cpp <<EOF
$boilerplate
Write a cv-qualifier after the type it modifies:
  void foo(T const&); // Yes.
  void foo(const T&); // No.
  void goo(std::string const&); // Yes.
  void goo(const std::string&); // No.
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

cat >eraseme_hpp_007.hpp <<EOF
$boilerplate
#ifndef eraseme_hpp_007_hpp
#define eraseme_hpp_007_hpp
#include "config.hpp"
Should be 'defined XYZ'.
#ifdef XYZ
Should be '!defined __cplusplus'.
#ifndef __cplusplus
#endif // eraseme_hpp_007_hpp
EOF

# Log files.

cat >eraseme_log_000.Log <<EOF
$boilerplate
'Log' is just a suffix, distinct from the extension '.Log'; the dot in
this file's name merely avoids "camel case", which the author dislikes.
This line's length is over the limit, but...acceptable in the preamble.
MAINTENANCE
After the "MAINTENANCE" line, a strict limit applies, but this line is
just short enough. However, certain literal quotations (e.g., snippets
of code in 'DefectLog') shouldn't be wrapped artificially, so any line
quoted with an initial '|', like this:
| std::string s("I never loved a dear Gazelle--nor anything that cost me much");
is permitted, as is any line that appears to be an optionally-indented
http URL, like this:
  http://www.gnu.org/server/standards/translations/priorities.html#Languages
.
EOF

cat >eraseme_log_001.Log <<EOF
$boilerplate
The word "MAINTENANCE" is expected on a line by itself, but this file has
no such line. The 'grep' regex is "^MAINTENANCE$", so these don't suffice:
Maintenance
'MAINTENANCE'
 MAINTENANCE
and each long line preceding them elicits a diagnostic.
EOF

cat >eraseme_log_002.Log <<EOF
$boilerplate
MAINTENANCE
This line's length is at the limit, so it should elicit no diagnostic.
This line's length is slightly over the limit, so it must be diagnosed.
It is helpful to diagnose all excessively-long lines--including this one.
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

# Check filename length, even for binary files.

cat >eraseme_png_003-dot.dash-dot.png <<EOF
     00000000011111111112222222222333
     12345678901234567890123456789012
This file's name is excessively long.
EOF

touch an_expungible_file.bak
touch an_unexpected_file
touch another.unexpected.file

# Compare observed to expected. Note that directory '.' is ignored.

2>&1 $PERFORM ./test_coding_rules \
  . \
  a_nonexistent_file \
  an_expungible_file.bak \
  an_unexpected_file \
  another.unexpected.file \
  eraseme* \
  | sed -e '/^[ 0-9]\{9\} \(source files\|source lines\|marked defects\)/d' \
  >observed_eraseme

cat >expected_eraseme <<EOF
Exception--file 'a_nonexistent_file': File not found.
File 'an_expungible_file.bak' ignored as being expungible.
Exception--file 'an_unexpected_file': File is unexpectedly uncategorizable.
Exception--file 'another.unexpected.file': File is unexpectedly uncategorizable.
Exception--file 'eraseme_1_byte_bad.touchstone': File does not end in newline.
Exception--file 'eraseme_2_bytes_bad.touchstone': File ends in backslash-newline.
File 'eraseme_copyright_001' lacks current copyright.
File 'eraseme_copyright_001' breaks taboo '\(c\) *[0-9]'.
File 'eraseme_copyright_003.html' lacks current copyright.
File 'eraseme_copyright_004.html' lacks current secondary copyright.
File 'eraseme_cpp_001.cpp' must not include 'config.hpp'.
File 'eraseme_cpp_002.cpp' has misindented label ' wrong   :'.
File 'eraseme_cpp_002.cpp' has misindented label '   No2   :'.
File 'eraseme_cpp_002.cpp' has misindented label '       x_:'.
File 'eraseme_cpp_003.cpp' should fuse '&' with type: 'foo &bar(); // bar() is a 'reference function returning foo'?'.
File 'eraseme_cpp_003.cpp' should fuse '*' with type: 'int *x;     // x is a 'pointer variable of type int'?'.
File 'eraseme_cpp_004.cpp' should write 'const' after the type it modifies: 'const T&'.
File 'eraseme_cpp_004.cpp' should write 'const' after the type it modifies: 'const std::string&'.
File 'eraseme_hpp_001.hpp' lacks end part of the canonical header guard.
File 'eraseme_hpp_002.hpp' lacks start part of the canonical header guard.
File 'eraseme_hpp_003.hpp' lacks start part of the canonical header guard.
File 'eraseme_hpp_004.hpp' lacks end part of the canonical header guard.
File 'eraseme_hpp_005.hpp' must include 'config.hpp' first.
File 'eraseme_hpp_006.hpp' must include 'config.hpp'.
File 'eraseme_hpp_006.hpp' lacks line '#include "config.hpp"'.
File 'eraseme_hpp_006.hpp' must include 'config.hpp' first.
File 'eraseme_hpp_007.hpp' should write '#if [!]defined' instead of '#if[n]def': '#ifdef XYZ'.
File 'eraseme_hpp_007.hpp' should write '#if [!]defined' instead of '#if[n]def': '#ifndef __cplusplus'.
File 'eraseme_log_001.Log' lacks expected 'MAINTENANCE' line.
File 'eraseme_log_001.Log' violates seventy-character limit:
0000000001111111111222222222233333333334444444444555555555566666666667
1234567890123456789012345678901234567890123456789012345678901234567890
The word "MAINTENANCE" is expected on a line by itself, but this file has
no such line. The 'grep' regex is "^MAINTENANCE$", so these don't suffice:
File 'eraseme_log_002.Log' violates seventy-character limit:
0000000001111111111222222222233333333334444444444555555555566666666667
1234567890123456789012345678901234567890123456789012345678901234567890
This line's length is slightly over the limit, so it must be diagnosed.
It is helpful to diagnose all excessively-long lines--including this one.
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
File 'eraseme_png_003-dot.dash-dot.png' exceeds 31-character file-name limit.
File 'eraseme_taboo_001' breaks taboo 'Cambridge'.
File 'eraseme_taboo_001' breaks taboo 'Temple P'.
File 'eraseme_taboo_001' breaks taboo 'Shibboleth'.
File 'eraseme_taboo_001' breaks taboo 'sibboleth'.
File 'eraseme_url_001' lacks lmi URL.
Exception--file 'eraseme_whitespace_001': File contains '\f'.
Exception--file 'eraseme_whitespace_002': File contains '\r'.
Exception--file 'eraseme_whitespace_003': File contains '\t'.
Exception--file 'eraseme_whitespace_004': File contains '\v'.
EOF

diff --unified=0 expected_eraseme observed_eraseme && rm --force \
  an_expungible_file.bak \
  an_unexpected_file \
  another.unexpected.file \
  eraseme* \
  ./*eraseme \

# This file does not end in backslash-newline.
