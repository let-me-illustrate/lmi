# Run the automated GUI test, filtering normal output.

# Copyright (C) 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

# If running this file directly is impossible, i.e. if the current execution
# policy prohibits it and running
#
#           > powershell -Command "& {Set-ExecutionPolicy RemoteSigned}"
#
# fails due to lack of administrative rights, it has to be run using the
# following command from cmd.exe command prompt:
#
#           > powershell -ExecutionPolicy Bypass -Command .\gui_test.ps1
#
# Additional parameters, passed directly to wx_test, can be specified at the
# end of the previous command line.

# This script deliberately parallels gui_test.sh as closely as possible, in
# particular the same sed syntax is used for the search expressions to
# facilitate keeping them in sync between the two files, even if this requires
# some extra logic below to transform it into an array of regex patterns in the
# format understood by PowerShell.

# Lines beginning with a capitalized word, viz.
#   /^NOTE: starting the test suite$/d
# at the beginning and, e.g.,
#   /^SUCCESS: 21 tests successfully completed\.$/d
#   /^NOTE: 4 tests were skipped$/d
# at the end, are deliberately not filtered out.

# Output containing the substring
#   'not running distribution tests'
# is filtered out because it results from running 'wx_test' without
# any proprietary input subdirectory. SOMEDAY !! It would be good to
# filter out whatever normal output is seen when such a subdirectory
# is used.

$gui_test_clutter='
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
/^Warning: Test files path ..opt.lmi.gui_test. doesn.t exist\.$/d
'

# Convert sed expressions to simple regex patterns.
#
# Points of note:
#   - This file itself could use CR LF or just LF line terminators, accept
#     both of them for robustness.
#   - PowerShell doesn't support POSIX classes, so we need to expand them.
#   - BRE syntax used by sed differs from RE syntax used by PowerShell in more
#     aspects than those covered here, but handling [+()] is enough for now.
#   - We need to filter out the empty lines that are part of the quoted text.
$test_ignore_patterns = $gui_test_clutter   `
    -split "`r?`n"                          `
    -creplace '\[:alnum:]','A-Za-z0-9'      `
    -creplace '\[:digit:]','0-9'            `
    -creplace '\\\+','+'                    `
    -creplace '\(','\('                     `
    -creplace '\)','\)'                     `
    -creplace '^/'                          `
    -creplace '/d$'                         |
    Where-Object {$_}

$prefix = "/opt/lmi"

# Directory for test logs.
$log_dir = "$prefix/gcc/x86_64-w64-mingw32/logs"

if (!(Test-Path -PathType Container $log_dir)) {
    New-Item -Path $log_dir -ItemType Directory > $null
}

Set-Location "$prefix/src/lmi"

&"$prefix/bin/wx_test" $args --ash_nazg --data_path="$prefix/data" 2>&1 |
    Tee-Object "$log_dir/gui_test" |
    Select-String -NotMatch -Pattern $test_ignore_patterns
