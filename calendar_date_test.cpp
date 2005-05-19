// Calendar dates--unit test.
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

// $Id: calendar_date_test.cpp,v 1.3 2005-05-19 00:08:03 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "calendar_date.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

#define TEST_EQUALITY(a,b) \
    if((a) != (b)) \
        { \
        std::cout \
            << '\n' \
            << "ERROR: " << (a) << " != " << (b) \
            << " line " << __LINE__ \
            << ", file " << __FILE__ \
            << '\n' \
            ; \
        } \
    BOOST_TEST((a) == (b));

#ifdef BC_BEFORE_5_5
// COMPILER !! The old borland compiler gets operator!=() wrong.
    int test_main(int, char*[])
    {
        BOOST_TEST(false);
        return 1;
    }
#else

int test_main(int, char*[])
{
    calendar_date effective_date;
    calendar_date birth_date;

    // If the third argument to calculate_age() is
    //   true,  then calculate age nearest birthday (ANB)
    //   false, then calculate age last birthday    (ALB)
    bool const anb = true;
    bool const alb = false;

    // Test leap year.

    BOOST_TEST(!calendar_date(1900,  1,  1).is_leap_year());
    BOOST_TEST( calendar_date(2000,  1,  1).is_leap_year());
    BOOST_TEST(!calendar_date(2003,  1,  1).is_leap_year());
    BOOST_TEST( calendar_date(2004,  1,  1).is_leap_year());

    // Test incrementing by whole number of years. If a policy
    // anniversary falls on a nonexistent day of the month, then it's
    // moved to the last day of the month. This is just one arbitrary
    // rule, and others are possible. For instance, people born on
    // leap-year day cannot vote on the 28th of February in the year
    // they attain legal adulthood.

    // Non-curtate tests.

    birth_date = calendar_date(2003,  1,  1);
    birth_date.add_years(1, false);
    TEST_EQUALITY(birth_date, calendar_date(2004,  1,  1));

    birth_date = calendar_date(2003, 12, 31);
    birth_date.add_years(1, false);
    TEST_EQUALITY(birth_date, calendar_date(2004, 12, 31));

    birth_date = calendar_date(1996,  2, 29);
    birth_date.add_years(1, false);
    TEST_EQUALITY(birth_date, calendar_date(1997,  3,  1));

    birth_date = calendar_date(1996,  2, 29);
    birth_date.add_years(4, false);
    TEST_EQUALITY(birth_date, calendar_date(2000,  2, 29));
    birth_date.add_years(1, false);
    TEST_EQUALITY(birth_date, calendar_date(2001,  3,  1));

    // Curtate tests.
    birth_date = calendar_date(2003,  1,  1);
    birth_date.add_years(1, true);
    TEST_EQUALITY(birth_date, calendar_date(2004,  1,  1));

    birth_date = calendar_date(2003, 12, 31);
    birth_date.add_years(1, true);
    TEST_EQUALITY(birth_date, calendar_date(2004, 12, 31));

    birth_date = calendar_date(1996,  2, 29);
    birth_date.add_years(1, true);
    TEST_EQUALITY(birth_date, calendar_date(1997,  2, 28));

    birth_date = calendar_date(1996,  2, 29);
    birth_date.add_years(4, true);
    TEST_EQUALITY(birth_date, calendar_date(2000,  2, 29));
    birth_date.add_years(1, true);
    TEST_EQUALITY(birth_date, calendar_date(2001,  2, 28));

    // Test incrementing by whole number of months and years.

    // Non-curtate tests.

    birth_date = calendar_date(1996,  1, 29);
    birth_date.add_years_and_months(0, 1, false);
    TEST_EQUALITY(birth_date, calendar_date(1996,  2, 29));
    birth_date.add_years_and_months(4, 0, false);
    TEST_EQUALITY(birth_date, calendar_date(2000,  2, 29));
    birth_date.add_years_and_months(1, 0, false);
    TEST_EQUALITY(birth_date, calendar_date(2001,  3,  1));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(0, 11, false);
    TEST_EQUALITY(birth_date, calendar_date(1996,  3,  1));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(1, -1, false);
    TEST_EQUALITY(birth_date, calendar_date(1996,  3,  1));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(-4, 11, false);
    TEST_EQUALITY(birth_date, calendar_date(1992,  3,  1));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(-3, -1, false);
    TEST_EQUALITY(birth_date, calendar_date(1992,  3,  1));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(0, -37, false);
    TEST_EQUALITY(birth_date, calendar_date(1992,  3,  1));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(0, 9, false);
    TEST_EQUALITY(birth_date, calendar_date(1995, 12, 31));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(0, 10, false);
    TEST_EQUALITY(birth_date, calendar_date(1996,  1, 31));

    birth_date = calendar_date(1994,  3, 31);
    birth_date.add_years_and_months(0, 21, false);
    TEST_EQUALITY(birth_date, calendar_date(1995, 12, 31));

    // Curtate tests.

    birth_date = calendar_date(1996,  1, 29);
    birth_date.add_years_and_months(0, 1, true);
    TEST_EQUALITY(birth_date, calendar_date(1996,  2, 29));
    birth_date.add_years_and_months(4, 0, true);
    TEST_EQUALITY(birth_date, calendar_date(2000,  2, 29));
    birth_date.add_years_and_months(1, 0, true);
    TEST_EQUALITY(birth_date, calendar_date(2001,  2, 28));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(0, 11, true);
    TEST_EQUALITY(birth_date, calendar_date(1996,  2, 29));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(1, -1, true);
    TEST_EQUALITY(birth_date, calendar_date(1996,  2, 29));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(-4, 11, true);
    TEST_EQUALITY(birth_date, calendar_date(1992,  2, 29));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(-3, -1, true);
    TEST_EQUALITY(birth_date, calendar_date(1992,  2, 29));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(0, -37, true);
    TEST_EQUALITY(birth_date, calendar_date(1992,  2, 29));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(0, 9, false);
    TEST_EQUALITY(birth_date, calendar_date(1995, 12, 31));

    birth_date = calendar_date(1995,  3, 31);
    birth_date.add_years_and_months(0, 10, false);
    TEST_EQUALITY(birth_date, calendar_date(1996,  1, 31));

    birth_date = calendar_date(1994,  3, 31);
    birth_date.add_years_and_months(0, 21, false);
    TEST_EQUALITY(birth_date, calendar_date(1995, 12, 31));

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
    TEST_EQUALITY(birth_date, calendar_date(2003,  2, 28));

    birth_date = calendar_date(2002,  3, 31);
    birth_date.add_years_and_months(0, 12, true);
    TEST_EQUALITY(birth_date, calendar_date(2003,  3, 31));

    // Suppose
    //   1958-07-02 is my birthdate, and
    //   2003-01-01 is the effective date.
    // Counting the days,
    //   2002-07-02, my age-44 birthday, is 183 days away, and
    //   2003-07-02, my age-45 birthday, is 182 days away,
    // so I'm age forty-five (ANB) in non-leap year 2003. But if my
    // birthdate were one day later, I'd be age forty-four (ANB).

    effective_date = calendar_date(2003,  1,  1);

    birth_date     = calendar_date(1958,  7,  2);
    TEST_EQUALITY(45, calculate_age(birth_date, effective_date, anb));
    TEST_EQUALITY(44, calculate_age(birth_date, effective_date, alb));

    // If birthdate is one day later, then ANB is one year less.

    birth_date     = calendar_date(1958,  7,  3);
    TEST_EQUALITY(44, calculate_age(birth_date, effective_date, anb));
    TEST_EQUALITY(44, calculate_age(birth_date, effective_date, alb));

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
    TEST_EQUALITY(46, calculate_age(birth_date, effective_date, anb));
    TEST_EQUALITY(45, calculate_age(birth_date, effective_date, alb));

    // If birthdate is one day earlier,
    // then ANB is unambiguously forty-six.

    birth_date     = calendar_date(1958,  7,  1);
    TEST_EQUALITY(46, calculate_age(birth_date, effective_date, anb));
    TEST_EQUALITY(45, calculate_age(birth_date, effective_date, alb));

    // If birthdate is one day later,
    // then ANB is unambiguously forty-five.

    birth_date     = calendar_date(1958,  7,  3);
    TEST_EQUALITY(45, calculate_age(birth_date, effective_date, anb));
    TEST_EQUALITY(45, calculate_age(birth_date, effective_date, alb));

    // Test leap-day birthdate.

    birth_date     = calendar_date(1956,  2, 29);

    effective_date = calendar_date(2003,  8, 30);
    TEST_EQUALITY(47, calculate_age(birth_date, effective_date, anb));
    TEST_EQUALITY(47, calculate_age(birth_date, effective_date, alb));

    effective_date = calendar_date(2003,  8, 31);
    TEST_EQUALITY(48, calculate_age(birth_date, effective_date, anb));
    TEST_EQUALITY(47, calculate_age(birth_date, effective_date, alb));

    effective_date = calendar_date(2004,  2, 28);
    TEST_EQUALITY(48, calculate_age(birth_date, effective_date, anb));
    TEST_EQUALITY(47, calculate_age(birth_date, effective_date, alb));

    effective_date = calendar_date(2004,  2, 29);
    TEST_EQUALITY(48, calculate_age(birth_date, effective_date, anb));
    TEST_EQUALITY(48, calculate_age(birth_date, effective_date, alb));

    effective_date = calendar_date(2004,  3,  1);
    TEST_EQUALITY(48, calculate_age(birth_date, effective_date, anb));
    TEST_EQUALITY(48, calculate_age(birth_date, effective_date, alb));

    // Test leap-day effective date, even though many companies
    // probably forbid it.

    effective_date = calendar_date(2004,  2, 29);

    birth_date     = calendar_date(1958,  8, 30);
    TEST_EQUALITY(46, calculate_age(birth_date, effective_date, anb));
    TEST_EQUALITY(45, calculate_age(birth_date, effective_date, alb));

    birth_date     = calendar_date(1958,  8, 31);
    TEST_EQUALITY(45, calculate_age(birth_date, effective_date, anb));
    TEST_EQUALITY(45, calculate_age(birth_date, effective_date, alb));

    // Effective date mustn't precede birthdate--this should throw:
    birth_date     = calendar_date(2003,  1,  2);
    effective_date = calendar_date(2003,  1,  1);
    BOOST_TEST_THROW
        (calculate_age(birth_date, effective_date, anb)
        ,std::runtime_error
        ,""
        );

    return 0;
}
#endif // Not old borland compiler.

