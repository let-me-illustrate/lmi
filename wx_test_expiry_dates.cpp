// Expire dates test case for the GUI test suite.
//
// Copyright (C) 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
//
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile_wx.hpp"

#include "assert_lmi.hpp"
#include "calendar_date.hpp"
#include "global_settings.hpp"
#include "path.hpp"
#include "version.hpp"
#include "wx_test_case.hpp"
#include "wx_test_date.hpp"

#include <iostream>

/// Validate dates in the 'expiry' file.
///
/// Write the begin and end dates to stdout, as JDN and as YYYYMMDD,
/// all on a single line, e.g.:
///   begin: 2457024 2015-01-01  end: 2457055 2015-02-01
/// (Those are the dates that would normally be expected for a
/// distribution prepared in December 2014.)
///
/// Validate dates only when the '--distribution' option is given.
///  - The begin date should be the first day of the next month.
///  - The end date should be the first day of the month after next.
/// Binaries are normally distributed toward the end of a month for
/// use throughout the following month. Consequently, these tests will
/// pass if run when a regular distribution is prepared, but they will
/// fail if rerun on the begin date--and that's exactly as desired:
/// such "failure" is not an error. (Incidentally, this illuminates
/// our decision to write all GUI-test output to stdout, not stderr.)
///
/// (When '--distribution' is not given, we had considered writing a
/// line to stdout saying that the tests had been skipped. But that is
/// not necessary, because in our normal workflow we'll compare each
/// '--distribution' run to a previously-saved '--distribution' run,
/// and diffs will make it obvious that the tests have been skipped.)
///
/// Occasionally "interim" distributions are issued, e.g., to add an
/// urgently-needed feature or to fix a critical mistake. They are
/// to be tested in the same way as regular distributions. All regular
/// distributions resemble each other; each "interim" distribution is
/// irregular in its own way, and its validation "failures" are not
/// errors, but may indeed convey useful information.

LMI_WX_TEST_CASE(expiry_dates)
{
    // Check that the expiry file can be read and is in valid format.
    fs::path expiry_path(global_settings::instance().data_directory() / "expiry");
    fs::ifstream is(expiry_path);
    LMI_ASSERT_WITH_MSG(is, "Failed to open \"expiry\" file for reading");

    calendar_date begin(last_yyyy_date ());
    calendar_date end  (gregorian_epoch());
    is >> begin >> end;
    LMI_ASSERT_WITH_MSG(is, "Failed to read dates from \"expiry\" file");

    std::cout
        << "Expiry dates: begin="
        << dump_date(begin)
        << ", end="
        << dump_date(end)
        << std::endl
        ;

    LMI_ASSERT_WITH_MSG(is.eof(), "Unexpected extra data in \"expiry\" file");

    if(is_distribution_test())
        {
        calendar_date const first_next_month = get_first_next_month(today());
        LMI_ASSERT_DATES_EQUAL(begin, first_next_month);

        calendar_date const
            first_after_next_month = get_first_next_month(first_next_month);
        LMI_ASSERT_DATES_EQUAL(end, first_after_next_month);
        }
}
