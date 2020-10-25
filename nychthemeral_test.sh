#!/bin/zsh

# Run a comprehensive set of tests (excluding the automated GUI test).

# Copyright (C) 2018, 2019, 2020 Gregory W. Chicares.
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

# Suggested use:
#   $make raze; ./nychthemeral_test.sh
# Omitting the 'raze' step when it's known to be unnecessary makes the
# tests take three minutes instead of thirteen on a dual E5-2630 v3
# machine. What's difficult is knowing when it's truly unnecessary.

# SOMEDAY !! Not all tests return nonzero on failure, so 'set -e'
# doesn't reliably exit after the first test failure.

set -e

# This is why 'zsh' is specified in the hash-bang (the POSIX shell
# provides no convenient alternative):
setopt PIPE_FAIL

# Directory where this script resides.

srcdir=$(dirname "$(readlink --canonicalize "$0")")

coefficiency=${coefficiency:-"--jobs=$(nproc)"}

build_clutter='
/^make.*\[[0-9]*\]: Entering directory/d
/^make.*\[[0-9]*\]: Leaving directory/d
/^make.*\[[0-9]*\]: Nothing to be done for/d
/^make.*\[[0-9]*\]: warning: -j1 forced in submake: resetting jobserver mode.$/d
/^make.*\[[0-9]*\]: warning: -jN forced in submake: disabling jobserver mode.$/d
/^make.*\[[0-9]*\]: .* is up to date\./d
/^[^ ]*cpp  *-x /d
/^[^ ]*g++  *-[Mo]/d
/^[^ ]*gcc  *-[Mo]/d
/^[^ ]*windres -o /d
'

concinnity_clutter='
/.*\/test_coding_rules_test\.sh$/d
/^Testing .test_coding_rules.\.$/d
'

install_clutter='
/^Generating product files.$/d
/^All product files written.$/d
/^$/d
'

cli_cgi_clutter='
/^Test speed:/d
/^Timing test skipped: takes too long in debug mode$/d
/^  naic.*solve *: [0123456789.e-]* s mean; *[0123456789]* us.*runs/d
/^  finra.*solve *: [0123456789.e-]* s mean; *[0123456789]* us.*runs/d
/^    Input:        [0-9]* milliseconds$/d
/^    Input:        [0-9]* milliseconds$/d
/^    Calculations: [0-9]* milliseconds$/d
/^    Output:       [0-9]* milliseconds$/d
/^  0 errors$/d
'

schemata_clutter='
/^  Test cell-subelement sorting\.$/d
/^  Test schemata\.\.\.$/d
/^  Test RNC files with .jing.\.$/d
/^  Test XSD files with .jing.\.$/d
/^  Test XSD files with .xmllint.\.$/d
/^sample\.cns validates$/d
/^sample\.ill validates$/d
/^  Generate RNG from RNC with .trang.\.$/d
/^  Test RNG files with .jing.\.$/d
/^  Test RNG files with .xmllint.\.$/d
/^sample\.cns validates$/d
/^sample\.ill validates$/d
/^  Test invalid input\.\.\.$/d
/^  Test invalid input: .\.cns.\.$/d
/^  Test invalid input: .\.ill.\.$/d
/^  Regenerate XSD files as they should appear in the repository\.$/d
/^  Done\.$/d
'

nychthemeral_clutter='
/^# install; check physical closure/d
/^# cgi and cli tests/d
/^Test common gateway interface:/d
/^Test command line interface:/d
/^Test sample.cns:/d
/^Test sample.ill:/d
/^# system test/d
/^System test:/d
/^All [1-9][0-9]* files match./d
/^# unit tests/d
/^[1-9][0-9]* tests succeeded/d
/^# build with shared-object attributes/d
/^# cgi and cli tests in libstdc++ debug mode/d
/^Test common gateway interface:/d
/^Test command line interface:/d
/^Test sample.cns:/d
/^Test sample.ill:/d
/^# unit tests in libstdc++ debug mode/d
/^[1-9][0-9]* tests succeeded/d
/^# test concinnity/d
/^  Problems detected by xmllint:/d
/^  Miscellaneous problems:/d
/^  *[1-9][0-9]* source files/d
/^  *[1-9][0-9]* source lines/d
/^  *[1-9][0-9]* marked defects/d
/^# xrc tests/d
/^# test all valid emission types/d
/^# schema tests/d
/^# test mst --> xst conversion/d
/^$/d
'

