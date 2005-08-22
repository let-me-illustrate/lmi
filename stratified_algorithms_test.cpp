// Miscellaneous mathematical routines--unit test.
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

// $Id: stratified_algorithms_test.cpp,v 1.1 2005-08-22 13:02:00 chicares Exp $

// TODO ?? Add tests for tiered_product<>() and tiered_rate<>().

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "mathmisc.hpp"

#include "materially_equal.hpp"
#include "miscellany.hpp" // lmi_array_size()

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

#include <cmath>          // std::fabs()
#include <limits>

void banded_test()
{
    double const m = std::numeric_limits<double>::max();
    double x[] = {1000.0 , 5000.0 , m   };
    double y[] = {   0.05,    0.02, 0.01};
    std::vector<double> const limits(x, x + lmi_array_size(x));
    std::vector<double> const rates(y, y + lmi_array_size(y));

    BOOST_TEST_EQUAL(0.05, banded_rate<double>()(      0.0, limits, rates));
    BOOST_TEST_EQUAL(0.05, banded_rate<double>()(     -0.0, limits, rates));
    BOOST_TEST_EQUAL(0.02, banded_rate<double>()(   1000.0, limits, rates));
    BOOST_TEST_EQUAL(0.01, banded_rate<double>()(   5000.0, limits, rates));
    BOOST_TEST_EQUAL(0.01, banded_rate<double>()(        m, limits, rates));
    BOOST_TEST_EQUAL(0.01, banded_rate<double>()(0.999 * m, limits, rates));
    BOOST_TEST_EQUAL(0.01, banded_rate<double>()(  0.1 * m, limits, rates));

    BOOST_TEST_THROW
        (banded_rate<double>()(-1.0, limits, rates)
        ,std::runtime_error
        ,""
        );

    BOOST_TEST_EQUAL(0.05, banded_rate<double>()(  900.0, limits, rates));
    BOOST_TEST_EQUAL(0.02, banded_rate<double>()( 1500.0, limits, rates));
    BOOST_TEST_EQUAL(0.01, banded_rate<double>()(10000.0, limits, rates));

    BOOST_TEST(materially_equal( 45.0, banded_product<double>()(  900.0, limits, rates)));
    BOOST_TEST(materially_equal( 30.0, banded_product<double>()( 1500.0, limits, rates)));
    BOOST_TEST(materially_equal(100.0, banded_product<double>()(10000.0, limits, rates)));
}

void progressively_limit_test()
{
    int a; // Addend to be reduced first.
    int b; // Addend to be reduced second.
    int z; // Limit to be imposed on sum.

    a =  2; b =  3; z = 6;
    progressively_limit(a, b, z);
    BOOST_TEST( 2 == a &&  3 == b);

    a =  2; b =  3; z = 5;
    progressively_limit(a, b, z);
    BOOST_TEST( 2 == a &&  3 == b);

    a =  2; b =  3; z = 4;
    progressively_limit(a, b, z);
    BOOST_TEST( 1 == a &&  3 == b);

    a =  2; b =  3; z = 3;
    progressively_limit(a, b, z);
    BOOST_TEST( 0 == a &&  3 == b);

    a =  2; b =  3; z = 2;
    progressively_limit(a, b, z);
    BOOST_TEST( 0 == a &&  2 == b);

    a =  2; b =  3; z = 1;
    progressively_limit(a, b, z);
    BOOST_TEST( 0 == a &&  1 == b);

    a =  2; b =  3; z = 0;
    progressively_limit(a, b, z);
    BOOST_TEST( 0 == a &&  0 == b);

    a =  0; b =  0; z = 1;
    progressively_limit(a, b, z);
    BOOST_TEST( 0 == a &&  0 == b);

    a = -1; b = -1; z = 1;
    progressively_limit(a, b, z);
    BOOST_TEST(-1 == a && -1 == b);

    a = -1; b =  3; z = 1;
    progressively_limit(a, b, z);
    BOOST_TEST(-1 == a &&  2 == b);

    a =  3; b = -1; z = 1;
    progressively_limit(a, b, z);
    BOOST_TEST( 2 == a && -1 == b);

// TODO ?? It would seem wise to test floating point too.
}

