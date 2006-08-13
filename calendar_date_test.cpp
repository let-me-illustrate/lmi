// Calendar dates--unit test.
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

// $Id: calendar_date_test.cpp,v 1.10 2006-08-13 11:22:09 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "calendar_date.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"
#include "timer.hpp"

#include <sstream>

// Function TestDateConversions() in 'wx_utility.cpp' validates the
// mapping between {year, month, day} triplets and JDN against the
// wx date-class implementation, for each date in the range
// [1752-09-14, 9999-12-31].

struct CalendarDateTest
{
    static void Test()
        {
        TestFundamentals();
        TestYMDBounds();
        TestYmdToJdnAndJdnToYmd();
        TestLeapYear();
        TestIncrementing();
        TestAgeCalculations();
        TestIo();
        TestSpeed();
        }

    static void TestFundamentals();
    static void TestYMDBounds();
    static void TestYmdToJdnAndJdnToYmd();
    static void TestLeapYear();
    static void TestIncrementing();
    static void TestAgeCalculations();
    static void TestIo();
    static void TestSpeed();
};

int test_main(int, char*[])
{
    CalendarDateTest::Test();
    return 0;
}

void CalendarDateTest::TestFundamentals()
{
    calendar_date dublin_epoch;
    dublin_epoch.julian_day_number(2415020);

    // *** Construction.

    // Default ctor.
    calendar_date const date0;
    // This test is not performed because it can fail if midnight is
    // crossed between it and the preceding statement, and also
    // because its success proves nothing.
//    BOOST_TEST_EQUAL(date0, today());

    // Copy ctor.
    calendar_date date1(date0);
    BOOST_TEST_EQUAL(date0, date1);

    // Construct from year, month, and day.
    calendar_date date2(1899, 12, 31);
    BOOST_TEST_EQUAL(dublin_epoch, date2);

    // Construct from jdn_t.
    calendar_date date3(jdn_t(2415020));
    BOOST_TEST_EQUAL(dublin_epoch, date3);

    // Construct from ymd_t.
    calendar_date date4(ymd_t(18991231));
    BOOST_TEST_EQUAL(dublin_epoch, date4);

    // *** Assignment.

    // Copy assignment operator.
    date1 = gregorian_epoch();
    BOOST_TEST_EQUAL(gregorian_epoch(), date1);

    // Assign from self.
    date1 = date1;
    BOOST_TEST_EQUAL(gregorian_epoch(), date1);

    // Assign from jdn_t.
    date2 = jdn_t(2361222);
    BOOST_TEST_EQUAL(gregorian_epoch(), date2);

    // Assign from ymd_t.
    date3 = ymd_t(17520914);
    BOOST_TEST_EQUAL(gregorian_epoch(), date3);
}

void CalendarDateTest::TestYMDBounds()
{
    // Test arguments that are prima facie out of bounds.

    BOOST_TEST_THROW
        (calendar_date( 2000,  0,  1)
        ,std::runtime_error
        ,"Date 2000-0-1 is invalid. Perhaps 1999-12-1 was meant."
        );

    BOOST_TEST_THROW
        (calendar_date( 2000, 13,  1)
        ,std::runtime_error
        ,"Date 2000-13-1 is invalid. Perhaps 2001-1-1 was meant."
        );

    BOOST_TEST_THROW
        (calendar_date( 2000,  1,  0)
        ,std::runtime_error
        ,"Date 2000-1-0 is invalid. Perhaps 1999-12-31 was meant."
        );

    BOOST_TEST_THROW
        (calendar_date( 2000,  1, 32)
        ,std::runtime_error
        ,"Date 2000-1-32 is invalid. Perhaps 2000-2-1 was meant."
        );

    // Test arguments that are out of bounds only in context.

    BOOST_TEST_THROW
        (calendar_date( 2000,  2, 30)
        ,std::runtime_error
        ,"Date 2000-2-30 is invalid. Perhaps 2000-3-1 was meant."
        );

    BOOST_TEST_THROW
        (calendar_date( 1900,  2, 29)
        ,std::runtime_error
        ,"Date 1900-2-29 is invalid. Perhaps 1900-3-1 was meant."
        );

    BOOST_TEST_THROW
        (calendar_date( 1999,  9, 31)
        ,std::runtime_error
        ,"Date 1999-9-31 is invalid. Perhaps 1999-10-1 was meant."
        );
}

