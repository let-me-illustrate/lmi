// Calendar dates.
//
// Copyright (C) 2003, 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: calendar_date.cpp,v 1.3 2005-05-26 22:01:15 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "calendar_date.hpp"

#include "value_cast.hpp"

#include <ctime>
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>

// TODO ?? Consider using this LGPL class:
//   http://liblookdb.sourceforge.net/source/liblookdb/looktypes/lkdatetime.cpp
// at least as a comparative basis for unit testing.

namespace
{
    int const gregorian_epoch_jdn = 2361222;

    void regularize(int& year, int& month)
        {
        int origin_zero_month = month - 1;
        year += origin_zero_month / 12;
        origin_zero_month %= 12;
        if(origin_zero_month < 0)
            {
            year -= 1;
            origin_zero_month += 12;
            }
        month = origin_zero_month + 1;
        }

    int const jdn00010301 = 1721119;
    int const days_in_four_centuries = 146097;
    int const days_in_four_years = 1461;

    int GregorianToJdn(int year, int month, int day)
    {
        if(2 < month)
            {
            month -= 3;
            }
        else
            {
            month += 9;
            --year;
            }
        int c = year / 100;
        year -= 100 * c;
        return
              jdn00010301
            + day
            + (2 + 153 * month) / 5
            + ((days_in_four_years     * year) >> 2)
            + ((days_in_four_centuries * c   ) >> 2)
            ;
    }

    void JdnToGregorian(int j, int& year, int& month, int& day)
    {
        j -= jdn00010301;
        year  = ((j   << 2) - 1) / days_in_four_centuries;
        j     = ((j   << 2) - 1) - days_in_four_centuries * year;
        day   = j >> 2;
        j     = ((day << 2) + 3) / days_in_four_years;
        day   = ((day << 2) + 3) - days_in_four_years * j;
        day   = (day + 4) >> 2;
        month = (5 * day    - 3) / 153;
        day   = (5 * day    - 3) - 153 * month;
        day   = (day + 5) / 5;
        year  = 100 * year + j;
        if(month < 10)
            {
            month += 3;
            }
        else
            {
            month -= 9;
            ++year;
            }
    }
/* TODO ?? Do we want functions like these?
    int YyyyMmDdToJdn(int g)
    {
        int year = g / 10000;
        g -= year * 10000;
        int month = g / 100;
        g -= month * 100;
        int day = g;
        return GregorianToJdn(year, month, day);
    }

    int JdnToYyyyMmDd(int j)
    {
        int year;
        int month;
        int day;
        JdnToGregorian(j, year, month, day);
        return day + 100 * month + 10000 * year;
    }

    std::string JdnToYyyyMmDd(std::string const& j_str)
    {
        return value_cast<std::string>(JdnToYyyyMmDd(value_cast<int>(j_str)));
    }
*/
} // Unnamed namespace.

// Today's date.
calendar_date::calendar_date()
{
    std::time_t t0 = time(0);
    std::tm* t1 = std::localtime(&t0);
    jdn_ = GregorianToJdn(1900 + t1->tm_year, 1 + t1->tm_mon, t1->tm_mday);
}

calendar_date::calendar_date(int year, int month, int day)
{
    jdn_ = GregorianToJdn(year, month, day);
}

calendar_date& calendar_date::operator++()
{
    ++jdn_;
    return *this;
}

calendar_date& calendar_date::operator--()
{
    --jdn_;
    return *this;
}

calendar_date& calendar_date::operator+=(int i)
{
    jdn_ += i;
    return *this;
}

calendar_date& calendar_date::operator-=(int i)
{
    jdn_ -= i;
    return *this;
}

bool calendar_date::operator==(calendar_date const& z) const
{
    return jdn_ == z.jdn_;
}

bool calendar_date::operator<(calendar_date const& z) const
{
    return jdn_ < z.jdn_;
}

int calendar_date::julian_day_number() const
{
    return jdn_;
}

int calendar_date::julian_day_number(int z)
{
    jdn_ = z;
    return jdn_;
}

// This could delegate to platform-specific code; for now, it just
// returns ISO8601 with hyphens.
std::string calendar_date::str() const
{
    std::ostringstream oss;
    oss << year() << '-' << month() << '-' << day();
    return oss.str();
}

