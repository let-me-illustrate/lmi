// Numerical algorithms for stratified rates--unit test.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "stratified_algorithms.hpp"

#include "materially_equal.hpp"
#include "test_tools.hpp"

#include <cmath>                        // fabs()
#include <limits>

void banded_test()
{
    double const m = std::numeric_limits<double>::max();
    std::vector<double> const limits {1000.0 , 5000.0 , m   };
    std::vector<double> const rates  {   0.05,    0.02, 0.01};

    // At limits.

    LMI_TEST_EQUAL(0.02, banded_rate<double>()( 1000.0, limits, rates));
    LMI_TEST_EQUAL(0.01, banded_rate<double>()( 5000.0, limits, rates));

    // Ascertain whether limits are treated as incremental or cumulative.
    // Here, they are treated as cumulative:
    //   [   0, 1000) --> 0.05
    //   [1000, 5000) --> 0.02
    //   [5000,  inf) --> 0.01

    LMI_TEST_EQUAL(0.02, banded_rate<double>()( 4500.0, limits, rates));
    LMI_TEST_EQUAL(0.01, banded_rate<double>()( 5500.0, limits, rates));

    // In the interior of various brackets.

    LMI_TEST_EQUAL(0.05, banded_rate<double>()(  900.0, limits, rates));
    LMI_TEST_EQUAL(0.02, banded_rate<double>()( 1500.0, limits, rates));
    LMI_TEST_EQUAL(0.01, banded_rate<double>()(10000.0, limits, rates));

    LMI_TEST(materially_equal( 45.0, banded_product<double>()(  900.0, limits, rates)));
    LMI_TEST(materially_equal( 30.0, banded_product<double>()( 1500.0, limits, rates)));
    LMI_TEST(materially_equal(100.0, banded_product<double>()(10000.0, limits, rates)));

    // With some brackets of measure zero.

    std::vector<double> const z_limits {0.0, 1000.0 , 1000.0, 1000.0, 5000.0 , m   };
    std::vector<double> const z_rates  {9.9,    0.05,    8.8,    7.7,    0.02, 0.01};
    LMI_TEST(materially_equal( 30.0, banded_product<double>()( 1500.0, z_limits, z_rates)));

    // In the vicinity of extrema.

    LMI_TEST_EQUAL(0.05, banded_rate<double>()(      0.0, limits, rates));
    LMI_TEST_EQUAL(0.05, banded_rate<double>()(     -0.0, limits, rates));
    LMI_TEST_EQUAL(0.01, banded_rate<double>()(        m, limits, rates));
    LMI_TEST_EQUAL(0.01, banded_rate<double>()(0.999 * m, limits, rates));
    LMI_TEST_EQUAL(0.01, banded_rate<double>()(  0.1 * m, limits, rates));

    // Precondition violations.

    LMI_TEST_THROW
        (banded_rate<double>()(-1.0, limits, rates)
        ,std::runtime_error
        ,"Assertion 'zero <= total_amount' failed."
        );

    std::vector<double> const empty;

    LMI_TEST_THROW
        (banded_rate<double>()(0.0, empty, rates)
        ,std::runtime_error
        ,"Assertion '!cumulative_limits.empty()' failed."
        );

    LMI_TEST_THROW
        (banded_rate<double>()(0.0, limits, empty)
        ,std::runtime_error
        ,"Assertion 'rates.size() == cumulative_limits.size()' failed."
        );

    std::vector<double> const negative(limits.size(), -1.0);
    LMI_TEST_THROW
        (banded_rate<double>()(0.0, negative, rates)
        ,std::runtime_error
        ,"Assertion 'zero <= extrema.minimum()' failed."
        );

    std::vector<double> const zero(limits.size(), 0.0);
    LMI_TEST_THROW
        (banded_rate<double>()(0.0, zero, rates)
        ,std::runtime_error
        ,"Assertion 'zero < extrema.maximum()' failed."
        );

    std::vector<double> nonincreasing(limits);
    nonincreasing[0] = nonincreasing[1];
    banded_rate<double>()(0.0, nonincreasing, rates);

    std::vector<double> decreasing(limits);
    decreasing[0] = 1.0 + decreasing[1];
    LMI_TEST_THROW
        (banded_rate<double>()(0.0, decreasing, rates)
        ,std::runtime_error
        ,"Assertion 'std::is_sorted(z.begin(), z.end())' failed."
        );
}

