// Helpers for working with dates in wx test suite code.
//
// Copyright (C) 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef wx_test_date_hpp
#define wx_test_date_hpp

#include "config.hpp"

#include "calendar_date.hpp"

#include <sstream>

// Return a string containing both the JDN and a string representation of the
// given date.
//
// This provides as much information as possible for the diagnostics.
inline
std::string dump_date(calendar_date const& date)
{
    std::ostringstream oss;
    oss << date << " (" << date.str() << ")";
    return oss.str();
}

// Return the date corresponding to the first day of the month following the
// month of the given date.
inline
calendar_date get_first_next_month(calendar_date const& date)
{
    int year = date.year();
    int month = date.month();
    if(month == 12)
        {
        month = 1;
        ++year;
        }
    else
        {
        ++month;
        }

    return calendar_date(year, month, 1);
}

// A variant of LMI_ASSERT_EQUAL which provides more information about dates in
// case of assertion failure.
#define LMI_ASSERT_DATES_EQUAL(observed,expected)                   \
    LMI_ASSERT_WITH_MSG                                             \
        ((observed) == (expected)                                   \
        ,"expected " << (dump_date(expected))                       \
            << " vs observed " << (dump_date(observed))             \
        )

#endif // wx_test_date_hpp
