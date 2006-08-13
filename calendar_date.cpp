// Calendar dates.
//
// Copyright (C) 2003, 2004, 2005, 2006 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: calendar_date.cpp,v 1.10 2006-08-13 11:22:09 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "calendar_date.hpp"

#include "alert.hpp"

#include <ctime>
#include <iomanip>
#include <istream>
#include <iterator>
#include <locale>
#include <ostream>
#include <sstream>

namespace
{
    int const gregorian_epoch_jdn = 2361222;

    bool is_leap_year(int year)
    {
        bool divisible_by_4   = 0 == year % 4;
        bool divisible_by_100 = 0 == year % 100;
        bool divisible_by_400 = 0 == year % 400;
        return divisible_by_400 || divisible_by_4 && !divisible_by_100;
    }

    int days_in_month(int month, bool leap_year)
    {
        static int month_lengths[12] =
            {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
            };
        if(!(0 < month && month < 13))
            {
            fatal_error()
                << "Month "
                << month
                << " is outside the range [1, 12]."
                << LMI_FLUSH
                ;
            }
        int n_days = month_lengths[month - 1];
        if(leap_year && 2 == month)
            {
            ++n_days;
            }
        return n_days;
    }

    // Reference for jdn <-> gregorian conversions: ACM algorithm 199.

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

    int CheckedGregorianToJdn(int year, int month, int day)
    {
        int const original_year  = year;
        int const original_month = month;
        int const original_day   = day;
        int const jdn = GregorianToJdn(year, month, day);
        JdnToGregorian(jdn, year, month, day);
        if
            (   original_year  != year
            ||  original_month != month
            ||  original_day   != day
            )
            {
            fatal_error()
                << "Date "
                << original_year << '-' << original_month << '-' << original_day
                << " is invalid. Perhaps "
                << year << '-' << month << '-' << day
                << " was meant."
                << LMI_FLUSH
                ;
            }
        return jdn;
    }
} // Unnamed namespace.

ymd_t JdnToYmd(jdn_t z)
{
    int year;
    int month;
    int day;
    JdnToGregorian(z.value(), year, month, day);
    return ymd_t(day + 100 * month + 10000 * year);
}

jdn_t YmdToJdn(ymd_t z)
{
    int g = z.value();
    int year = g / 10000;
    g -= year * 10000;
    int month = g / 100;
    g -= month * 100;
    int day = g;
    return jdn_t(CheckedGregorianToJdn(year, month, day));
}

calendar_date::calendar_date()
    :jdn_(today().julian_day_number())
{
    cache_gregorian_elements();
}

calendar_date::calendar_date(jdn_t z)
    :jdn_(z.value())
{
    cache_gregorian_elements();
}

calendar_date::calendar_date(ymd_t z)
    :jdn_(YmdToJdn(z).value())
{
    cache_gregorian_elements();
}

calendar_date::calendar_date(int year, int month, int day)
{
    assign_from_gregorian(year, month, day);
}

calendar_date& calendar_date::operator=(jdn_t j)
{
    jdn_ = j.value();
    cache_gregorian_elements();
    return *this;
}

calendar_date& calendar_date::operator=(ymd_t ymd)
{
    jdn_ = YmdToJdn(ymd).value();
    cache_gregorian_elements();
    return *this;
}

calendar_date& calendar_date::operator++()
{
    ++jdn_;
    cache_gregorian_elements();
    return *this;
}

calendar_date& calendar_date::operator--()
{
    --jdn_;
    cache_gregorian_elements();
    return *this;
}

calendar_date& calendar_date::operator+=(int i)
{
    jdn_ += i;
    cache_gregorian_elements();
    return *this;
}

calendar_date& calendar_date::operator-=(int i)
{
    jdn_ -= i;
    cache_gregorian_elements();
    return *this;
}

int calendar_date::julian_day_number() const
{
    return jdn_;
}

int calendar_date::julian_day_number(int z)
{
    jdn_ = z;
    cache_gregorian_elements();
    return jdn_;
}

bool calendar_date::operator==(calendar_date const& z) const
{
    return jdn_ == z.jdn_;
}

