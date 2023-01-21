#!/bin/zsh

# Run a comprehensive set of tests.

# Copyright (C) 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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
/^[^ ]*clang++  *-[Mo]/d
/^[^ ]*clang  *-[Mo]/d
/^[^ ]*windres -o /d
'

uninstall_clutter='
/^find \/opt\/lmi\/bin -type f -delete$/d
'

concinnity_clutter='
/.*\/test_coding_rules_test\.sh$/d
/^Testing .test_coding_rules.\.$/d
'

install_clutter='
/^install -m 0775 lmi_md5sum.*\/bin$/d
/^Generating product files.$/d
/^All product files written.$/d
/^$/d
'

# GUI-test output containing the substring
#   'not running distribution tests'
# is filtered out because it results from running 'wx_test' without
# any proprietary input subdirectory. SOMEDAY !! It would be good to
# filter out whatever normal output is seen when such a subdirectory
# is used.
#
# The "X connection to [...] broken" message might be avoided by
# techniques such as are mentioned here [reformatted]:
#   https://lists.nongnu.org/archive/html/lmi/2020-10/msg00077.html
#   [...] I believe that the X server connection is actually opened
#   not by wine itself, but by wineserver which [it] launches. But
#   xvfb-run exits once wine command itself does, while wineserver
#   exits slightly later, so it loses its connection to the server
#   because it [exits] earlier.
# but it is more expedient simply to filter it out (with a regex
# that, for some unknown reason, mustn't end in '$').

gui_test_clutter='
/^it looks like wine32 is missing, you should install it\.$/d
/^multiarch needs to be enabled first\.  as root, please$/d
/^execute "dpkg --add-architecture i386 && apt-get update &&$/d
/^apt-get install wine32"$/d
/^apt-get install wine32:i386"$/d
/^about_dialog_version: started$/d
/^About dialog version string is .[[:digit:]]\+T[[:digit:]]\+Z.\.$/d
/^time=[[:digit:]]\+ms (for about_dialog_version)$/d
/^about_dialog_version: ok$/d
/^benchmark_census: started$/d
/^Run case for [_[:alnum:]]\+\.cns: [[:digit:]]\+ms elapsed$/d
/^Print case to PDF for [_[:alnum:]]\+\.cns: [[:digit:]]\+ms elapsed$/d
/^Print case to spreadsheet for [_[:alnum:]]\+\.cns: [[:digit:]]\+ms elapsed$/d
/^time=[[:digit:]]\+ms (for benchmark_census)$/d
/^benchmark_census: ok$/d
/^calculation_summary: started$/d
/^time=[[:digit:]]\+ms (for calculation_summary)$/d
/^calculation_summary: ok$/d
/^configurable_settings: started$/d
/^configurable_settings: skipped (not running distribution tests)$/d
/^create_open_census: started$/d
/^time=[[:digit:]]\+ms (for create_open_census)$/d
/^create_open_census: ok$/d
/^create_open_database: started$/d
/^time=[[:digit:]]\+ms (for create_open_database)$/d
/^create_open_database: ok$/d
/^create_open_gpt: started$/d
/^time=[[:digit:]]\+ms (for create_open_gpt)$/d
/^create_open_gpt: ok$/d
/^create_open_illustration: started$/d
/^time=[[:digit:]]\+ms (for create_open_illustration)$/d
/^create_open_illustration: ok$/d
/^create_open_mec: started$/d
/^time=[[:digit:]]\+ms (for create_open_mec)$/d
/^create_open_mec: ok$/d
/^create_open_policy: started$/d
/^time=[[:digit:]]\+ms (for create_open_policy)$/d
/^create_open_policy: ok$/d
/^create_open_rounding: started$/d
/^time=[[:digit:]]\+ms (for create_open_rounding)$/d
/^create_open_rounding: ok$/d
/^create_open_strata: started$/d
/^time=[[:digit:]]\+ms (for create_open_strata)$/d
/^create_open_strata: ok$/d
/^create_open_text: started$/d
/^time=[[:digit:]]\+ms (for create_open_text)$/d
/^create_open_text: ok$/d
/^default_input: started$/d
/^default_input: skipped (not running distribution tests)$/d
/^time=[[:digit:]]\+ms (for default_input)$/d
/^default_input: ok$/d
/^default_update: started$/d
/^default_update: skipped (not running distribution tests)$/d
/^expiry_dates: started$/d
/^Expiry dates: begin=[[:digit:]]\+ ([[:digit:]]\+-[[:digit:]]\+-[[:digit:]]\+), end=[[:digit:]]\+ ([[:digit:]]\+-[[:digit:]]\+-[[:digit:]]\+)$/d
/^time=[[:digit:]]\+ms (for expiry_dates)$/d
/^expiry_dates: ok$/d
/^input_sequences: started$/d
/^time=[[:digit:]]\+ms (for input_sequences)$/d
/^input_sequences: ok$/d
/^input_validation: started$/d
/^input_validation: skipped (not running distribution tests)$/d
/^log_error: started$/d
/^time=[[:digit:]]\+ms (for log_error)$/d
/^log_error: ok$/d
/^paste_census: started$/d
/^time=[[:digit:]]\+ms (for paste_census)$/d
/^paste_census: ok$/d
/^pdf_census: started$/d
/^time=[[:digit:]]\+ms (for pdf_census)$/d
/^pdf_census: ok$/d
/^pdf_illustration: started$/d
/^time=[[:digit:]]\+ms (for pdf_illustration)$/d
/^pdf_illustration: ok$/d
/^validate_output_census: started$/d
/^time=[[:digit:]]\+ms (for validate_output_census)$/d
/^validate_output_census: ok$/d
/^validate_output_illustration: started$/d
/^time=[[:digit:]]\+ms (for validate_output_illustration)$/d
/^validate_output_illustration: ok$/d
/^validate_output_mec: started$/d
/^time=[[:digit:]]\+ms (for validate_output_mec)$/d
/^validate_output_mec: ok$/d
/^time=[[:digit:]]\+ms (for all tests)$/d
/^Test files path ..opt.lmi.gui_test. doesn.t exist--using current path instead\.$/d
/^NOTE: starting the test suite$/d
/^SUCCESS: [[:digit:]]\+ tests successfully completed\.$/d
/^NOTE: [[:digit:]]\+ tests were skipped$/d
/^X connection to .* broken (explicit kill or server shutdown)\./d
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

