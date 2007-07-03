// Calendar dates.
//
// Copyright (C) 2003, 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: calendar_date.hpp,v 1.11 2007-07-03 19:42:36 chicares Exp $

#ifndef calendar_date_hpp
#define calendar_date_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <boost/operators.hpp>

#include <iosfwd>
#include <string>

/// Julian day number, encapsulated in a class for use as a ctor
/// argument for class calendar_date.
///
/// Implicitly-declared special member functions do the right thing.
/// The implicitly-defined dtor being nonvirtual, this class of course
/// should not be derived from.

class jdn_t
{
  public:
    explicit jdn_t(int d) : value_(d) {}

    int value() const {return value_;}

  private:
    int value_;
};

/// Year, month, and day packed into an integer, encapsulated in a
/// class for use as a ctor argument for class calendar_date. The year
/// must incorporate the century: '99' means 99 AD, not 1999.
///
/// Implicitly-declared special member functions do the right thing.
/// The implicitly-defined dtor being nonvirtual, this class of course
/// should not be derived from.

class ymd_t
{
  public:
    explicit ymd_t(int d) : value_(d) {}

    int value() const {return value_;}

  private:
    int value_;
};

ymd_t JdnToYmd(jdn_t);
jdn_t YmdToJdn(ymd_t);

/// Class calendar_date represents a gregorian-calendar date in the
/// range [1752-09-14, 9999-12-31]. Date calculations are probably
/// valid for later dates, and for earlier dates on the proleptic
/// gregorian calendar, though only the given range is tested. The
/// gregorian epoch is assumed to be 1752-09-14, the value predominant
/// among the English-speaking peoples. The range limits are available
/// as [gregorian_epoch(), last_yyyy_date()].
///
/// A date is internally represented as its 'julian day number', which
/// is defined as number of days since January 1, 4713 BC according to
/// the proleptic julian calendar. Seemingly-similar terms such as
/// 'julian date' and 'julian day' are rendered irredeemably vague by
/// diverse lay usage: they might be intended to mean the date on the
/// julian as opposed to the gregorian calendar, or perhaps the number
/// of days elapsed since the beginning of the current year. Which
/// variant of julian day number the internal representation signifies
/// need not be specified, and deliberately is not: astronomers may
/// regard days as beginning at noon UTC, while most others probably
/// think of days as beginning at midnight and respecting civil time
/// in a local timezone, with appropriate daylight-saving adjustments.
/// Such concerns are irrelevant to this class, which represents only
/// a date, not a time of day.
///
/// The default ctor uses the current date.
///
/// By convention, the interface presents year, month, and day in
/// origin one, so that concatenating their numeric representations as
/// YYYYMMDD produces a valid ISO8601 date. Thus, the ranges of those
/// elements are:
///   year:  [1752 | 1753, 9999]
///   month: [1, 12]
///   day:   [1, 28 | 29 | 30 | 31]
/// depending on context.
///
/// Functions that increment a date by a given number of months or
/// years support two different conventions:
///
///  - A person born on a leap day attains legal majority on the first
///    of March, not the twenty-eighth of February.
///
///  - Monthly business processes may be expected to occur in distinct
///    calendar months. If the first occurrence falls on the thirty-
///    first of January, then the third falls on the thirty-first of
///    March, and therefore the second must fall on the twenty-eighth
///    of February, not the first of March. This is defined as the
///    'curtate' case because the nonexistent thirty-first of February
///    is shortened to the twenty-eighth.
///
/// Implicitly-declared special member functions do the right thing.

class LMI_SO calendar_date
    :boost::additive<calendar_date, int>
    ,boost::totally_ordered<calendar_date>
    ,boost::unit_steppable<calendar_date>
{
  public:
    enum
        {gregorian_epoch_jdn = 2361222
        ,last_yyyy_date_jdn  = 5373484
        ,min_verified_jdn    = 1721120
        ,max_verified_jdn    = 9999999
        };

    calendar_date();
    explicit calendar_date(jdn_t);
    explicit calendar_date(ymd_t);
    calendar_date(int year, int month, int day);

    calendar_date& operator=(jdn_t);
    calendar_date& operator=(ymd_t);

    calendar_date& operator++();
    calendar_date& operator--();
    calendar_date& operator+=(int);
    calendar_date& operator-=(int);

    int julian_day_number(int);
    int julian_day_number() const;

    bool operator==(calendar_date const&) const;
    bool operator<(calendar_date const&) const;

    int year() const;
    int month() const;
    int day() const;

    int days_in_month() const;
    int days_in_year() const;
    bool is_leap_year() const;

    std::string str() const;

  private:
    void assign_from_gregorian(int year, int month, int day);
    void cache_gregorian_elements() const;

    int jdn_;
    mutable int cached_year_;
    mutable int cached_month_;
    mutable int cached_day_;
};

// gcc-3.x and bc-5.5.1 do not work at all well with this technique
// suggested as a space optimization in the boost documentation:
//    template struct boost::additive<calendar_date, int>;
//    template struct boost::totally_ordered<calendar_date>;
//    template struct boost::unit_steppable<calendar_date>;

std::ostream& LMI_SO operator<<(std::ostream& os, calendar_date const&);
std::istream& LMI_SO operator>>(std::istream& is, calendar_date&);

calendar_date add_years
    (calendar_date const& date
    ,int                  n_years
    ,bool                 is_curtate
    );

calendar_date add_years_and_months
    (calendar_date const& date
    ,int                  n_years
    ,int                  n_months
    ,bool                 is_curtate
    );

/// Age on 'as_of_date' if born on 'birthdate'. Throws an exception
/// if 'as_of_date' precedes 'birthdate'.

int attained_age
    (calendar_date const& birthdate
    ,calendar_date const& as_of_date
    ,bool                 use_age_nearest_birthday
    );

/// Earliest as-of date consonant with a given maximum age and epoch.

calendar_date minimum_as_of_date
    (int                  maximum_age
    ,calendar_date const& epoch
    );

/// Earliest birthdate consonant with a given age and as-of date.

calendar_date minimum_birthdate
    (int                  age
    ,calendar_date const& as_of_date
    ,bool                 use_age_nearest_birthday
    );

/// Latest birthdate consonant with a given age and as-of date.

calendar_date maximum_birthdate
    (int                  age
    ,calendar_date const& as_of_date
    ,bool                 use_age_nearest_birthday
    );

std::string month_name(int);

// Some particularly useful dates.

calendar_date const& LMI_SO gregorian_epoch();
calendar_date const& LMI_SO last_yyyy_date();
calendar_date        LMI_SO today();

#endif // calendar_date_hpp

