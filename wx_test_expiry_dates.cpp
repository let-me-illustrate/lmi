// Expire dates test case for the GUI test suite.
//
// Copyright (C) 2014 Gregory W. Chicares.
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
// http://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id$

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "assert_lmi.hpp"
#include "calendar_date.hpp"
#include "global_settings.hpp"
#include "wx_test_case.hpp"
#include "version.hpp"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

LMI_WX_TEST_CASE(expiry_dates)
{
    fs::path expiry_path(global_settings::instance().data_directory() / "expiry");
    fs::ifstream is(expiry_path);
    LMI_ASSERT(is);

    calendar_date begin(last_yyyy_date ());
    calendar_date end  (gregorian_epoch());
    is >> begin >> end;
    LMI_ASSERT(is);
    LMI_ASSERT(is.eof());

    // The begin date must either be the first of month itself or a date in the
    // previous month, in which case we're interested in the end of the
    // following month and not the same one.
    int year = begin.year();
    int month = begin.month();
    int days_in_month;

    if(begin.day() == 1)
        {
        days_in_month = begin.days_in_month();
        }
    else
        {
        if(month == 12)
            {
            month = 1;
            year++;
            }
        else
            {
            month++;
            }

        days_in_month = calendar_date(year, month, 1).days_in_month();
        }

    calendar_date const end_of_month(year, month, days_in_month);
    LMI_ASSERT_EQUAL(end, end_of_month);
}