void CalendarDateTest::TestYmdToJdnAndJdnToYmd()
{
    BOOST_TEST_EQUAL(2361222, YmdToJdn(ymd_t(17520914)).value());
    BOOST_TEST_EQUAL(17520914, JdnToYmd(jdn_t(2361222)).value());

    BOOST_TEST_EQUAL(2400000, YmdToJdn(ymd_t(18581116)).value());
    BOOST_TEST_EQUAL(18581116, JdnToYmd(jdn_t(2400000)).value());

    ymd_t const z0(18581116);
    BOOST_TEST_EQUAL(2400000, calendar_date(z0).julian_day_number());

    jdn_t const z1(YmdToJdn(ymd_t(18581116)));
    BOOST_TEST_EQUAL(2400000, calendar_date(z1).julian_day_number());
}

void CalendarDateTest::TestLeapYear()
{
    BOOST_TEST(!calendar_date(1900,  1,  1).is_leap_year());
    BOOST_TEST( calendar_date(2000,  1,  1).is_leap_year());
    BOOST_TEST(!calendar_date(2003,  1,  1).is_leap_year());
    BOOST_TEST( calendar_date(2004,  1,  1).is_leap_year());
}

void CalendarDateTest::TestIncrementing()
{
    // Test incrementing by whole number of years. If a policy
    // anniversary falls on a nonexistent day of the month, then it's
    // moved to the last day of the month. This is just one arbitrary
    // rule, and others are possible. For instance, people born on
    // leap-year day cannot vote on the 28th of February in the year
    // they attain legal adulthood.

    // Non-curtate tests.

    calendar_date birth_date;

    birth_date = calendar_date(2003,  1,  1);
    birth_date.add_years(1, false);
    BOOST_TEST_EQUAL(birth_date, calendar_date(2004,  1,  1));

    birth_date = calendar_date(2003, 12, 31);
    birth_date.add_years(1, false);
    BOOST_TEST_EQUAL(birth_date, calendar_date(2004, 12, 31));

    birth_date = calendar_date(1996,  2, 29);
    birth_date.add_years(1, false);
    BOOST_TEST_EQUAL(birth_date, calendar_date(1997,  3,  1));

    birth_date = calendar_date(1996,  2, 29);
    birth_date.add_years(4, false);
    BOOST_TEST_EQUAL(birth_date, calendar_date(2000,  2, 29));
    birth_date.add_years(1, false);
    BOOST_TEST_EQUAL(birth_date, calendar_date(2001,  3,  1));

    // Curtate tests.
    birth_date = calendar_date(2003,  1,  1);
    birth_date.add_years(1, true);
    BOOST_TEST_EQUAL(birth_date, calendar_date(2004,  1,  1));

    birth_date = calendar_date(2003, 12, 31);
    birth_date.add_years(1, true);
    BOOST_TEST_EQUAL(birth_date, calendar_date(2004, 12, 31));

    birth_date = calendar_date(1996,  2, 29);
    birth_date.add_years(1, true);
    BOOST_TEST_EQUAL(birth_date, calendar_date(1997,  2, 28));

    birth_date = calendar_date(1996,  2, 29);
    birth_date.add_years(4, true);
    BOOST_TEST_EQUAL(birth_date, calendar_date(2000,  2, 29));
    birth_date.add_years(1, true);
    BOOST_TEST_EQUAL(birth_date, calendar_date(2001,  2, 28));

    // Test incrementing by whole number of months and years.

    // Non-curtate tests.

    birth_date = calendar_date(1996,  1, 29);
    birth_date.add_years_and_months(0, 1, false);
    BOOST_TEST_EQUAL(birth_date, calendar_date(1996,  2, 29));
    birth_date.add_years_and_months(4, 0, false);
    BOOST_TEST_EQUAL(birth_date, calendar_date(2000,  2, 29));
    birth_date.add_years_and_months(1, 0, false);
    BOOST_TEST_EQUAL(birth_date, calendar_date(2001,  3,  1));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(0, 11, false);
    BOOST_TEST_EQUAL(birth_date, calendar_date(1996,  3,  1));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(1, -1, false);
    BOOST_TEST_EQUAL(birth_date, calendar_date(1996,  3,  1));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(-4, 11, false);
    BOOST_TEST_EQUAL(birth_date, calendar_date(1992,  3,  1));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(-3, -1, false);
    BOOST_TEST_EQUAL(birth_date, calendar_date(1992,  3,  1));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(0, -37, false);
    BOOST_TEST_EQUAL(birth_date, calendar_date(1992,  3,  1));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(0, 9, false);
    BOOST_TEST_EQUAL(birth_date, calendar_date(1995, 12, 31));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(0, 10, false);
    BOOST_TEST_EQUAL(birth_date, calendar_date(1996,  1, 31));

    birth_date = calendar_date(1994,  3, 31);
    birth_date.add_years_and_months(0, 21, false);
    BOOST_TEST_EQUAL(birth_date, calendar_date(1995, 12, 31));

    birth_date = calendar_date(2001,  1, 31);
    birth_date.add_years_and_months(0,  1, false);
    BOOST_TEST_EQUAL(birth_date, calendar_date(2001,  3,  1));

    birth_date = calendar_date(2001,  1, 31);
    birth_date.add_years_and_months(0,  3, false);
    BOOST_TEST_EQUAL(birth_date, calendar_date(2001,  5,  1));

    // Curtate tests.

    birth_date = calendar_date(1996,  1, 29);
    birth_date.add_years_and_months(0, 1, true);
    BOOST_TEST_EQUAL(birth_date, calendar_date(1996,  2, 29));
    birth_date.add_years_and_months(4, 0, true);
    BOOST_TEST_EQUAL(birth_date, calendar_date(2000,  2, 29));
    birth_date.add_years_and_months(1, 0, true);
    BOOST_TEST_EQUAL(birth_date, calendar_date(2001,  2, 28));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(0, 11, true);
    BOOST_TEST_EQUAL(birth_date, calendar_date(1996,  2, 29));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(1, -1, true);
    BOOST_TEST_EQUAL(birth_date, calendar_date(1996,  2, 29));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(-4, 11, true);
    BOOST_TEST_EQUAL(birth_date, calendar_date(1992,  2, 29));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(-3, -1, true);
    BOOST_TEST_EQUAL(birth_date, calendar_date(1992,  2, 29));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(0, -37, true);
    BOOST_TEST_EQUAL(birth_date, calendar_date(1992,  2, 29));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(0, 9, true);
    BOOST_TEST_EQUAL(birth_date, calendar_date(1995, 12, 31));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(0, 10, true);
    BOOST_TEST_EQUAL(birth_date, calendar_date(1996,  1, 31));

    birth_date = calendar_date(1994,  3, 31);
    birth_date.add_years_and_months(0, 21, true);
    BOOST_TEST_EQUAL(birth_date, calendar_date(1995, 12, 31));

    birth_date = calendar_date(2001,  1, 31);
    birth_date.add_years_and_months(0,  1, true);
    BOOST_TEST_EQUAL(birth_date, calendar_date(2001,  2, 28));

    birth_date = calendar_date(2001,  1, 31);
    birth_date.add_years_and_months(0,  3, true);
    BOOST_TEST_EQUAL(birth_date, calendar_date(2001,  4, 30));

    // Test the example in this comment block that appears in a
    // different translation unit:
        // ...Note that monthiversary
        // dates must be calculated wrt the effective date rather
        // than wrt any anniversary or monthiversary date because
        // information is lost if the day of the former exceeds
        // the number of days in the latter: for instance, if the
        // contract effective date is 2002-03-31, the eleventh
        // monthiversary is 2003-02-28, but the twelfth is not the
        // twenty-eighth of 2003-03 but rather the thirty-first.

    birth_date = calendar_date(2002,  3, 31);
    birth_date.add_years_and_months(0, 11, true);
    BOOST_TEST_EQUAL(birth_date, calendar_date(2003,  2, 28));

    birth_date = calendar_date(2002,  3, 31);
    birth_date.add_years_and_months(0, 12, true);
    BOOST_TEST_EQUAL(birth_date, calendar_date(2003,  3, 31));
}