void progressively_reduce_test()
{
    int a; // Addend to be reduced first.
    int b; // Addend to be reduced second.
    int z; // Desired reduction in their sum.
    int r; // Result: portion of desired reduction that couldn't be applied.

    a = -2; b = -3; z =  6;
    r = progressively_reduce(a, b, z);
    BOOST_TEST(-2 == a && -3 == b &&  6 == r);

    a = -2; b = -3; z =  1;
    r = progressively_reduce(a, b, z);
    BOOST_TEST(-2 == a && -3 == b &&  1 == r);

    a = -2; b = -3; z =  0;
    r = progressively_reduce(a, b, z);
    BOOST_TEST(-2 == a && -3 == b &&  0 == r);

    a = -2; b = -3; z = -0;
    r = progressively_reduce(a, b, z);
    BOOST_TEST(-2 == a && -3 == b &&  0 == r);

    a = -2; b = -3; z = -1;
    r = progressively_reduce(a, b, z);
    BOOST_TEST(-1 == a && -3 == b &&  0 == r);

    a = -2; b = -3; z = -2;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a && -3 == b &&  0 == r);

    a = -2; b = -3; z = -3;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a && -2 == b &&  0 == r);

    a = -2; b = -3; z = -4;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a && -1 == b &&  0 == r);

    a = -2; b = -3; z = -5;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a &&  0 == b &&  0 == r);

    a = -2; b = -3; z = -6;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 1 == a &&  0 == b &&  0 == r);


    a =  2; b =  3; z =  0;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 2 == a &&  3 == b &&  0 == r);

    a =  2; b =  3; z = -0;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 2 == a &&  3 == b &&  0 == r);

    a =  2; b =  3; z = -1;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 3 == a &&  3 == b &&  0 == r);

    a =  2; b =  3; z =  1;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 1 == a &&  3 == b &&  0 == r);

    a =  2; b =  3; z =  2;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a &&  3 == b &&  0 == r);

    a =  2; b =  3; z =  3;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a &&  2 == b &&  0 == r);

    a =  2; b =  3; z =  4;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a &&  1 == b &&  0 == r);

    a =  2; b =  3; z =  5;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a &&  0 == b &&  0 == r);

    a =  2; b =  3; z =  6;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a &&  0 == b &&  1 == r);

    a = -2; b =  3; z =  4;
    r = progressively_reduce(a, b, z);
    BOOST_TEST(-2 == a &&  0 == b &&  1 == r);

    a = -2; b =  3; z =  3;
    r = progressively_reduce(a, b, z);
    BOOST_TEST(-2 == a &&  0 == b &&  0 == r);

    a = -2; b =  3; z =  2;
    r = progressively_reduce(a, b, z);
    BOOST_TEST(-2 == a &&  1 == b &&  0 == r);

    a = -2; b =  3; z =  1;
    r = progressively_reduce(a, b, z);
    BOOST_TEST(-2 == a &&  2 == b &&  0 == r);

    a = -2; b =  3; z =  0;
    r = progressively_reduce(a, b, z);
    BOOST_TEST(-2 == a &&  3 == b &&  0 == r);

    a = -2; b =  3; z = -0;
    r = progressively_reduce(a, b, z);
    BOOST_TEST(-2 == a &&  3 == b &&  0 == r);

    a = -2; b =  3; z = -1;
    r = progressively_reduce(a, b, z);
    BOOST_TEST(-1 == a &&  3 == b &&  0 == r);

    a = -2; b =  3; z = -2;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a &&  3 == b &&  0 == r);

    a = -2; b =  3; z = -3;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 1 == a &&  3 == b &&  0 == r);


    a =  2; b = -3; z =  3;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a && -3 == b &&  1 == r);

    a =  2; b = -3; z =  2;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a && -3 == b &&  0 == r);

    a =  2; b = -3; z =  1;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 1 == a && -3 == b &&  0 == r);

    a =  2; b = -3; z =  0;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 2 == a && -3 == b &&  0 == r);

    a =  2; b = -3; z = -0;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 2 == a && -3 == b &&  0 == r);

    a =  2; b = -3; z = -1;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 2 == a && -2 == b &&  0 == r);

    a =  2; b = -3; z = -2;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 2 == a && -1 == b &&  0 == r);

    a =  2; b = -3; z = -3;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 2 == a &&  0 == b &&  0 == r);

    a =  2; b = -3; z = -4;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 3 == a &&  0 == b &&  0 == r);


    a =  0; b = -3; z =  1;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a && -3 == b &&  1 == r);

    a =  0; b = -3; z =  0;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a && -3 == b &&  0 == r);

    a =  0; b = -3; z = -0;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a && -3 == b &&  0 == r);

    a =  0; b = -3; z = -1;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a && -2 == b &&  0 == r);

    a =  0; b = -3; z = -3;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a &&  0 == b &&  0 == r);

    a =  0; b = -3; z = -4;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 1 == a &&  0 == b &&  0 == r);


    a = -2; b =  0; z =  1;
    r = progressively_reduce(a, b, z);
    BOOST_TEST(-2 == a &&  0 == b &&  1 == r);

    a = -2; b =  0; z =  0;
    r = progressively_reduce(a, b, z);
    BOOST_TEST(-2 == a &&  0 == b &&  0 == r);

    a = -2; b =  0; z = -0;
    r = progressively_reduce(a, b, z);
    BOOST_TEST(-2 == a &&  0 == b &&  0 == r);

    a = -2; b =  0; z = -1;
    r = progressively_reduce(a, b, z);
    BOOST_TEST(-1 == a &&  0 == b &&  0 == r);

    a = -2; b =  0; z = -2;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a &&  0 == b &&  0 == r);

    a = -2; b =  0; z = -3;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 1 == a &&  0 == b &&  0 == r);


    a =  0; b =  0; z = -1;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 1 == a &&  0 == b &&  0 == r);

    a =  0; b =  0; z =  1;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a &&  0 == b &&  1 == r);


    a =  0; b =  0; z =  0;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a &&  0 == b &&  0 == r);

    a =  0; b =  0; z = -0;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a &&  0 == b &&  0 == r);

    a =  0; b = -0; z =  0;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a &&  0 == b &&  0 == r);

    a =  0; b = -0; z = -0;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a &&  0 == b &&  0 == r);

    a = -0; b =  0; z =  0;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a &&  0 == b &&  0 == r);

    a = -0; b =  0; z = -0;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a &&  0 == b &&  0 == r);

    a = -0; b = -0; z =  0;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a &&  0 == b &&  0 == r);

    a = -0; b = -0; z = -0;
    r = progressively_reduce(a, b, z);
    BOOST_TEST( 0 == a &&  0 == b &&  0 == r);

    // Make sure we can pass an rvalue as the third argument.
    a =  1; b =  2;
    r = progressively_reduce(a, b, 3);
    BOOST_TEST( 0 == a &&  0 == b &&  0 == r);

    // These values are designed to fail with IEC 60559:1989 64-bit doubles.
    double ad, bd, zd, rd;
    ad = .03; bd = 0.04; zd = .07;
    rd = progressively_reduce(ad, bd, zd);
    // This is likely to fail with 'rd' being something like 6.93889e-18:
    //   BOOST_TEST( 0.0 == ad &&  0.0 == bd &&  0.0 == rd);
    // We can expect only that it's within a tolerance of
    //   machine epsilon
    //   times largest value
    //   times number of operations (taken approximately as ten).
    double t = 10.0 * zd * std::numeric_limits<double>::epsilon();
    BOOST_TEST(std::fabs(ad) < t && std::fabs(bd) < t && std::fabs(rd) < t);
}

int test_main(int, char*[])
{
    banded_test();
    progressively_limit_test();
    progressively_reduce_test();
    return 0;
}