unit_test_stderr_clutter='
/.*\/test_coding_rules_test\.sh$/d
/^Testing .test_coding_rules.\.$/d
/^  This message should appear on stderr\.$/d
/^Integrity check failed for .coleridge.$/d
/^Please report this: culminate() not called\.$/d
/^sh: 1: xyzzy: not found$/d
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

# When /dev/tty doesn't exist, 'wine' issues these extra diagnostics.
#
# Escape a literal dollar sign as '[$]' rather than '\$'. The latter
# is less weird, but 'shellcheck' thinks '\$VARIABLE' calls for an
# expansion.

absent_tty_clutter='
/^[[:xdigit:]]*:err:winediag:nodrv_CreateWindow Application tried to create a window, but no driver could be loaded\.$/d
/^[[:xdigit:]]*:err:winediag:nodrv_CreateWindow Make sure that your X server is running and that [$]DISPLAY is set correctly\.$/d
/^[[:xdigit:]]*:err:systray:initialize_systray Could not create tray window$/d
'

nychthemeral_clutter='
/^# install; check physical closure/d
/^# GUI test/d
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
/^# default targets with shared-object attributes/d
/^# cgi and cli tests in libstdc++ debug mode/d
/^Test common gateway interface:/d
/^Test command line interface:/d
/^Test sample.cns:/d
/^Test sample.ill:/d
/^# default targets in libstdc++ debug mode/d
/^# unit tests in libstdc++ debug mode/d
/^# default targets with UBSan/d
/^# unit tests with UBSan/d
/^# ubsan tests skipped--used with POSIX only/d
/^[1-9][0-9]* tests succeeded/d
/^# test concinnity/d
/^  Problems detected by xmllint:/d
/^  Miscellaneous problems:/d
/^  *[1-9][0-9]* source files/d
/^  *[1-9][0-9]* source lines/d
/^  *[1-9][0-9]* marked defects/d
/^# concinnity test skipped--it uses POSIX only/d
/^# speed test/d
/^# xrc tests/d
/^# test all "emit_\*" output types supported by CLI/d
/^# test all valid emission types/d
/^# schema tests/d
/^# test mst --> xst conversion/d
/^# test PETE rebuild/d
/^$/d
'

if ! tty -s; then nychthemeral_clutter="${absent_tty_clutter}${nychthemeral_clutter}"; fi