void CalendarDateTest::TestAgeCalculations()
{
    // Suppose
    //   1958-07-02 is my birthdate, and
    //   2003-01-01 is the effective date.
    // Counting the days,
    //   2002-07-02, my age-44 birthday, is 183 days away, and
    //   2003-07-02, my age-45 birthday, is 182 days away,
    // so I'm age forty-five (ANB) in non-leap year 2003. But if my
    // birthdate were one day later, I'd be age forty-four (ANB).

    calendar_date effective_date = calendar_date(2003,  1,  1);
    calendar_date birth_date;

    // If the third argument to calculate_age() is
    //   true,  then calculate age nearest birthday (ANB)
    //   false, then calculate age last birthday    (ALB)
    bool const anb = true;
    bool const alb = false;

    birth_date     = calendar_date(1958,  7,  2);
    BOOST_TEST_EQUAL(45, calculate_age(birth_date, effective_date, anb));
    BOOST_TEST_EQUAL(44, calculate_age(birth_date, effective_date, alb));

    // If birthdate is one day later, then ANB is one year less.

    birth_date     = calendar_date(1958,  7,  3);
    BOOST_TEST_EQUAL(44, calculate_age(birth_date, effective_date, anb));
    BOOST_TEST_EQUAL(44, calculate_age(birth_date, effective_date, alb));

    // In a leap year, effective date can be an equal number of days
    // away from the two birthdays that bracket it. We arbitrarily
    // choose the higher age in this case for ANB, although we might
    // equally well choose the lower age.
    //
    // Thus, if
    //   1958-07-02 is my birthdate
    //   2004-01-01 is the effective date (in a leap year)
    // Counting the days,
    //   2003-07-02, my age-45 birthday, is 183 days away
    //   2004-07-02, my age-46 birthday, is 183 days away
    // so I'm forty-six (ANB) under this arbitrary choice, although an
    // equally good argument could be made that I'm forty-five.

    effective_date = calendar_date(2004,  1,  1);

    birth_date     = calendar_date(1958,  7,  2);
    BOOST_TEST_EQUAL(46, calculate_age(birth_date, effective_date, anb));
    BOOST_TEST_EQUAL(45, calculate_age(birth_date, effective_date, alb));

    // If birthdate is one day earlier,
    // then ANB is unambiguously forty-six.

    birth_date     = calendar_date(1958,  7,  1);
    BOOST_TEST_EQUAL(46, calculate_age(birth_date, effective_date, anb));
    BOOST_TEST_EQUAL(45, calculate_age(birth_date, effective_date, alb));

    // If birthdate is one day later,
    // then ANB is unambiguously forty-five.

    birth_date     = calendar_date(1958,  7,  3);
    BOOST_TEST_EQUAL(45, calculate_age(birth_date, effective_date, anb));
    BOOST_TEST_EQUAL(45, calculate_age(birth_date, effective_date, alb));

    // Test leap-day birthdate.

    birth_date     = calendar_date(1956,  2, 29);

    effective_date = calendar_date(2003,  8, 30);
    BOOST_TEST_EQUAL(47, calculate_age(birth_date, effective_date, anb));
    BOOST_TEST_EQUAL(47, calculate_age(birth_date, effective_date, alb));

    effective_date = calendar_date(2003,  8, 31);
    BOOST_TEST_EQUAL(48, calculate_age(birth_date, effective_date, anb));
    BOOST_TEST_EQUAL(47, calculate_age(birth_date, effective_date, alb));

    effective_date = calendar_date(2004,  2, 28);
    BOOST_TEST_EQUAL(48, calculate_age(birth_date, effective_date, anb));
    BOOST_TEST_EQUAL(47, calculate_age(birth_date, effective_date, alb));

    effective_date = calendar_date(2004,  2, 29);
    BOOST_TEST_EQUAL(48, calculate_age(birth_date, effective_date, anb));
    BOOST_TEST_EQUAL(48, calculate_age(birth_date, effective_date, alb));

    effective_date = calendar_date(2004,  3,  1);
    BOOST_TEST_EQUAL(48, calculate_age(birth_date, effective_date, anb));
    BOOST_TEST_EQUAL(48, calculate_age(birth_date, effective_date, alb));

    // Test leap-day effective date, even though many companies
    // probably forbid it.

    effective_date = calendar_date(2004,  2, 29);

    birth_date     = calendar_date(1958,  8, 30);
    BOOST_TEST_EQUAL(46, calculate_age(birth_date, effective_date, anb));
    BOOST_TEST_EQUAL(45, calculate_age(birth_date, effective_date, alb));

    birth_date     = calendar_date(1958,  8, 31);
    BOOST_TEST_EQUAL(45, calculate_age(birth_date, effective_date, anb));
    BOOST_TEST_EQUAL(45, calculate_age(birth_date, effective_date, alb));

    // Effective date mustn't precede birthdate--this should throw:
    birth_date     = calendar_date(2003,  1,  2);
    effective_date = calendar_date(2003,  1,  1);
    BOOST_TEST_THROW
        (calculate_age(birth_date, effective_date, anb)
        ,std::runtime_error
        ,"As-of date (2003-01-01) precedes birthdate (2003-01-02)."
        );
}