lmi_build_type=$(/usr/share/misc/config.guess)

case "$lmi_build_type" in
    (*-*-cygwin*)
        platform=Cygwin
        ;;
esac

# This for-loop can iterate over as many toolchains as desired.
# Make sure the current production architecture is built last, so that
# it's the one installed to /opt/lmi/bin/ when this script ends.
triplets="x86_64-w64-mingw32 i686-w64-mingw32"
if [ "Cygwin" != "$platform" ] && [ "WSL" != "$platform" ]
then
# 'triplets' really is used, but in a zsh-specific way
# shellcheck disable=SC2034
    triplets="x86_64-pc-linux-gnu x86_64-w64-mingw32 i686-w64-mingw32"
fi
export LMI_COMPILER=gcc
export LMI_TRIPLET
# shellcheck disable=SC2043
#for LMI_TRIPLET in i686-w64-mingw32 ;
# "${=...} expansion--see zsh faq 3.1:
#  "Why does $var where var="foo bar" not do what I expect?"
for LMI_TRIPLET in ${=triplets} ;
do
# Directory for test logs.
#
# It seems redundant to construct yet another $prefix and $exec_prefix here;
# perhaps that should be done OAOO in a script that selects a toolchain.
prefix=/opt/lmi
exec_prefix="$prefix/${LMI_COMPILER}_${LMI_TRIPLET}"
log_dir="$exec_prefix"/logs
mkdir --parents "$log_dir"
{
printf 'LMI_TRIPLET = "%s"\n' "$LMI_TRIPLET" > /dev/tty

# Cannot recursively check script on path determined at runtime, so
# a directive like 'source="$srcdir"' doesn't work.
# shellcheck disable=SC1090
. "$srcdir"/set_toolchain.sh

cd /opt/lmi/src/lmi

printf '\n# test concinnity\n\n'
make "$coefficiency" check_concinnity 2>&1 \
  | tee "$log_dir"/concinnity | sed -e "$build_clutter" -e "$concinnity_clutter"

printf '# install; check physical closure\n\n'
make "$coefficiency" install check_physical_closure 2>&1 \
  | tee "$log_dir"/install | sed -e "$build_clutter" -e "$install_clutter"

printf '  Production system built--ready to start GUI test in another session.\n' > /dev/tty

printf '\n# cgi and cli tests\n\n'
make "$coefficiency" --output-sync=recurse cgi_tests cli_tests 2>&1 \
  | tee "$log_dir"/cgi_cli | sed -e "$build_clutter" -e "$cli_cgi_clutter"

if [ "i686-w64-mingw32" = "$LMI_TRIPLET" ]
then
  printf '\n# system test\n\n'
  make "$coefficiency" system_test 2>&1 \
    | tee "$log_dir"/system_test | sed -e "$build_clutter" -e "$install_clutter"
else
  printf '\n# system test skipped--it succeeds only with the production architecture\n\n'
fi

printf '\n# unit tests\n\n'
# shellcheck disable=SC2039
make "$coefficiency" --output-sync=recurse unit_tests 2>&1 \
  | tee >(grep '\*\*\*') >(grep \?\?\?\?) >(grep '!!!!' --count | xargs printf '%d tests succeeded\n') >"$log_dir"/unit_tests

printf '\n# build with shared-object attributes\n\n'
make "$coefficiency" all build_type=so_test USE_SO_ATTRIBUTES=1 2>&1 \
  | tee "$log_dir"/default_targets_so_test | sed -e "$build_clutter"

printf '\n# cgi and cli tests with shared-object attributes\n\n'
make "$coefficiency" --output-sync=recurse cgi_tests cli_tests build_type=so_test USE_SO_ATTRIBUTES=1 2>&1 \
  | tee "$log_dir"/cgi_cli_so_test | sed -e "$build_clutter" -e "$cli_cgi_clutter"

printf '\n# cgi and cli tests in libstdc++ debug mode\n\n'
make "$coefficiency" --output-sync=recurse cgi_tests cli_tests build_type=safestdlib 2>&1 \
  | tee "$log_dir"/cgi_cli_safestdlib | sed -e "$build_clutter" -e "$cli_cgi_clutter"

printf '\n# unit tests in libstdc++ debug mode\n\n'
# shellcheck disable=SC2039
make "$coefficiency" --output-sync=recurse unit_tests build_type=safestdlib 2>&1 \
  | tee >(grep '\*\*\*') >(grep \?\?\?\?) >(grep '!!!!' --count | xargs printf '%d tests succeeded\n') >"$log_dir"/unit_tests_safestdlib

printf '\n# xrc tests\n\n'
java -jar /opt/lmi/third_party/rng/jing.jar -c xrc.rnc ./*.xrc 2>&1 \
  | tee "$log_dir"/xrc

# Run the following tests in a throwaway directory so that the files
# they create can be cleaned up easily.
throwaway_dir="$log_dir"/tmp
mkdir --parents "$throwaway_dir"
cd "$throwaway_dir"

# Copy these files hither because the emission tests write some
# output files to the input file's directory.
install -m 0664 /opt/lmi/src/lmi/sample.ill .
install -m 0664 /opt/lmi/src/lmi/sample.cns .

printf '\n# test all valid emission types\n\n'

$PERFORM /opt/lmi/bin/lmi_cli_shared --file="$throwaway_dir"/sample.ill --accept --ash_nazg --data_path=/opt/lmi/data --emit=emit_test_data,emit_spreadsheet,emit_text_stream,emit_custom_0,emit_custom_1 >/dev/null

$PERFORM /opt/lmi/bin/lmi_cli_shared --file="$throwaway_dir"/sample.cns --accept --ash_nazg --data_path=/opt/lmi/data --emit=emit_test_data,emit_spreadsheet,emit_group_roster,emit_text_stream,emit_custom_0,emit_custom_1 >/dev/null

printf '\n# schema tests\n\n'
/opt/lmi/src/lmi/test_schemata.sh 2>&1 \
  | tee "$log_dir"/schemata | sed -e "$schemata_clutter"

printf '\n# test mst --> xst conversion\n\n'

# All unique characters found in '*.mst' as of 2019-05-13.
cat >eraseme.mst <<'EOF'
 !"#$%&'()*+,-./
0123456789
:;<=>?@
ABCDEFGHIJKLMNOPRSTUVWXYZ
[]^_
abcdefghijklmnopqrstuvwxyz
{}
EOF

printf '%b' "\
\\0337\\0336\\0335\\0334\\0333\\0332\\0331\\0330\\0327\\0326\
\\0325\\0324\\0323\\0322\\0321\\0320\\0365\\0317\\0316\\0315\
\\0314\\0313\\0312\\0311\\0310\\0307\\0306\\0365\\0305\\0304\
\\0303\\0302\\0301\\0300\\0277\\0365\\0276\\0275\\0274\\0273\
\\0272\\0271\\0270\\0267\\0266\\0265\\0264\\0263\\0262\\0261\
\\0260\\0257\\0255\\0254\\0253\\0252\\0251\\0250\\0247\\0246\
\\0245\\0365\\0244\\0242\\0241\\0240\\0365\\0236\\0235\\0234\
\\0233\\0232\\0231\\0230\\0227\\0226\\0225\\0224\\0223\\0222\
\\0221\\0220\\0217\\0216\\0215\\0214\\0213\\0212\\0211\\0210\
\\0207\\0206\\0205\\0365\\0204\\0202\\0365\
" >eraseme.touchstone

srcdir=. datadir=. /opt/lmi/src/lmi/mst_to_xst.sh
cmp eraseme.xst eraseme.touchstone

# Clean up stray output. (The zsh '(N)' glob qualifier turns on
# null_glob for a single expansion.)
# shellcheck disable=SC2039
for z in "$throwaway_dir"/*(N); do rm "$z"; done

# The automated GUI test simulates keyboard and mouse actions, so
# no such actions must be performed manually while it is running.
# Therefore, it is deliberately excluded from this script.
printf "\n  Do not forget to run the 'gui_test.sh' script.\n"
} 2>&1 | tee "$log_dir"/nychthemeral_test | sed -e "$nychthemeral_clutter"
done