void tiered_test()
{
    double const m = std::numeric_limits<double>::max();
    std::vector<double> const limits {1000.0 , 4000.0 , m   };
    std::vector<double> const rates  {   0.05,    0.02, 0.01};

    // At limits.

    LMI_TEST(materially_equal(0.0500, tiered_rate<double>()( 1000.0, limits, rates)));
    LMI_TEST(materially_equal(0.0275, tiered_rate<double>()( 4000.0, limits, rates)));

    // Ascertain whether limits are treated as incremental or cumulative.
    // Here, they are treated as incremental:
    //   [        0,      1000) --> 0.05
    //   [     1000, 1000+4000) --> 0.02
    //   [1000+4000,       inf) --> 0.01

    LMI_TEST(materially_equal(0.0260, tiered_rate<double>()( 5000.0, limits, rates)));

    // In the interior of various brackets.

    LMI_TEST(materially_equal(0.0500, tiered_rate<double>()(  900.0, limits, rates)));
    LMI_TEST(materially_equal(0.0400, tiered_rate<double>()( 1500.0, limits, rates)));
    LMI_TEST(materially_equal(0.0180, tiered_rate<double>()(10000.0, limits, rates)));

    LMI_TEST(materially_equal( 45.0, tiered_product<double>()(  900.0, 0.0, limits, rates)));
    LMI_TEST(materially_equal( 60.0, tiered_product<double>()( 1500.0, 0.0, limits, rates)));
    LMI_TEST(materially_equal(180.0, tiered_product<double>()(10000.0, 0.0, limits, rates)));

    // With nonzero second argument to tiered_product().

    LMI_TEST(materially_equal(  0.0, tiered_product<double>()(    0.0, 10000.0, limits, rates)));
    LMI_TEST(materially_equal( 10.0, tiered_product<double>()( 1000.0,  9000.0, limits, rates)));
    LMI_TEST(materially_equal( 50.0, tiered_product<double>()( 5000.0,  5000.0, limits, rates)));

    LMI_TEST(materially_equal( 10.0, tiered_product<double>()(  500.0,  1000.0, limits, rates)));

    LMI_TEST(materially_equal(  5.0, tiered_product<double>()(  100.0,   100.0, limits, rates)));
    LMI_TEST(materially_equal(175.0, tiered_product<double>()( 9900.0,   100.0, limits, rates)));

    // With some brackets of measure zero.

    std::vector<double> const z_limits {0.0, 1000.0 , 0.0, 0.0, 4000.0 , m   };
    std::vector<double> const z_rates  {9.9,    0.05, 8.8, 7.7,    0.02, 0.01};
    LMI_TEST(materially_equal(175.0, tiered_product<double>()( 9900.0,   100.0, z_limits, z_rates)));

    // In the vicinity of extrema.

    LMI_TEST(materially_equal(0.0500, tiered_rate<double>()(      0.0, limits, rates)));
    LMI_TEST(materially_equal(0.0500, tiered_rate<double>()(     -0.0, limits, rates)));
    LMI_TEST(materially_equal(0.0100, tiered_rate<double>()(        m, limits, rates)));
    LMI_TEST(materially_equal(0.0100, tiered_rate<double>()(0.999 * m, limits, rates)));
    LMI_TEST(materially_equal(0.0100, tiered_rate<double>()(  0.1 * m, limits, rates)));

    // Precondition violations.

    LMI_TEST_THROW
        (tiered_product<double>()(-1.0, 0.0, limits, rates)
        ,std::runtime_error
        ,"Assertion 'zero <= new_incremental_amount' failed."
        );

    LMI_TEST_THROW
        (tiered_product<double>()(0.0, -1.0, limits, rates)
        ,std::runtime_error
        ,"Assertion 'zero <= prior_total_amount' failed."
        );

    std::vector<double> const empty;

    LMI_TEST_THROW
        (tiered_product<double>()(0.0, 0.0, empty, rates)
        ,std::runtime_error
        ,"Assertion '!incremental_limits.empty()' failed."
        );

    LMI_TEST_THROW
        (tiered_product<double>()(0.0, 0.0, limits, empty)
        ,std::runtime_error
        ,"Assertion 'rates.size() == incremental_limits.size()' failed."
        );

    std::vector<double> const negative(limits.size(), -1.0);
    LMI_TEST_THROW
        (tiered_product<double>()(0.0, 0.0, negative, rates)
        ,std::runtime_error
        ,"Assertion 'zero <= extrema.minimum()' failed."
        );

    std::vector<double> const zero(limits.size(), 0.0);
    LMI_TEST_THROW
        (tiered_product<double>()(0.0, 0.0, zero, rates)
        ,std::runtime_error
        ,"Assertion 'zero < extrema.maximum()' failed."
        );

    std::vector<double> nonincreasing(limits);
    nonincreasing[0] = nonincreasing[1];
    tiered_product<double>()(0.0, 0.0, nonincreasing, rates);

    std::vector<double> decreasing(limits);
    decreasing[0] = 1.0 + decreasing[1];
    tiered_product<double>()(0.0, 0.0, decreasing, rates);
}