void CalendarDateTest::TestIo()
{
    BOOST_TEST_EQUAL("1752-09-14", calendar_date(1752,  9, 14).str());
    BOOST_TEST_EQUAL("2001-01-01", calendar_date(2001,  1,  1).str());

    calendar_date z;
    std::stringstream ss;
    ss << calendar_date(1752, 9, 14);
    ss >> z;
    BOOST_TEST_EQUAL(gregorian_epoch(), z);

    // Of course, a different locale might use different strings.
    BOOST_TEST_EQUAL("January"  , month_name( 1));
    BOOST_TEST_EQUAL("February" , month_name( 2));
    BOOST_TEST_EQUAL("March"    , month_name( 3));
    BOOST_TEST_EQUAL("April"    , month_name( 4));
    BOOST_TEST_EQUAL("May"      , month_name( 5));
    BOOST_TEST_EQUAL("June"     , month_name( 6));
    BOOST_TEST_EQUAL("July"     , month_name( 7));
    BOOST_TEST_EQUAL("August"   , month_name( 8));
    BOOST_TEST_EQUAL("September", month_name( 9));
    BOOST_TEST_EQUAL("October"  , month_name(10));
    BOOST_TEST_EQUAL("November" , month_name(11));
    BOOST_TEST_EQUAL("December" , month_name(12));

    BOOST_TEST_THROW
        (month_name( 0)
        ,std::runtime_error
        ,"Month 0 is outside the range [1, 12]."
        );

    BOOST_TEST_THROW
        (month_name(13)
        ,std::runtime_error
        ,"Month 13 is outside the range [1, 12]."
        );
}

