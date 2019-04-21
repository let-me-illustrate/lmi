#!/bin/zsh

# Run a comprehensive set of tests (excluding the automated GUI test).

# Copyright (C) 2018, 2019 Gregory W. Chicares.
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

# Suggested use:
#   $make clobber; ./nychthemeral_test.sh
# Omitting the 'clobber' step when it's known to be unnecessary makes
# that command take two minutes instead of five on a dual E5-2630 v3
# machine. What's difficult is knowing when it's truly unnecessary.

# SOMEDAY !! Not all tests return nonzero on failure, so 'set -e'
# doesn't reliably exit after the first test failure.

set -e

# This is why 'zsh' is specified in the hash-bang (the POSIX shell
# provides no convenient alternative):
setopt PIPE_FAIL

lmi_build_type=$(/usr/share/libtool/build-aux/config.guess)
case "$lmi_build_type" in
    (*-*-linux*)
        PERFORM=wine
        ;;
    (*)
        PERFORM=
        ;;
esac

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
/^cp /d
/^Test solve speed: /d
/^Timing test skipped: takes too long in debug mode$/d
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

# Directory for test logs.
log_dir=/tmp/lmi/logs
mkdir --parents "$log_dir"

{
cd /opt/lmi/src/lmi

printf '\n# test concinnity\n\n'
make "$coefficiency" check_concinnity 2>&1 \
  | sed -e "$build_clutter" -e "$concinnity_clutter"

printf '# install; check physical closure\n\n'
make "$coefficiency" install check_physical_closure 2>&1 \
  | tee "$log_dir"/install | sed -e "$build_clutter" -e "$install_clutter"

printf 'Production system built--ready to start GUI test in another session.\n' > /dev/tty

printf '\n# cgi and cli tests\n\n'
make "$coefficiency" --output-sync=recurse cgi_tests cli_tests 2>&1 \
  | tee "$log_dir"/cgi-cli | sed -e "$build_clutter" -e "$cli_cgi_clutter"

printf '\n# system test\n\n'
make "$coefficiency" system_test 2>&1 \
  | tee "$log_dir"/system-test | sed -e "$build_clutter" -e "$install_clutter"

printf '\n# unit tests\n\n'
make "$coefficiency" unit_tests 2>&1 \
  | tee >(grep '\*\*\*') >(grep \?\?\?\?) >(grep '!!!!' --count | xargs printf '%d tests succeeded\n') >"$log_dir"/unit-tests

printf '\n# build with shared-object attributes\n\n'
make "$coefficiency" all build_type=so_test USE_SO_ATTRIBUTES=1 2>&1 \
  | tee "$log_dir"/so_test | sed -e "$build_clutter"

printf '\n# cgi and cli tests in libstdc++ debug mode\n\n'
make "$coefficiency" --output-sync=recurse cgi_tests cli_tests build_type=safestdlib 2>&1 \
  | tee "$log_dir"/cgi-cli-safestdlib | sed -e "$build_clutter" -e "$cli_cgi_clutter"

printf '\n# unit tests in libstdc++ debug mode\n\n'
make "$coefficiency" unit_tests build_type=safestdlib 2>&1 \
  | tee >(grep '\*\*\*') >(grep \?\?\?\?) >(grep '!!!!' --count | xargs printf '%d tests succeeded\n') >"$log_dir"/unit-tests-safestdlib

printf '\n# xrc tests\n\n'
java -jar /opt/lmi/third_party/rng/jing.jar -c xrc.rnc ./*.xrc 2>&1 \
  | tee "$log_dir"/xrc

# Run the following tests in a throwaway directory so that the files
# they create can be cleaned up easily.
cd /tmp
mkdir --parents /tmp/lmi/tmp
cd /tmp/lmi/tmp

# Copy these files hither because the emission tests write some
# output files to the input file's directory.
cp /opt/lmi/src/lmi/sample.ill .
cp /opt/lmi/src/lmi/sample.cns .

printf '\n# test all valid emission types\n\n'

"$PERFORM" /opt/lmi/bin/lmi_cli_shared --file=/tmp/lmi/tmp/sample.ill --accept --ash_nazg --data_path=/opt/lmi/data --emit=emit_test_data,emit_spreadsheet,emit_text_stream,emit_custom_0,emit_custom_1 >/dev/null

"$PERFORM" /opt/lmi/bin/lmi_cli_shared --file=/tmp/lmi/tmp/sample.cns --accept --ash_nazg --data_path=/opt/lmi/data --emit=emit_test_data,emit_spreadsheet,emit_group_roster,emit_text_stream,emit_custom_0,emit_custom_1 >/dev/null

printf '\n# schema tests\n\n'
/opt/lmi/src/lmi/test_schemata.sh 2>&1 \
  | tee "$log_dir"/schemata | sed -e "$schemata_clutter"

# Clean up stray output. (The zsh '(N)' glob qualifier turns on
# null_glob for a single expansion.)
for z in /tmp/lmi/tmp/*(N); do rm "$z"; done

# The automated GUI test simulates keyboard and mouse actions, so
# no such actions must be performed manually while it is running.
# Therefore, it is deliberately excluded from this script.
printf '\nDo not forget to run wx_test.\n'
} 2>&1 | tee "$log_dir"/nychthemeral_test | sed -f errors.sed
