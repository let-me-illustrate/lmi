// Calendar dates.
//
// Copyright (C) 2003, 2005 Gregory W. Chicares.
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

// $Id: calendar_date.hpp,v 1.1 2005-01-14 19:47:44 chicares Exp $

#ifndef calendar_date_hpp
#define calendar_date_hpp

#include "config.hpp"

#include "expimp.hpp"

#ifndef LMI_LACKS_STD_ITERATOR
// <boost/operators.hpp> requires 'std::iterator'.
#   include <boost/operators.hpp>
#endif // not defined LMI_LACKS_STD_ITERATOR

#include <string>

class LMI_EXPIMP calendar_date
#ifndef LMI_LACKS_STD_ITERATOR
    :boost::additive<calendar_date, int>
    ,boost::totally_ordered<calendar_date>
    ,boost::unit_steppable<calendar_date>
#endif // defined LMI_LACKS_STD_ITERATOR
{
  public:
    calendar_date(); // Defaults to today's date.
    calendar_date(int year, int month, int day);
#ifdef BC_BEFORE_5_5
// TODO ?? For some reason I don't understand, this compiler wants to
// instantiate std::numeric_limits<calendar_date>, but providing this
// ctor declaration with no implementation stops that problem.
    calendar_date(void*);
#endif // BC_BEFORE_5_5

    calendar_date& operator++();
    calendar_date& operator--();
    calendar_date& operator+=(int);
    calendar_date& operator-=(int);

    // At least two different conventions are needed for anniversaries
    // and monthiversaries.
    //
    // If you were born on a leap day, then legally you become an
    // adult on the first of March, not the twenty-eighth of February.
    //
    // On the other hand, the monthiversaries of UL contracts are
    // expected to occur in distinct calendar months. If the effective
    // date is January thirty-first, then the February monthiversary
    // must occur on the twenty-eighth of February, not the first of
    // March. We refer to this as the 'curtate' case because the
    // thirty-first is shortened to the twenty-eighth.

    calendar_date add_years(int years, bool curtate);
    calendar_date add_years_and_months(int years, int months, bool curtate);

    // Julian day number as defined by ACM algorithm 199.
    //
    // The terms 'julian date' and 'julian day' are in all too common
    // use and irredeemably vague. They might mean the date on the
    // Julian as opposed to the Gregorian calendar, or perhaps the
    // number of days elapsed since the beginning of the current year.
    // Here, we use Julian Day Number, which is the number of days
    // since January 1, 4713 BC according to the proleptic Julian
    // calendar. More precisely, we use Chronological Julian Day,
    // which starts at midnight and respects local time, as opposed to
    // Astronomical Julian Day, which starts at noon and follows UTC.
    int julian_day_number(int);
    int julian_day_number() const;

    bool operator==(calendar_date const&) const;
    bool operator<(calendar_date const&) const;

#ifdef LMI_LACKS_STD_ITERATOR
    bool operator<=(calendar_date const&) const;
    bool operator>(calendar_date const&) const;
    bool operator>=(calendar_date const&) const;
    bool operator!=(calendar_date const&) const;
#endif // defined LMI_LACKS_STD_ITERATOR

    // Year, month, and day functions are origin one by convention,
    // so that concatenating their representations as YYYYMMDD would
    // produce a valid ISO8601 date.
    int year() const;        // [1752 | 1753, 9999]
    int month() const;       // [1, 12]
    int day() const;         // [1, 28 | 29 | 30 | 31]

    int days_in_month() const;
    int days_in_year() const;
    bool is_leap_year() const;

    std::string str() const;

    // Julian day number for 1752-09-14, the gregorian epoch
    // predominant among the English-speaking peoples.
    static calendar_date gregorian_epoch();
    static std::string month_name(int);

  private:
    int jdn_;
};

#ifndef LMI_LACKS_STD_ITERATOR
// gcc-3.x and bc5.51 do not work at all well with this technique
// suggested as a space optimization in the boost documentation:
//    template struct boost::additive<calendar_date, int>;
//    template struct boost::totally_ordered<calendar_date>;
//    template struct boost::unit_steppable<calendar_date>;
#else // defined LMI_LACKS_STD_ITERATOR

inline bool operator!=(calendar_date const& x, calendar_date const& y)
    {return !(x == y);}

inline calendar_date operator-(calendar_date const& x, int i)
    {return calendar_date(x) -= i;}

#endif // defined LMI_LACKS_STD_ITERATOR

// Age on EffDate if born on DOB.
int calculate_age
    (calendar_date const& DOB
    ,calendar_date const& EffDate
    ,bool                 UseANB
    );

std::ostream& operator<<(std::ostream& os, calendar_date const&);
std::istream& operator>>(std::istream& is, calendar_date&);

#endif // calendar_date_hpp