namespace
{
    calendar_date x;
    calendar_date y(1899, 12, 31);

    void mete()
    {
        calendar_date x;
        calendar_date y(1899, 12, 31);
        x = y;
        x++;
        std::string s = x.str();
        x.add_years_and_months(1, 1, true);
        calculate_age(y, x, false);
    }

    void mete_construct()
    {
        calendar_date x;
        calendar_date y(1899, 12, 31);
    }

    void mete_assign()
    {
        x = y;
    }

    void mete_increment()
    {
        x++;
    }

    void mete_get_y_m_d()
    {
        x.year();
        x.month();
        x.day();
    }

    void mete_format()
    {
        std::string s = x.str();
    }

    void mete_calculate_age()
    {
        x.add_years_and_months(1, 1, true);
        calculate_age(y, x, false);
    }

} // Unnamed namespace.

void CalendarDateTest::TestSpeed()
{
    std::cout << "  Speed tests...\n"
        << "  Aggregate    : " << aliquot_timer(mete              ) << '\n'
        << "  Construct    : " << aliquot_timer(mete_construct    ) << '\n'
        << "  Assign       : " << aliquot_timer(mete_assign       ) << '\n'
        << "  Increment    : " << aliquot_timer(mete_increment    ) << '\n'
        << "  Get y, m, d  : " << aliquot_timer(mete_get_y_m_d    ) << '\n'
        << "  Format       : " << aliquot_timer(mete_format       ) << '\n'
        << "  Calculate age: " << aliquot_timer(mete_calculate_age) << '\n'
        ;
}