# Install a bland 'configurable_settings.xml' for all architectures.
# This overwrites any existing file, but developers probably won't
# care, and end users are unaffected. Without this step, the
#   'test all "emit_*" output types supported by CLI'
# test below may fail, e.g. if different calculation-summary columns
# were selected.
/opt/lmi/src/lmi/bland_configurable_settings.sh /opt/lmi/data

lmi_build_type=$(/usr/share/misc/config.guess)

case "$lmi_build_type" in
    (*-*-cygwin*)
        platform=Cygwin
        ;;
esac

# This for-loop can iterate over as many toolchains as desired.
# Make sure the current production architecture is built last, so that
# it's the one installed to /opt/lmi/bin/ when this script ends.
lmi_toolchains="gcc_msw64"
if [ "Cygwin" != "$platform" ] && [ "WSL" != "$platform" ]
then
# 'lmi_toolchains' really is used, but in a zsh-specific way
# shellcheck disable=SC2034
  lmi_toolchains="clang_gnu64 gcc_gnu64 gcc_msw64"
fi
export LMI_COMPILER
export LMI_TRIPLET
# shellcheck disable=SC2043
# "${=...} expansion--see zsh faq 3.1:
#  "Why does $var where var="foo bar" not do what I expect?"
for toolchain in ${=lmi_toolchains} ;
do
case "$toolchain" in
    (clang_gnu64)
        LMI_COMPILER="clang"
        LMI_TRIPLET="x86_64-pc-linux-gnu"
        ;;
    (gcc_gnu64)
        LMI_COMPILER="gcc"
        LMI_TRIPLET="x86_64-pc-linux-gnu"
        ;;
    (gcc_msw64)
        LMI_COMPILER="gcc"
        LMI_TRIPLET="x86_64-w64-mingw32"
        ;;
    (*)
        printf 'Unknown toolchain "%s".\n' "$toolchain"
        return 1;
        ;;