bool calendar_date::operator<(calendar_date const& z) const
{
    return jdn_ < z.jdn_;
}

int calendar_date::year() const
{
    return cached_year_;
}

int calendar_date::month() const
{
    return cached_month_;
}

int calendar_date::day() const
{
    return cached_day_;
}

int calendar_date::days_in_month() const
{
    return ::days_in_month(cached_month_, ::is_leap_year(cached_year_));
}

int calendar_date::days_in_year() const
{
    return 365 + is_leap_year();
}

bool calendar_date::is_leap_year() const
{
    return ::is_leap_year(cached_year_);
}

// This could delegate to platform-specific code; for now, it just
// returns ISO8601 with hyphens.
//
std::string calendar_date::str() const
{
    std::ostringstream oss;
    oss
        << std::setfill('0') << std::setw(4) << year()
        << '-'
        << std::setfill('0') << std::setw(2) << month()
        << '-'
        << std::setfill('0') << std::setw(2) << day()
        ;
    return oss.str();
}

void calendar_date::assign_from_gregorian(int year, int month, int day)
{
    jdn_ = CheckedGregorianToJdn(year, month, day);
    cached_year_  = year;
    cached_month_ = month;
    cached_day_   = day;
}

void calendar_date::cache_gregorian_elements() const
{
    JdnToGregorian(jdn_, cached_year_, cached_month_, cached_day_);
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

/// Increment by a given number of years and months.
///
/// What date is one month after 2001-01-31? Two answers are possible:
///  - 2001-02-28 is 'curtate';
///  - 2001-03-01 is not.

calendar_date calendar_date::add_years_and_months
    (int n_years
    ,int n_months
    ,bool curtate
    )
{
    int year  = cached_year_  + n_years;
    int month = cached_month_ + n_months;
    int day   = cached_day_;

    int origin_zero_month = month - 1;
    year += origin_zero_month / 12;
    origin_zero_month %= 12;
    if(origin_zero_month < 0)
        {
        year              -=  1;
        origin_zero_month += 12;
        }
    month = origin_zero_month + 1;

    int last_day_of_month = ::days_in_month(month, ::is_leap_year(year));
    bool no_such_day = last_day_of_month < day;
    if(no_such_day)
        {
        day = last_day_of_month;
        }
    assign_from_gregorian(year, month, day);
    if(no_such_day && !curtate)
        {
        operator++();
        }
    cache_gregorian_elements();
    return *this;
}

calendar_date calendar_date::add_years(int n_years, bool curtate)
{
    add_years_and_months(n_years, 0, curtate);
    return *this;
}

int calculate_age
    (calendar_date const& birthdate
    ,calendar_date const& as_of_date
    ,bool                 use_age_nearest_birthday
    )
{
    if(as_of_date < birthdate)
        {
        fatal_error()
            << "As-of date ("
            << as_of_date.str()
            << ") precedes birthdate ("
            << birthdate.str()
            << ")."
            << LMI_FLUSH
            ;
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

std::string month_name(int month)
{
    if(!(0 < month && month < 13))
        {
        fatal_error()
            << "Month "
            << month
            << " is outside the range [1, 12]."
            << LMI_FLUSH
            ;
        }
    std::tm c_time;
    c_time.tm_mon = month - 1;
    char format[] = "%B";
    std::ostringstream oss;
    std::use_facet<std::time_put<char> >(oss.getloc()).put
        (std::ostreambuf_iterator<char>(oss.rdbuf())
        ,oss
        ,'\0'
        ,&c_time
        ,format
        ,format + std::char_traits<char>::length(format)
        );
    return oss.str();
}

calendar_date const& gregorian_epoch()
{
    static calendar_date const z((jdn_t(gregorian_epoch_jdn)));
    return z;
}

calendar_date const& last_yyyy_date()
{
    static calendar_date const z(9999, 12, 31);
    return z;
}

calendar_date today()
{
    std::time_t const t0 = time(0);
    std::tm* const t1 = std::localtime(&t0);
    return calendar_date
        (1900 + t1->tm_year
        ,   1 + t1->tm_mon
        ,       t1->tm_mday
        );
}