void progressively_limit_test()
{
    int a; // Addend to be reduced first.
    int b; // Addend to be reduced second.
    int z; // Limit to be imposed on sum.

    a =  2; b =  3; z = 6;
    progressively_limit(a, b, z);
    LMI_TEST( 2 == a &&  3 == b);

    a =  2; b =  3; z = 5;
    progressively_limit(a, b, z);
    LMI_TEST( 2 == a &&  3 == b);

    a =  2; b =  3; z = 4;
    progressively_limit(a, b, z);
    LMI_TEST( 1 == a &&  3 == b);

    a =  2; b =  3; z = 3;
    progressively_limit(a, b, z);
    LMI_TEST( 0 == a &&  3 == b);

    a =  2; b =  3; z = 2;
    progressively_limit(a, b, z);
    LMI_TEST( 0 == a &&  2 == b);

    a =  2; b =  3; z = 1;
    progressively_limit(a, b, z);
    LMI_TEST( 0 == a &&  1 == b);

    a =  2; b =  3; z = 0;
    progressively_limit(a, b, z);
    LMI_TEST( 0 == a &&  0 == b);

    a =  0; b =  0; z = 1;
    progressively_limit(a, b, z);
    LMI_TEST( 0 == a &&  0 == b);

    a = -1; b = -1; z = 1;
    progressively_limit(a, b, z);
    LMI_TEST(-1 == a && -1 == b);

    a = -1; b =  3; z = 1;
    progressively_limit(a, b, z);
    LMI_TEST(-1 == a &&  2 == b);

    a =  3; b = -1; z = 1;
    progressively_limit(a, b, z);
    LMI_TEST( 2 == a && -1 == b);

// TODO ?? It would seem wise to test floating point too.
}