esac
# Directory for test logs.
#
# It seems redundant to construct yet another $prefix and $exec_prefix here;
# perhaps that should be done OAOO in a script that selects a toolchain.
prefix=/opt/lmi
exec_prefix="$prefix/${LMI_COMPILER}_${LMI_TRIPLET}"
log_dir="$exec_prefix"/logs
mkdir --parents "$log_dir"
{
printf 'toolchain: %s\n' "${LMI_COMPILER}_${LMI_TRIPLET}" > /dev/tty || true

# Cannot recursively check script on path determined at runtime, so
# a directive like 'source="$srcdir"' doesn't work.
# shellcheck disable=SC1090
. "$srcdir"/set_toolchain.sh

cd /opt/lmi/src/lmi

make "$coefficiency" uninstall 2>&1 \
  | tee "$log_dir"/uninstall | sed -e "$build_clutter" -e "$uninstall_clutter"

if [ "x86_64-pc-linux-gnu" = "$LMI_TRIPLET" ]
then
  printf '\n# test concinnity\n\n'
  make "$coefficiency" check_concinnity 2>&1 \
    | tee "$log_dir"/concinnity | sed -e "$build_clutter" -e "$concinnity_clutter"
else
  printf '\n# concinnity test skipped--it uses POSIX only\n\n'
fi

printf '# install; check physical closure\n\n'
make "$coefficiency" install check_physical_closure 2>&1 \
  | tee "$log_dir"/install | sed -e "$build_clutter" -e "$install_clutter"

# The automated GUI test simulates keyboard and mouse actions, so
# no such actions must be performed manually while it is running
# (unless it is run in a virtual frame buffer, as with Xvfb here).
# The $WINEDEBUG setting inhibits wine-{6,7}.0 nuisance messages.
if [ "x86_64-pc-linux-gnu" != "$LMI_TRIPLET" ]
then
  printf '\n# GUI test\n\n'
  WINEDEBUG="fixme-event,fixme-imm,fixme-shell" \
  timeout 5m \
  xvfb-run "$PERFORM" "$prefix"/bin/wx_test"$EXEEXT" \
    --ash_nazg --data_path="$prefix"/data 2>&1 \
    | tee "$log_dir"/gui_test | sed -e "$build_clutter" -e "$gui_test_clutter" \
  || true ;
else
  printf '\n# GUI test skipped--it does not work properly with GTK\n'
fi

printf '\n# cgi and cli tests\n\n'
make "$coefficiency" --output-sync=recurse cgi_tests cli_tests 2>&1 \
  | tee "$log_dir"/cgi_cli | sed -e "$build_clutter" -e "$cli_cgi_clutter"

printf '\n# system test\n\n'
make "$coefficiency" system_test 2>&1 \
  | tee "$log_dir"/system_test | sed -e "$build_clutter" -e "$install_clutter"

printf '\n# unit tests\n\n'
# shellcheck disable=SC2039,SC3001
make "$coefficiency" --output-sync=recurse unit_tests 2>&1 \
  | tee >(grep '\*\*\*') >(grep \?\?\?\?) >(grep '!!!!' --count | xargs printf '%d tests succeeded\n') >"$log_dir"/unit_tests

printf '\n# default targets with shared-object attributes\n\n'
make "$coefficiency" build_type=so_test 2>&1 \
  | tee "$log_dir"/default_targets_so_test | sed -e "$build_clutter" -e "$concinnity_clutter" -e "$install_clutter"

printf '\n# cgi and cli tests with shared-object attributes\n\n'
make "$coefficiency" --output-sync=recurse cgi_tests cli_tests build_type=so_test 2>&1 \
  | tee "$log_dir"/cgi_cli_so_test | sed -e "$build_clutter" -e "$cli_cgi_clutter"

printf '\n# default targets in libstdc++ debug mode\n\n'
make "$coefficiency" build_type=safestdlib 2>&1 \
  | tee "$log_dir"/default_targets_safestdlib | sed -e "$build_clutter" -e "$concinnity_clutter" -e "$install_clutter"

printf '\n# cgi and cli tests in libstdc++ debug mode\n\n'
make "$coefficiency" --output-sync=recurse cgi_tests cli_tests build_type=safestdlib 2>&1 \
  | tee "$log_dir"/cgi_cli_safestdlib | sed -e "$build_clutter" -e "$cli_cgi_clutter"

printf '\n# unit tests in libstdc++ debug mode\n\n'
# shellcheck disable=SC2039,SC3001
make "$coefficiency" --output-sync=recurse unit_tests build_type=safestdlib 2>&1 \
  | tee >(grep '\*\*\*') >(grep \?\?\?\?) >(grep '!!!!' --count | xargs printf '%d tests succeeded\n') >"$log_dir"/unit_tests_safestdlib

if [ "x86_64-pc-linux-gnu" = "$LMI_TRIPLET" ]
then
  printf '\n# default targets with UBSan\n\n'

  export UBSAN_OPTIONS=print_stacktrace=1

  # WX !! Inhibit 'leak' by setting 'detect_leaks=0' in $ASAN_OPTIONS
  # until the next wx upgrade.
  #
  # Specify 'detect_invalid_pointer_pairs' even though that feature
  # isn't necessarily usable with gcc (see:
  #   https://lists.nongnu.org/archive/html/lmi/2022-06/msg00033.html
  # ) to support potential future experimentation. This has no effect
  # unless an applicable '-fsanitize=' option is specified at build
  # time; it's too confusing to have to change both a gcc option and
  # an environment variable. Set its value to one because a value of
  # two is known to give false positives.
  export ASAN_OPTIONS=detect_leaks=0:detect_invalid_pointer_pairs=1:strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1

  make "$coefficiency" build_type=ubsan 2>&1 \
    | tee "$log_dir"/default_targets_ubsan | sed -e "$build_clutter" -e "$concinnity_clutter" -e "$install_clutter"

  printf '\n# unit tests with UBSan\n\n'
  # shellcheck disable=SC3001
  (setopt nomultios; \
    ( \
      (make "$coefficiency" --output-sync=recurse unit_tests \
        build_type=ubsan \
      | tee \
        >(grep '\*\*\*') \
        >(grep \?\?\?\?) \
        >(grep '!!!!' --count | xargs printf '%d tests succeeded\n') \
      >"$log_dir"/unit_tests_ubsan_stdout \
      ) \
      3>&1 1>&2 2>&3 \
      | tee "$log_dir"/unit_tests_ubsan_stderr \
        | sed -e "$unit_test_stderr_clutter" \
        | sed -e's/^/UBSan: /' \
    ) 3>&1 1>&2 2>&3 \
  );

  printf '\n# system test with UBSan\n\n'
  make "$coefficiency" system_test \
    build_type=ubsan 2>&1 \
    | tee "$log_dir"/system_test_ubsan | sed -e "$build_clutter" -e "$install_clutter"
else
  printf '\n# ubsan tests skipped--used with POSIX only\n\n'
fi

if [ "greg" = "$(whoami)" ]
then
  printf '\n# speed test\n\n'
  make cli_timing
fi

printf '\n# xrc tests\n\n'
java -jar /opt/lmi/third_party/rng/jing.jar -c xrc.rnc ./*.xrc 2>&1 \
  | tee "$log_dir"/xrc

# Run the following tests in a throwaway directory so that the files
# they create can be cleaned up easily. To guard against interference
# from any files left over from previous runs, destroy that directory
# first, then recreate it.
throwaway_dir="$log_dir"/tmp
rm -rf "$throwaway_dir"
mkdir --parents "$throwaway_dir"
cd "$throwaway_dir"

# Copy these files hither because the emission tests write some
# output files to the input file's directory.
install -m 0664 /opt/lmi/src/lmi/sample.ill .
install -m 0664 /opt/lmi/src/lmi/sample.cns .

printf '\n# test all "emit_*" output types supported by CLI\n\n'

# PDF types not tested: they require wxPdfDoc

# 'emit_text_stream' output (on stdout) is tested by 'cli_tests'
# target above; here, it's discarded (but stderr is not)

# group-roster type omitted: sensible only for a census
$PERFORM /opt/lmi/bin/lmi_cli_shared"$EXEEXT" --file="$throwaway_dir"/sample.ill --accept --ash_nazg --data_path=/opt/lmi/data --emit=emit_to_pwd,emit_test_data,emit_spreadsheet,emit_text_stream,emit_custom_0,emit_custom_1,emit_calculation_summary_html,emit_calculation_summary_tsv >/dev/null

# same output filename for '.cns' as for '.ill': uniquify it
mv sample.tsv                       sample.ill.tsv

# calculation-summary types omitted: not sensible for a census
$PERFORM /opt/lmi/bin/lmi_cli_shared"$EXEEXT" --file="$throwaway_dir"/sample.cns --accept --ash_nazg --data_path=/opt/lmi/data --emit=emit_to_pwd,emit_test_data,emit_spreadsheet,emit_group_roster,emit_text_stream,emit_custom_0,emit_custom_1 >/dev/null

# same output filename for '.cns' as for '.ill': uniquify it
mv sample.tsv                       sample.cns.tsv

# remove '*.test' files: their sizes make it unattractive to store
# "touchstone" copies, and comparing them to such a touchstone would
# be useless anyway because the '*.test' format is exceedingly well
# tested by the 'system_test' target
rm sample.000000001.test
rm sample.composite.000000000.test
rm sample.test

# shorten and decorate output file names
mv sample.000000001.test0           sample.001.test0.touchstone
mv sample.000000001.test1           sample.001.test1.touchstone
mv sample.cns.tsv                   sample.cns.tsv.touchstone
mv sample.composite.000000000.test0 sample.000.test0.touchstone
mv sample.composite.000000000.test1 sample.000.test1.touchstone
mv sample.ill.tsv                   sample.ill.tsv.touchstone
mv sample.roster.tsv                sample.roster.tsv.touchstone
mv sample.summary.html              sample.summary.html.touchstone
mv sample.summary.tsv               sample.summary.tsv.touchstone
mv sample.test0                     sample.test0.touchstone
mv sample.test1                     sample.test1.touchstone

# DatePrepared: it's unclear why ".*" is needed, but it "works"
for z in *.touchstone; do \
  diff \
    --unified=0 \
    --strip-trailing-cr \
    --ignore-matching-lines="^DatePrepared[ \t]*.*'[0-9-]*'$" \
    "$z" "$srcdir/$z" \
    2>&1 | tee "$log_dir"/emit_cli \
  || true ; \
done

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

printf '\n# test PETE rebuild\n\n'

# Automatically-generated PETE files are in the repository.
# It is not actually necessary to rebuild them (except when PETE
# is changed). Running this test routinely simply verifies that
# the PETE rebuild script continues to work.

cd /opt/lmi/src/lmi/tools/pete-2.1.1
./rebuild_pete.sh >/dev/null
} 2>&1 | tee "$log_dir"/nychthemeral_test | sed -e "$nychthemeral_clutter"
done
