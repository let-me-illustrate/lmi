#!/bin/zsh

# Run the automated GUI test, filtering normal output.

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

# Suggested use: start 'nychthemeral_test.sh' in one session, in the
# way its inline documentation suggests; then run
#   $./gui_test.sh
# in a different session when prompted.

# SOMEDAY !! 'wx_test$(EXEEXT)' doesn't return nonzero on failure,
# so 'set -e' doesn't reliably exit after the first test failure.
# That doesn't much matter as long as only one real command is run.

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

gui_test_clutter='
/^about_dialog_version: started$/d
/^About dialog version string is .[[:digit:]]\+T[[:digit:]]\+Z.\.$/d
/^time=[[:digit:]]\+ms (for about_dialog_version)$/d
/^about_dialog_version: ok$/d
/^benchmark_census: started$/d
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

# Directory for test logs.
log_dir=/tmp/lmi/"${LMI_COMPILER}_${LMI_TRIPLET}"/logs
mkdir --parents "$log_dir"

cd /opt/lmi/src/lmi

"$PERFORM" /opt/lmi/bin/wx_test --ash_nazg --data_path=/opt/lmi/data 2>&1 \
  | tee "$log_dir"/gui_test | sed -e "$gui_test_clutter"