void progressively_reduce_test()
{
    int a; // Addend to be reduced first.
    int b; // Addend to be reduced second.
    int z; // Desired reduction in their sum.
    int r; // Result: portion of desired reduction that couldn't be applied.

    // {negative, negative}

    a = -2; b = -3; z =  6;
    r = progressively_reduce(a, b, z);
    LMI_TEST(-2 == a && -3 == b &&  6 == r);

    a = -2; b = -3; z =  1;
    r = progressively_reduce(a, b, z);
    LMI_TEST(-2 == a && -3 == b &&  1 == r);

    a = -2; b = -3; z =  0;
    r = progressively_reduce(a, b, z);
    LMI_TEST(-2 == a && -3 == b &&  0 == r);

    a = -2; b = -3; z = -0;
    r = progressively_reduce(a, b, z);
    LMI_TEST(-2 == a && -3 == b &&  0 == r);

    a = -2; b = -3; z = -1;
    r = progressively_reduce(a, b, z);
    LMI_TEST(-1 == a && -3 == b &&  0 == r);

    a = -2; b = -3; z = -2;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a && -3 == b &&  0 == r);

    a = -2; b = -3; z = -3;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a && -2 == b &&  0 == r);

    a = -2; b = -3; z = -4;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a && -1 == b &&  0 == r);

    a = -2; b = -3; z = -5;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a &&  0 == b &&  0 == r);

    a = -2; b = -3; z = -6;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 1 == a &&  0 == b &&  0 == r);

    // {positive, positive}

    a =  2; b =  3; z =  0;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 2 == a &&  3 == b &&  0 == r);

    a =  2; b =  3; z = -0;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 2 == a &&  3 == b &&  0 == r);

    a =  2; b =  3; z = -1;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 3 == a &&  3 == b &&  0 == r);

    a =  2; b =  3; z =  1;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 1 == a &&  3 == b &&  0 == r);

    a =  2; b =  3; z =  2;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a &&  3 == b &&  0 == r);

    a =  2; b =  3; z =  3;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a &&  2 == b &&  0 == r);

    a =  2; b =  3; z =  4;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a &&  1 == b &&  0 == r);

    a =  2; b =  3; z =  5;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a &&  0 == b &&  0 == r);

    a =  2; b =  3; z =  6;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a &&  0 == b &&  1 == r);

    // {negative, positive}

    a = -2; b =  3; z =  4;
    r = progressively_reduce(a, b, z);
    LMI_TEST(-2 == a &&  0 == b &&  1 == r);

    a = -2; b =  3; z =  3;
    r = progressively_reduce(a, b, z);
    LMI_TEST(-2 == a &&  0 == b &&  0 == r);

    a = -2; b =  3; z =  2;
    r = progressively_reduce(a, b, z);
    LMI_TEST(-2 == a &&  1 == b &&  0 == r);

    a = -2; b =  3; z =  1;
    r = progressively_reduce(a, b, z);
    LMI_TEST(-2 == a &&  2 == b &&  0 == r);

    a = -2; b =  3; z =  0;
    r = progressively_reduce(a, b, z);
    LMI_TEST(-2 == a &&  3 == b &&  0 == r);

    a = -2; b =  3; z = -0;
    r = progressively_reduce(a, b, z);
    LMI_TEST(-2 == a &&  3 == b &&  0 == r);

    a = -2; b =  3; z = -1;
    r = progressively_reduce(a, b, z);
    LMI_TEST(-1 == a &&  3 == b &&  0 == r);

    a = -2; b =  3; z = -2;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a &&  3 == b &&  0 == r);

    a = -2; b =  3; z = -3;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 1 == a &&  3 == b &&  0 == r);

    // {positive, negative}

    a =  2; b = -3; z =  3;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a && -3 == b &&  1 == r);

    a =  2; b = -3; z =  2;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a && -3 == b &&  0 == r);

    a =  2; b = -3; z =  1;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 1 == a && -3 == b &&  0 == r);

    a =  2; b = -3; z =  0;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 2 == a && -3 == b &&  0 == r);

    a =  2; b = -3; z = -0;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 2 == a && -3 == b &&  0 == r);

    a =  2; b = -3; z = -1;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 2 == a && -2 == b &&  0 == r);

    a =  2; b = -3; z = -2;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 2 == a && -1 == b &&  0 == r);

    a =  2; b = -3; z = -3;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 2 == a &&  0 == b &&  0 == r);

    a =  2; b = -3; z = -4;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 3 == a &&  0 == b &&  0 == r);

    // {0, negative}

    a =  0; b = -3; z =  1;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a && -3 == b &&  1 == r);

    a =  0; b = -3; z =  0;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a && -3 == b &&  0 == r);

    a =  0; b = -3; z = -0;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a && -3 == b &&  0 == r);

    a =  0; b = -3; z = -1;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a && -2 == b &&  0 == r);

    a =  0; b = -3; z = -3;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a &&  0 == b &&  0 == r);

    a =  0; b = -3; z = -4;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 1 == a &&  0 == b &&  0 == r);

    // {negative, 0}

    a = -2; b =  0; z =  1;
    r = progressively_reduce(a, b, z);
    LMI_TEST(-2 == a &&  0 == b &&  1 == r);

    a = -2; b =  0; z =  0;
    r = progressively_reduce(a, b, z);
    LMI_TEST(-2 == a &&  0 == b &&  0 == r);

    a = -2; b =  0; z = -0;
    r = progressively_reduce(a, b, z);
    LMI_TEST(-2 == a &&  0 == b &&  0 == r);

    a = -2; b =  0; z = -1;
    r = progressively_reduce(a, b, z);
    LMI_TEST(-1 == a &&  0 == b &&  0 == r);

    a = -2; b =  0; z = -2;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a &&  0 == b &&  0 == r);

    a = -2; b =  0; z = -3;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 1 == a &&  0 == b &&  0 == r);

    // {0, 0}

    a =  0; b =  0; z = -1;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 1 == a &&  0 == b &&  0 == r);

    a =  0; b =  0; z =  1;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a &&  0 == b &&  1 == r);

    // {signed 0, signed 0}

    a =  0; b =  0; z =  0;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a &&  0 == b &&  0 == r);

    a =  0; b =  0; z = -0;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a &&  0 == b &&  0 == r);

    a =  0; b = -0; z =  0;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a &&  0 == b &&  0 == r);

    a =  0; b = -0; z = -0;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a &&  0 == b &&  0 == r);

    a = -0; b =  0; z =  0;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a &&  0 == b &&  0 == r);

    a = -0; b =  0; z = -0;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a &&  0 == b &&  0 == r);

    a = -0; b = -0; z =  0;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a &&  0 == b &&  0 == r);

    a = -0; b = -0; z = -0;
    r = progressively_reduce(a, b, z);
    LMI_TEST( 0 == a &&  0 == b &&  0 == r);

    // Make sure we can pass an rvalue as the third argument.
    a =  1; b =  2;
    r = progressively_reduce(a, b, 3);
    LMI_TEST( 0 == a &&  0 == b &&  0 == r);

    // These values are designed to fail with IEC 60559:1989 64-bit doubles.
    double ad, bd, zd, rd;
    ad = 0.03; bd = 0.04; zd = 0.07;
    rd = progressively_reduce(ad, bd, zd);
    // This is likely to fail with 'rd' being something like 6.93889e-18:
    //   LMI_TEST(0.0 == ad && 0.0 == bd && 0.0 == rd);
    // We can expect only that it's within a tolerance of
    //   machine epsilon
    //   times largest value
    //   times number of operations (taken approximately as ten).
    double t = 10.0 * zd * std::numeric_limits<double>::epsilon();
    LMI_TEST(std::fabs(ad) < t && std::fabs(bd) < t && std::fabs(rd) < t);

    // Here, delta is less than the sum of the first two arguments,
    // but greater than the first alone, so the first argument and
    // the returned remainder should both be reduced to exactly zero.
    ad = 0.0301; bd = 0.1110; zd = 0.0711;
    rd = progressively_reduce(ad, bd, zd);
    LMI_TEST(0.0 == ad && materially_equal(0.07, bd) && 0.0 == rd);
}

int test_main(int, char*[])
{
    banded_test();
    tiered_test();
    progressively_limit_test();
    progressively_reduce_test();
    return 0;
}