// TODO ?? Fix the manifest i18n defect.
std::string calendar_date::month_name(int month)
{
    if(!(0 < month && month < 13))
        {
        throw std::runtime_error("Month out of bounds in month_name().");
        }
    static char const* month_names[12] =
        {"January"
        ,"February"
        ,"March"
        ,"April"
        ,"May"
        ,"June"
        ,"July"
        ,"August"
        ,"September"
        ,"October"
        ,"November"
        ,"December"
        };
    return month_names[month - 1];
}

calendar_date calendar_date::gregorian_epoch()
{
    calendar_date z;
    z.jdn_ = gregorian_epoch_jdn;
    return z;
}

int calendar_date::year() const
{
    int year;
    int month;
    int day;
    JdnToGregorian(jdn_, year, month, day);
    return year;
}

int calendar_date::month() const
{
    int year;
    int month;
    int day;
    JdnToGregorian(jdn_, year, month, day);
    return month;
}

int calendar_date::day() const
{
    int year;
    int month;
    int day;
    JdnToGregorian(jdn_, year, month, day);
    return day;
}

int calendar_date::days_in_month() const
{
    static int month_lengths[12] =
        {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
        };
    int year;
    int month;
    int day;
    JdnToGregorian(jdn_, year, month, day);
    int n_days = month_lengths[month - 1];
    if(is_leap_year() && 2 == month)
        {
        ++n_days;
        }
    return n_days;
}

int calendar_date::days_in_year() const
{
    return 365 + is_leap_year();
}

bool calendar_date::is_leap_year() const
{
    int year;
    int month;
    int day;
    JdnToGregorian(jdn_, year, month, day);
    bool divisible_by_4   = 0 == year % 4;
    bool divisible_by_100 = 0 == year % 100;
    bool divisible_by_400 = 0 == year % 400;
    return divisible_by_400 || divisible_by_4 && !divisible_by_100;
}

// Add 'n_years' years and 'n_months' months.
calendar_date calendar_date::add_years_and_months
    (int n_years
    ,int n_months
    ,bool curtate
    )
{
    int year;
    int month;
    int day;
    JdnToGregorian(jdn_, year, month, day);

    year += n_years;
    month += n_months;
    regularize(year, month);

    // Some months have only 28, 29, or 30 days, but the desired date
    // cannot be later than the last day of the month. Set this date
    // to the first day of the month, use that temporary value to
    // ascertain the last day of the month, and limit the day to that
    // maximum if curtate, else use the next day.
    jdn_ = GregorianToJdn(year, month, 1);
    int last_day_of_month = days_in_month();
    bool no_such_day = last_day_of_month < day;
    if(no_such_day)
        {
        day = last_day_of_month;
        }
    jdn_ = GregorianToJdn(year, month, day);
    if(no_such_day && !curtate)
        {
        operator++();
        }
    return *this;
}

// Add 'n_years' years.
calendar_date calendar_date::add_years(int n_years, bool curtate)
{
    add_years_and_months(n_years, 0, curtate);
    return *this;
}

// Age on 'as_of_date' if born on 'birthdate'.
int calculate_age
    (calendar_date const& birthdate
    ,calendar_date const& as_of_date
    ,bool                 use_age_nearest_birthday
    )
{
    if(as_of_date < birthdate)
        {
        throw std::runtime_error("Effective date precedes birthdate.");
        }

    calendar_date last_birthday(birthdate);
    last_birthday.add_years(as_of_date.year() - birthdate.year(), false);

    if(as_of_date < last_birthday)
        {
        last_birthday.add_years(-1, false);
        }

    int age_last_birthday = last_birthday.year() - birthdate.year();

    if(!use_age_nearest_birthday)
        {
        return age_last_birthday;
        }

    double half_a_year = .5 * as_of_date.days_in_year();

    int diff =
            as_of_date.julian_day_number()
        -   last_birthday.julian_day_number()
        ;
    if(use_age_nearest_birthday && half_a_year <= diff)
        {
        return 1 + age_last_birthday;
        }
    else
        {
        return age_last_birthday;
        }
}

std::ostream& operator<<(std::ostream& os, calendar_date const& date)
{
    return os << date.julian_day_number();
}

std::istream& operator>>(std::istream& is, calendar_date& date)
{
    int z;
    is >> z;
    date.julian_day_number(z);
    return is;
}

